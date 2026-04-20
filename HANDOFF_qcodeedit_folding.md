# Handoff — bounded byte-range folding for textviewers

Second handoff, **landing after** `HANDOFF_qcodeedit.md` (byte-range
highlighter). Goal: let textviewers present Kate-driven code folding
for regions that fit within a caller-defined byte window, without
having to scan the whole file.

Reference points in this viewer repo:

- `wid/PaintArea.cpp::paintHighlighted` — where a `FoldingGutter`-like
  widget would sit, and where the active fold set is queried
- `HANDOFF_qcodeedit.md` — assumes `qce::IHighlighter::tokenizeBytes`
  from that handoff is already shipped; folding builds on the same
  byte buffer

---

## 1 · Why the editor's folding model doesn't work for us

`qce::RuleBasedFoldingProvider` (and `highlightLineEx`) produce
`FoldMarker` events as Kate rules fire on each line. The editor keeps
the whole document in memory, so it can walk from line 0 to line N
pairing begin/end markers and build a complete fold tree.

Textviewers has only a viewport into a possibly 100 GB mmap. The XML
root element (e.g. `<mediawiki>`) spans the entire file; finding its
matching close means scanning the whole thing, which is exactly what
we need not to do.

**Observation.** Every meaningful fold the user wants to toggle is
close to the top of the viewport — `<page>`, `<revision>`, `<text>`
in a Wiktionary dump, nested `<div>` chunks in HTML, etc. Their
begin/end pair typically fits within a few hundred kilobytes. The
gigantic root we can't fold is also the one the user never wants to
collapse interactively, because collapsing it would just blank the
screen.

So: **fold only regions whose begin and end both fall inside a
bounded window around the viewport**. Outside the window, no marker.

---

## 2 · Proposed qce changes

### 2.1 · Byte-range folding API

Mirror of `IHighlighter::tokenizeBytes` from handoff #1:

```cpp
// qce/IFoldingProvider.h
class IFoldingProvider {
public:
    virtual ~IFoldingProvider() = default;

    // Existing per-line API stays untouched.
    virtual void foldersInLine(const QString& line,
                               const HighlightState& stateIn,
                               QVector<FoldMarker>& markers,
                               HighlightState& stateOut) const = 0;

    // NEW — byte-range API. Markers carry byte offsets into `data`.
    virtual void foldersInBytes(const char *data,
                                qsizetype len,
                                const HighlightState& stateIn,
                                QVector<FoldMarker>& markers,
                                HighlightState& stateOut) const;
};
```

`FoldMarker::column` becomes byte offset when produced by
`foldersInBytes`. Document that clearly. Default implementation:
split on `\n`, delegate per line, translate columns via `Utf8Map`
(the same helper §2.3 of handoff #1 introduces).

### 2.2 · `RuleBasedFoldingProvider::foldersInBytes` override

For the rule-based provider, byte-native override lets us stream the
file without any QString allocation. Same scope as handoff #1 §2.2 —
ASCII rules run on bytes; regex rules fall back to QString per line.

### 2.3 · Pairing helper — the bounded part

Add a free function (or a class) in qce that takes a flat list of
begin/end markers plus a byte budget and returns matched pairs only
for regions that close within the budget:

```cpp
// qce/FoldPairing.h
namespace qce {

struct FoldRange {
    qsizetype beginByte;   // inclusive
    qsizetype endByte;     // exclusive
    int       regionId;
    int       depth;
};

// Given FoldMarkers in byte order, pair them as begin/end by regionId.
// A begin with no matching end within `maxSpanBytes` is dropped (no
// fold offered). Caller provides the byte budget, typically the size
// of the scan window. Markers crossing chunk boundaries are the
// caller's responsibility to stitch.
QVector<FoldRange> pairFolds(const QVector<FoldMarker>& markers,
                             qsizetype maxSpanBytes);

}
```

This is the only piece that's viewer-specific. The editor can ignore
it (it pairs via a different data structure — the line index).

---

## 3 · Unmatched-open semantics

Be explicit in docs:

> A `FoldMarker` with `isBegin == true` that has no matching close
> within the byte buffer passed to `foldersInBytes` is reported as-is.
> It is `pairFolds()` that decides whether to emit a `FoldRange` or
> drop the begin. The viewer will drop; an editor that has the full
> document may choose its own policy.

This keeps the low-level API honest (no hidden filtering) and puts
policy in one documented place.

---

## 4 · Acceptance tests

In qcodeedit's test suite:

1. **Byte-range equivalence** — for a rule-based provider on a
   multi-line UTF-8 buffer, byte markers from `foldersInBytes` map to
   the same (line, col) positions as the per-line output via
   `Utf8Map`.
2. **Pairing locality** — synthesised begin/end streams with known
   spans; verify `pairFolds()` drops begins whose end is beyond
   `maxSpanBytes` and keeps begins whose end is within it.
3. **Kate regression** — load `xml.xml`, feed a Wiktionary-sized page
   (say 4 MB `<page>` extracted from an actual dump); expect the page
   and each `<revision>` to pair, but a synthetic 200 MB dummy root
   to be dropped at `maxSpanBytes = 1 MiB`.
4. **Stitch property** — tokenising `[0, N)` and `[N, M)` with
   state-threading then `pairFolds` on the concatenated marker list
   equals `pairFolds` on the one-shot result.

---

## 5 · Viewer-side integration sketch

After qce lands §2, on the textviewers side:

- **Scan window.** The highlighter already runs on
  `[topByte − prelude, endByte + foldLookahead)`. `foldLookahead`
  is the "bounded" budget for §2.3. 1–4 MiB seems right; configurable.
- **Gutter widget.** New `wid/FoldingGutter` analogous to the
  scrollbars, drawn between row numbers and paint area. Iterates the
  `FoldRange` set overlapping the viewport and draws ▸/▾ icons.
- **Fold state.** `std::set<qsizetype>` of currently-collapsed
  `beginByte` values. Toggle on gutter click.
- **Render adjustment.** When a row falls inside a collapsed
  `FoldRange` (`beginByte <= lineStartByte < endByte`), skip it and
  draw a "folded" stub on the begin row (e.g. `… 412 lines`).
- **Cache.** The `FoldRange` list changes only when the scan window
  shifts or the file changes. Reuse across paints; recompute on
  scroll crossing a threshold (e.g. half the prelude).

Expected additions: ~300 lines (gutter + fold state + render hook).

---

## 6 · Open questions

1. **Interaction with wrap mode.** Smart-wrap breaks long lines at
   word boundaries. A fold range's `beginByte` sits on a specific
   wrap segment — the gutter icon lives next to that segment, not
   the logical line top. Needs care in the gutter hit-test.
2. **Scroll through a collapsed region.** `AbstractView::scrollDown`
   currently advances line-by-line. Collapsed folds should appear as
   a single row. Probably easiest at the view layer: wrap the
   existing `AbstractView` with a `FoldedView` that skips lines
   inside active folds. Not in scope for qce.
3. **Search inside collapsed folds.** Should `Searcher` expand the
   fold when a hit lands inside? Today's search works on the raw
   byte file and ignores visual state; the viewer would have to
   un-fold on match. Out of scope for qce.
4. **Selection crossing a fold boundary.** Mouse drag across a
   collapsed stub — select the whole hidden region, or just up to
   the stub? Editor convention: whole region. Out of scope for qce.

---

## 7 · Suggested commit sequence in qcodeedit

1. Add `qce::IFoldingProvider::foldersInBytes` default delegating to
   per-line via `Utf8Map`. Test §4.1.
2. Override in `RuleBasedFoldingProvider` for byte-native matching
   on ASCII rules. Test §4.4.
3. Add `qce::pairFolds()` helper. Test §4.2 and §4.3.
4. Bump qce to 1.3.
5. Consume from textviewers (§5).

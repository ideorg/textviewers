# Handoff — make qcodeedit's highlighter usable by textviewers

**To-do in qcodeedit** so that `textviewers` can consume qce highlighters
(and KateXmlReader output) without adapter layers. Both projects then share
one `qce::IHighlighter` contract.

Reference points in this viewer repo:

- `logic/Highlighter.h` — viewer-side interface we want to retire
- `logic/XmlHighlighter.cpp` — built-in FSM we want to delete once Kate XML
  covers it
- `wid/PaintArea.cpp::paintHighlighted` — consumer; shows the access pattern

---

## 1 · Why current qce::IHighlighter hurts the viewer

| dimension | qce today | viewer needs |
|---|---|---|
| call unit | **one QString line** per call | **arbitrary byte range** (mmap'd 100 GB file, single logical line may be 10 MB) |
| input type | `QString` (UTF-16) | `const char *` + length (UTF-8 bytes) |
| span positions | QChar indices | byte offsets |
| driver loop | editor re-highlights from cached "dirty" line | viewer tokenises `[topByte − prelude, endByte)` each paint |

Converting a 10 MB line into a 20 MB QString every paint is a non-starter,
and translating QChar spans back to byte offsets on each consumer is the
same work repeated everywhere.

---

## 2 · Proposed changes (prioritised)

### 2.1 · Add a byte-range entry point on `qce::IHighlighter`

```cpp
// qce/IHighlighter.h
class IHighlighter {
public:
    virtual ~IHighlighter() = default;
    virtual HighlightState initialState() const = 0;

    // Existing per-line API stays untouched.
    virtual void highlightLine(const QString& line,
                               const HighlightState& stateIn,
                               QVector<StyleSpan>& spans,
                               HighlightState& stateOut) const = 0;

    // NEW — byte-range API. Spans returned carry byte offsets into `data`.
    // Default implementation delegates to highlightLine() line-by-line so
    // existing highlighters keep working unchanged; subclasses that can
    // tokenise bytes natively override this for efficiency.
    virtual void tokenizeBytes(const char *data,
                               qsizetype len,
                               const HighlightState& stateIn,
                               QVector<StyleSpan>& spans,
                               HighlightState& stateOut) const;

    virtual const QVector<TextAttribute>& attributes() const = 0;
};
```

`StyleSpan::start` / `length` must be documented to mean **byte offsets when
produced by tokenizeBytes**, QChar indices when produced by highlightLine.
Callers pick one path and stay there.

Default implementation (in IHighlighter.cpp or a free function) roughly:

```cpp
void IHighlighter::tokenizeBytes(const char *data, qsizetype len,
                                 const HighlightState& stateIn,
                                 QVector<StyleSpan>& spans,
                                 HighlightState& stateOut) const {
    spans.clear();
    stateOut = stateIn;
    qsizetype i = 0;
    while (i < len) {
        qsizetype eol = i;
        while (eol < len && data[eol] != '\n') eol++;
        const QString line = QString::fromUtf8(data + i, eol - i);
        QVector<StyleSpan> lineSpans;
        HighlightState before = stateOut;
        highlightLine(line, before, lineSpans, stateOut);
        // Map QChar columns back to byte offsets within [i, eol):
        // build a QChar→byte table by re-scanning the line's UTF-8.
        // See notes in §2.3 for the mapping helper.
        for (const auto& s : lineSpans) {
            spans.append({ i + byteForQChar(line, s.start),
                           byteForQChar(line, s.start + s.length)
                             - byteForQChar(line, s.start),
                           s.attributeId });
        }
        i = eol + (eol < len ? 1 : 0);
    }
}
```

### 2.2 · `RulesHighlighter::tokenizeBytes` override

`RulesHighlighter` is the interesting case because its rule matching is
almost all ASCII (DetectChar, Detect2Chars, StringDetect, AnyChar, …).

A native byte override:

- Find line boundaries by scanning for `\n` (or whatever Kate's line-end
  rule says).
- For each line, run the rule loop directly on the UTF-8 bytes.
  - DetectChar / Detect2Chars / AnyChar / StringDetect / WordDetect /
    LineContinue → trivial on bytes.
  - RegExpr → QRegularExpression needs a QString. Either keep per-line
    QString conversion only when a regex rule is reachable in the active
    context, or require Kate files that hit the byte path to avoid
    RegExpr rules in hot paths.
  - Keyword → same byte-level lookup; case-insensitive list uses the
    byte-preserving lowercase fold the viewer already ships with
    `vl::Searcher` (see `logic/Searcher.cpp::foldByteRange`), or a
    KeywordList::lowercasedByteForms preprocess.
  - DetectIdentifier / DetectSpaces / Int / Float / HlC* → byte-level
    operators.

For the MVP the default line-splitting wrapper from §2.1 is enough to
make Kate files work in the viewer; the native RulesHighlighter override
is an optional follow-up.

### 2.3 · Export a QChar↔byte mapping helper

Spans produced by `highlightLine` are in QChar columns; byte consumers
need them in UTF-8 bytes. A single helper avoids every viewer-shaped
project re-implementing it:

```cpp
// qce/Utf8Map.h
namespace qce {

struct Utf8Map {
    // Build from a UTF-8 byte range. O(n).
    void buildFromUtf8(const char *data, qsizetype len);

    qsizetype byteForQChar(int qcharIndex) const;
    int       qcharForByte(qsizetype byteOffset) const;
};

}
```

### 2.4 · Relax coupling where cheap

Nothing that breaks API; low-priority cosmetics:

- `HighlightState::contextStack` — replace `QVector<int>` with
  `QList<int>` (same thing in Qt6 — already QList) and add a small
  inline-vector optimisation if profiling shows allocation pressure.
- `StyleSpan` — it's already a POD; no change needed.
- `TextAttribute` — keeps QColor; no std replacement makes sense.

---

## 3 · Acceptance tests

Add to qcodeedit's test suite:

1. **Round-trip** — for a hand-written highlighter that only implements
   `highlightLine`, the default `tokenizeBytes` produces spans whose
   byte ranges decode back to the same QChar ranges as the per-line
   call.
2. **Line-split invariance** — tokenising a multi-line UTF-8 buffer as
   one range equals concatenating per-line tokenisations at every `\n`.
3. **Resumability** — splitting the byte buffer at any boundary and
   threading `stateOut → stateIn` yields the same spans as one-shot.
   (Property test, sample 50 split points.)
4. **Kate XML regression** — load `xml.xml` from your
   `qcodeedit-kate` package, tokenise `books.xml` (copy from
   `textviewers/test/data/`), assert span-set stable.

---

## 4 · Migration on the viewer side

Once qce 1.2 ships with `tokenizeBytes`:

- Remove `logic/Highlighter.h`, `logic/XmlHighlighter.{h,cpp}`,
  `test/testXmlHighlighter.cpp`.
- `wid/PaintArea::setHighlighter` takes `qce::IHighlighter*` and
  `qce::QVector<TextAttribute>`.
- `wid/HighlightColors` either disappears (use qce's TextAttribute
  directly) or becomes a thin owner of the attribute palette.
- `MainWindow::Highlighting` menu changes to a "Load Kate file…"
  action (QFileDialog → `KateXmlReader::load(path)` →
  `widget->setHighlighter(...)`). Library consumers
  (Gemini-Commander) pass their own.
- Paint loop calls `m_highlighter->tokenizeBytes(buf, bufLen,
  initialState(), spans, endState)`. Everything else stays the
  same — span starts are already byte offsets.

Expected diff size: **−700 lines** (highlighter + FSM + tests),
**+30 lines** in paint + demo.

---

## 5 · Open questions for the qce side

1. **RegExpr inside byte path** — convert the line on demand (per-rule
   fallback) or forbid at config time with an error? Kate XML files use
   RegExpr heavily for numbers; forbidding would cripple Python.
2. **Folding provider (`RuleBasedFoldingProvider`)** — presumably stays
   QChar-based; viewer doesn't fold. No change needed.
3. **`KateXmlReader` dependency** — can it stay unchanged, since its
   output (`RulesHighlighter`) remains valid as long as
   `RulesHighlighter::tokenizeBytes` is added? Should be yes.

---

## 6 · Suggested commit sequence in qcodeedit

1. Add `qce::Utf8Map` helper + tests (§2.3).
2. Add default `IHighlighter::tokenizeBytes` that uses `Utf8Map`. Tests
   §3.1 and §3.2.
3. Override `tokenizeBytes` in `RulesHighlighter` with direct byte
   matching for ASCII rule kinds; fall back to QString for regex-bearing
   contexts. Test §3.3, §3.4.
4. Bump qce version to 1.2.
5. Re-install system package, consume from textviewers (§4).

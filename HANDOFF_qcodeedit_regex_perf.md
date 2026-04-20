# Handoff — regex hot path in qce::RulesHighlighter

Fourth handoff. Ships after qce 1.3 (byte-range highlighter,
dynamic rules, bounded folding — already consumed). Purpose:
`qce::RulesHighlighter::matchAt` spends ~90% of its time in
`pcre2_match_16` on realistic Kate XML inputs, which dominates
both `tokenizeBytes` and `foldersInBytes` for the viewer and
makes large XML files scroll-laggy.

Reference points in this viewer repo:

- Reproducer: `examples/viewer/textViewer` loaded with a
  Wiktionary-scale XML (the dumps at
  `https://dumps.wikimedia.org/enwiktionary/` start around
  11 GB uncompressed) + `~/.local/share/org.kde.syntax-highlighting/syntax/xml.xml`.
- `wid/PaintArea.cpp::paintHighlighted` — the caller, already
  caches `foldersInBytes` output, bounds tokenize width and
  reuses markers across small scrolls; none of that moves the
  needle once a cache miss fires because the rescan itself is
  regex-bound.

---

## 1 · Measurement

`perf record --call-graph dwarf` on a Release+`-g` build of the
viewer, scrolling for a few seconds in the 11 GB file:

```
Overhead  Shared Object         Symbol
  11.47%  libpcre2-16.so.0      [.] 0x…6c70f
  11.38%  libpcre2-16.so.0      [.] 0x…6c700
  11.36%  libpcre2-16.so.0      [.] 0x…6c715
  11.35%  libpcre2-16.so.0      [.] 0x…6c71a
  11.28%  libpcre2-16.so.0      [.] 0x…6c70c
  11.23%  libpcre2-16.so.0      [.] 0x…6c703
  11.20%  libpcre2-16.so.0      [.] 0x…6c707
  11.16%  libpcre2-16.so.0      [.] 0x…6c713
   1.57%  libpcre2-16.so.0      [.] pcre2_match_16
```

All top nine callers resolve back to
`QRegularExpression::match(QString, …) const` called from
`qce::RulesHighlighter::matchAt` (inlined, address not
symbolised because qcodeedit was a stripped Release build at
the time of the capture).

At a Wiktionary-like workload that means we are paying ~1 pcre
match per byte inside an element body, because xml.xml's
`<context name="Element">` ends with

```xml
<RegExpr attribute="Error" context="#stay" String="\S" />
```

i.e. "anything non-whitespace I didn't match above", evaluated
through the full regex engine for every character of every
value, every attribute name, every tag name. Running the xml.xml
pass on a 4 MB scan window (2× `foldMaxBytes = 1 MiB` of slack)
takes long enough to be felt as scroll lag.

---

## 2 · Proposed fixes, cheap first

### 2.1 · Fast path for trivial regex shapes

Recognise a handful of tiny regex shapes at rule-compile time
and dispatch them without going through pcre:

| Regex source | Semantics | Replacement |
|---|---|---|
| `\s` / `\s+` | whitespace(s) | `QChar::isSpace` scan |
| `\S` / `\S+` | non-whitespace | `!QChar::isSpace` scan |
| `[a-zA-Z]+`, `[a-zA-Z_]\w*` | identifier | same as `DetectIdentifier` |
| `[0-9]+` | digits | same as `Int` |
| `[0-9a-fA-F]+` | hex digits | simple loop |
| literal-only (no metacharacters, no alternation) | string match | `StringDetect` semantics |

At rule-compile time (inside the XML reader or the first call
to `matchAt`) classify `rule.regex` into one of these buckets
and store the bucket ID in `HighlightRule`. `matchAt`'s
`RegExpr` arm checks the bucket first and only falls back to
`QRegularExpression::match` for rules that don't fit.

The `\S` case alone kills the hot loop above — it is the rule
that dominates xml.xml's profile.

### 2.2 · Skip `matchAt` when the rule can't possibly match

For every rule that starts with a known non-meta character
(`DetectChar`, `Detect2Chars`, `StringDetect` with non-empty
prefix, many `RegExpr` with a literal prefix), build a
per-context dispatch table keyed on the first byte of the
current line position. Rules whose first character doesn't
match the byte at `pos` are skipped without entering
`matchAt` at all. For contexts with many rules (Kate's C++
definition has dozens) this is a big cut even before the
regex-shape work.

### 2.3 · Reuse `QRegularExpressionMatch` scratch space

`QRegularExpression::match` allocates a fresh
`QRegularExpressionMatchPrivate` per call. Keeping a thread-local
scratch match around for the engine (or switching to
`QRegularExpression::match(..., QRegularExpressionMatchType,
QRegularExpression::DontCheckSubjectStringMatchOption)` where
applicable) avoids the per-call heap traffic. Smaller win than
2.1/2.2 but free to add.

### 2.4 · Optional: pcre2 JIT

`QRegularExpression::optimize()` triggers the pcre2 JIT compile.
Calling it once per rule at load time (after
`QRegularExpression::setPattern`) can double throughput on
non-trivial regexes. Safe to add behind a flag.

---

## 3 · Expected impact

Viewer scroll on the 11 GB Wiktionary dump with `xml.xml`:

- §2.1 alone removes ~90% of the pcre time (Element context's
  `\S` rule is the entire top-9 of the profile). Fold-scan on
  a 4 MB window drops from hundreds of milliseconds into the
  tens.
- §2.2 piles onto it and helps languages with big rule lists
  (C++, JS) even more than XML.
- §2.3, §2.4 are polish.

---

## 4 · Acceptance tests

1. **Regression on existing corpus** — any Kate XML in the qce
   test set must produce identical spans after §2.1/§2.2. This
   is a pure performance change; output is contractually
   unchanged.
2. **Fast-path coverage** — unit test that each of the shape
   classes in the table above matches exactly like
   `QRegularExpression::match` on a battery of inputs
   (including UTF-8 edge cases, empty strings, and strings
   longer than `QString::maxSize()`).
3. **Benchmark** — add a google-benchmark or similar that
   measures `tokenizeBytes` throughput on a fixed 1 MB XML
   sample with xml.xml. Assert post-change throughput is at
   least 3× pre-change (target: 5×).

---

## 5 · Deferred / out of scope

- Replacing pcre2 with a hand-rolled DFA engine. Kate regexes
  use features (backreferences, lookaround) that aren't trivial
  to support, and the fast paths in §2 cover the common cases
  without that work.
- Changing xml.xml / html.xml / other Kate files to avoid `\S`.
  The syntax library is upstream; we don't want to fork it.
- Parallelising tokenisation across threads. The byte-range API
  is already per-chunk; callers could parallelise, but the
  single-thread win from §2 is big enough on its own.

---

## 6 · Suggested commit sequence in qcodeedit

1. Add rule-shape classification at load/first-match time; fast
   paths for `\S`, `\s`, `[a-zA-Z_]\w*`, `[0-9]+`, `[0-9a-fA-F]+`
   and literal-only patterns. Test §4.2.
2. Per-context first-byte dispatch table; skip rules whose first
   char can't match. Test §4.1 regression.
3. Reuse match scratch + `QRegularExpression::optimize()`
   behind-the-flag.
4. Benchmark harness (§4.3). Bump qce to 1.4.

Viewer side needs no change once qce 1.4 ships; the perf gain
propagates through `tokenizeBytes` and `foldersInBytes`
transparently.

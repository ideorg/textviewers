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

`perf record --call-graph dwarf` on a RelWithDebInfo build of
qcodeedit 1.3 + the viewer, scrolling for a few seconds inside
the 11 GB file (xml.xml loaded):

```
Overhead  Shared Object          Symbol
 16.47%   libpcre2-16.so         [.] pcre2_match_16
  7.06%   textViewer             [.] qce::RulesHighlighter::matchAt
  5.16%   textViewer             [.] qce::RulesHighlighter::highlightLineEx
  4.97%   libc.so.6              [.] _int_malloc
  3.46%   libc.so.6              [.] malloc
  3.19%   libc.so.6              [.] cfree
  2.24%   libQt6Core             [.] QRegularExpression::match(...)
  1.53%   libc.so.6              [.] _int_free_merge_chunk
  1.35%   libc.so.6              [.] _int_free_chunk
  1.34%   libpcre2-16.so         [.] pcre2_match_data_free_16
  1.33%   libc.so.6              [.] __libc_malloc2
  1.20%   textViewer             [.] qce::Utf8Map::buildFromUtf8
  1.17%   libQt6Core             [.] QRegularExpression::QRegularExpression()
  1.14%   libstdc++              [.] operator new(unsigned long)
  1.10%   libQt6Core             [.] QExplicitlySharedDataPointer<QRegularExpressionPrivate>::~QExplicitlySharedDataPointer()
  ~9%     libpcre2-16.so         [.] (other unresolved addresses inside pcre2_match_16)
```

Two independent cost centres, about the same size:

- **Regex evaluation** — roughly **30-40%** of cycles are in
  `pcre2_match_16` and its internal dispatch. The dominant
  caller is `qce::RulesHighlighter::matchAt → QRegularExpression::match`,
  triggered from `qce::RuleBasedFoldingProvider::foldersInBytes`
  (and `tokenizeBytes` when a cache miss hits). For xml.xml the
  dominant pattern is the Element context's

  ```xml
  <RegExpr attribute="Error" context="#stay" String="\S" />
  ```

  i.e. "any non-whitespace character that no earlier rule in
  this context consumed", evaluated through pcre for every byte
  inside a tag.

- **Allocator churn** — roughly another **15-20%** of cycles
  are in `malloc`/`free`/`operator new`, overwhelmingly from
  two paths:

  1. **Per-match pcre allocation.** `QRegularExpression::match`
     calls `pcre2_match_data_create_from_pattern` on every call
     and `pcre2_match_data_free_16` on every return
     (`_int_malloc` as a child of `pcre2_match_16` itself is
     already 3.69% of total).

  2. **`QRegularExpression` construction and destruction on the
     hot path.** 1.17% of total cycles are in the
     `QRegularExpression()` constructor called from
     `QRegularExpression::isValid()` called from
     `qce::RulesHighlighter::matchAt`. A symmetric 1.10% sits
     in the `QExplicitlySharedDataPointer<QRegularExpressionPrivate>`
     destructor. Someone in the `RegExpr` arm of `matchAt` is
     default-constructing (or copying) a QRegularExpression per
     call, probably as an intermediate.

  These two allocator paths alone account for more cycles than
  any single regex's execution. Reducing regex calls (§2.1/§2.2)
  helps but does not make the alloc path go away; the engine
  side needs its own pass.

- **Utf8Map rebuild** — `qce::Utf8Map::buildFromUtf8` is 1.2%,
  called from `IHighlighter::tokenizeBytes`'s default
  implementation once per line. Cheap per call but multiplied
  by the line count of a Wiktionary fold-scan window.

---

## 2 · Proposed fixes, cheap first

### 2.0 · Stop allocating per match

Two quick wins that make no semantic change:

1. **Don't default-construct `QRegularExpression` inside the
   `RegExpr` arm of `matchAt`.** Whatever path
   `QRegularExpression::QRegularExpression()` +
   `QRegularExpression::isValid()` is taking (1.17% + 1.10% +
   their children) should be replaced by a direct check on
   `rule.regex.isValid()` without constructing anything new. If
   it's a copy into a local (e.g.
   `QRegularExpression re = rule.regex;`), switch to a
   `const QRegularExpression &`.

2. **Reuse `QRegularExpressionMatch` scratch.** Today each call
   to `rule.regex.match(line, pos, …)` returns a fresh match
   object whose destruction frees pcre2_match_data; that
   malloc/free shows up as ~15% of the whole profile. Options:

   - Keep a single `QRegularExpressionMatch` owned by the
     highlighter (or per-context), reuse its storage by calling
     a variant that accepts a pre-allocated match. Qt 6.8+ has
     `QRegularExpression::matchView(QStringView)` overloads but
     not yet a public "match into this object" API. Workaround
     is to call pcre2 directly and hold the `pcre2_match_data`
     long-lived per rule.
   - Cheaper alternative: cache the compiled
     `QRegularExpressionPrivate` (via `QRegularExpression::isValid`
     trigger at load time once) and ensure JIT is on; allocator
     churn is then bounded by reuse inside pcre when it can.

Implementing either item 1 or 2 independently is a 1-3 day job
and removes ~10-15% of total cycles immediately, no behavioural
change to any rule.

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

- §2.0 alone should remove ~15% of cycles (allocator path) with
  zero semantic change. The biggest single win per line of code.
- §2.1 removes ~30% (the `\S` fast path on the Element context
  is essentially the Wiktionary hot loop).
- §2.2 piles onto it and helps languages with big rule lists
  (C++, JS) even more than XML.
- §2.3, §2.4 are polish.
- Utf8Map caching (see §5) can squeeze another ~1%.

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

1. **Eliminate stray QRegularExpression construction inside
   `matchAt`** (§2.0 item 1). Verify with a fresh
   perf recording that the `QRegularExpression::QRegularExpression()`
   entry disappears from the profile. Tiny diff, big win.
2. **Share `pcre2_match_data` / `QRegularExpressionMatch`
   scratch across calls** (§2.0 item 2). Combined with §2.4
   (JIT on `optimize()`) these remove the allocator-dominated
   path.
3. Add rule-shape classification at load/first-match time; fast
   paths for `\S`, `\s`, `[a-zA-Z_]\w*`, `[0-9]+`, `[0-9a-fA-F]+`
   and literal-only patterns. Test §4.2.
4. Per-context first-byte dispatch table; skip rules whose first
   char can't match. Test §4.1 regression.
5. Benchmark harness (§4.3). Bump qce to 1.4.

Viewer side needs no change once qce 1.4 ships; the perf gain
propagates through `tokenizeBytes` and `foldersInBytes`
transparently.

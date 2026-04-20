# Handoff — support Kate dynamic rules in qce::RulesHighlighter

Third handoff (after the byte-range highlighter, after folding).
`RulesHighlighter` in qcodeedit 1.2 ignores Kate's `dynamic="true"`
attribute, which cripples `xml.xml` (and any other syntax that closes
a region using the name matched when it opened).

Reference points in this viewer repo:

- Reproducer: `examples/viewer/textViewer` opens `books.xml` by default.
  Menu → Highlighting → Load Kate XML… → pick
  `~/.local/share/org.kde.syntax-highlighting/syntax/xml.xml`.
- What you see: every opening tag past `<catalog>` renders in
  `dsError` (pure red bold); element names render as `dsNormal`.

---

## 1 · Symptom on xml.xml

Dumping `tokenizeBytes` output for `books.xml` (palette attribute ids
in brackets match `itemDatas` order of xml.xml — 10 = Element, 11 =
Element Symbols, 14 = Value, 17 = Error):

```
  [  22.. 23) attr=11 "<"            ← Element Symbols, OK
  [  23.. 31) attr= 1 "catalog>"     ← Other Text, wrong (expected
                                       10 Element + 11 Element Symbols)
  [  36.. 41) attr=17 "<book"        ← Error, pure red bold
```

Everything from the first non-PI opening tag onward is wrong.

## 2 · Root cause

`xml.xml` identifies the element name with a look-ahead regex and then
consumes it in a dynamic `StringDetect`:

```xml
<context name="FindXML" ...>
    <RegExpr attribute="Element Symbols" context="ElementTagName"
             String="&lt;(?=(&name;))" beginRegion="element" />
    …
</context>
<context name="ElementTagName" ...>
    <StringDetect attribute="Element" context="#pop!Element"
                  String="%1" dynamic="true" />
</context>
```

The `%1` in `String="%1"` is a placeholder: Kate expands it to the
first capture group of the regex that triggered the context switch
(`&name;` = the element name). `dynamic="true"` signals that the
string needs expansion per match.

qce's `RulesHighlighter::matchAt` treats the rule as a literal
StringDetect:

```cpp
case HighlightRule::StringDetect: {
    const int n = rule.str.size();
    if (remaining < n) return 0;
    const auto cs = rule.caseSensitive ? Qt::CaseSensitive : Qt::CaseInsensitive;
    return (QStringView(line).mid(pos, n).compare(rule.str, cs) == 0) ? n : 0;
}
```

It searches for the literal two characters `"%1"`, which the source
never contains. The rule never matches, `ElementTagName` never pops
into `Element`, the context stack stays on `ElementTagName` for the
rest of the line, eventually hitting the fallthrough/lineEnd that
lands us in `Element` on the next line — where every tag-start `<`
matches `RegExpr \S` and gets `attr=Error`.

The same mechanism powers `context="El End TagName"` /
`StringDetect String="%1" dynamic="true"` that matches the closing
tag's name; C++ and most other Kate definitions don't use dynamic
rules, which is why `cpp.xml` works.

## 3 · Proposed change

### 3.1 · Track captures across context switches

Extend the triggering regex step so that its capture groups are
stored alongside the new context on the stack:

```cpp
struct HighlightState {
    QVector<int> contextStack;
    QVector<QStringList> captureStack;   // NEW; size matches contextStack
};
```

When a rule with `nextContextId >= 0` pushes a new context, record
the capture groups of the rule's match (empty list for non-regex
rules). Pop discards the corresponding capture list. `operator==`
compares both stacks so editor caches still work.

### 3.2 · Mark rules as dynamic

Add one field to `HighlightRule`:

```cpp
struct HighlightRule {
    …
    bool dynamic = false;   // NEW; set by reader when the XML has dynamic="true"
};
```

`KateXmlReader` copies the attribute unchanged.

### 3.3 · Expand `%1`, `%2`, … at match time

In `matchAt`, for `StringDetect` and `RegExpr` whose `dynamic` is
true, replace occurrences of `%n` in `rule.str` with
`captureStack.last()[n-1]` before comparing. For `RegExpr` with
dynamic, rebuild a `QRegularExpression` per match (or cache by
(ruleId, capture tuple) if this shows up in profiles).

Numbering follows Kate: `%1` is the first capture, `%0` is the
whole match (rare in practice).

### 3.4 · Kinds that need dynamic support

Per Kate's format:

- `StringDetect` — the one `xml.xml` and `html.xml` need.
- `RegExpr` — less common but used by e.g. LaTeX-ish syntaxes.
- `Detect2Chars`, `DetectChar` — never dynamic in practice; skip.

## 4 · Acceptance tests

1. **xml.xml tag pair** — tokenise `<foo>hello</foo>` with xml.xml;
   expect spans for `<`, `foo`, `>`, `hello`, `</`, `foo`, `>` at the
   right attribute ids. Span for `foo` inside the opener must be
   `Element` (not `Other Text`), and span for `foo` inside the closer
   must be `Element` too.
2. **Nested unrelated names** — `<a><b></a></b>` (malformed):
   after `</a>` the closing-name consumer expects `a`, not `b`; the
   rule must fail on `b`, not eat it as if it were `a`.
3. **Capture depth** — stack push/pop across nested contexts must
   not cross-pollute captures. Test with `<a attr="x">` entering
   attribute context while element context still holds the `a`
   capture for its eventual closer.
4. **State equality cache** — identical captureStack states compare
   equal; distinct ones do not. CodeEditArea's incremental
   re-highlight must still stop at matching states.

## 5 · Migration on the viewer side

None. The viewer already consumes `qce::IHighlighter` and the
palette carries the right colors; once xml.xml stops emitting
`attr=Error` for opening tags, the render is correct without any
code change here.

The reproducer (`books.xml` default + xml.xml load) is a smoke
test that ships with the viewer; after the fix its screen is
predominantly black/green/dark-red instead of bright-red.

## 6 · Suggested commit sequence in qcodeedit

1. Extend `HighlightState` with `captureStack`; update `operator==`.
   Make sure the editor's cache path still works (test §4.4).
2. Add `HighlightRule::dynamic`. Wire the reader to set it when the
   XML carries `dynamic="true"`.
3. Track captures on context push in `highlightLineEx`; discard on
   pop.
4. Implement `%n` expansion in `matchAt` for `StringDetect` (test
   §4.1) and `RegExpr` (test §4.2).
5. Bump qce to 1.3.

Folding (HANDOFF_qcodeedit_folding.md) and dynamic rules are
independent; either can land first. The viewer benefits from both.

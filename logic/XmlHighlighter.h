#pragma once

#include "Highlighter.h"

namespace vl {

// Attribute ids emitted by XmlHighlighter. The widget layer owns the palette
// that turns these into colours.
enum XmlAttr {
    XmlDefault = 0,  // text outside any markup
    XmlBracket,      // <, >, /, </, <?, ?>, <!
    XmlElement,      // element name right after <
    XmlAttrName,     // attribute name inside a tag
    XmlValue,        // quoted attribute value, including the quotes
    XmlComment,      // <!-- ... -->, entire run
    XmlDoctype,      // <!DOCTYPE ...>, <![CDATA[...]]>, generic <!...>
    XmlProcessing,   // <?xml ...?>
    XmlAttrCount
};

// Hand-rolled XML highlighter. Not a full XML parser - it just colours the
// obvious lexical shapes: comments, processing instructions, doctype, tags
// with element / attribute / value runs, and the angle brackets themselves.
// Enough for the typical MediaWiki dump the viewer is aimed at; we can swap
// in a rules engine later without breaking callers.
class XmlHighlighter : public IHighlighter {
public:
    HighlightState initialState() const override;
    void tokenize(const char *data,
                  int64_t len,
                  const HighlightState &stateIn,
                  std::vector<StyleSpan> &spansOut,
                  HighlightState &stateOut) const override;
    int attributeCount() const override { return XmlAttrCount; }
};

}

#pragma once

#include <cstdint>
#include <string_view>
#include <vector>

namespace vl {

// Visual style slot. The highlighter picks an attributeId for each byte range;
// a separate palette (defined by the widget layer) maps attributeId -> color.
struct StyleSpan {
    int64_t start;        // byte offset within the tokenize buffer
    int64_t length;       // bytes
    int     attributeId;  // index into the highlighter's palette
};

// Between-chunk state. Modelled as a stack of context ids so that rule-based
// highlighters can represent nested states (string inside macro, ...). A plain
// FSM highlighter keeps a single int on top and treats the stack as scalar.
// Opaque to the caller except for operator==.
using HighlightState = std::vector<int>;

// Abstract highlighter. Tokenises a contiguous byte range starting in some
// state and reports every byte's attribute as a compact span list. Designed to
// run from a synthetic prelude, so one can start at an arbitrary file offset
// (as long as a few kilobytes of context are fed in first to establish state).
class IHighlighter {
public:
    virtual ~IHighlighter() = default;

    // State at the logical beginning of a document.
    virtual HighlightState initialState() const = 0;

    // Tokenise [data, data+len). stateIn is the state returned by the previous
    // tokenize() call (or initialState() for the first chunk). Appends
    // non-overlapping, sorted spans to spansOut (caller clears if needed) and
    // writes the end-of-buffer state to stateOut. Byte positions in spansOut
    // are offsets within [data, data+len).
    virtual void tokenize(const char *data,
                          int64_t len,
                          const HighlightState &stateIn,
                          std::vector<StyleSpan> &spansOut,
                          HighlightState &stateOut) const = 0;

    // Number of palette entries used by this highlighter (0 .. attributeCount()-1).
    virtual int attributeCount() const = 0;
};

}

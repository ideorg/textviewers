#include "XmlHighlighter.h"

#include <cctype>
#include <cstdint>

namespace vl {

namespace {

// Pure byte-by-byte FSM: every character consumes at most one byte, and every
// transition decision needs only the current state + current byte. That way
// splitting the buffer at any boundary yields the same result as one-shot
// tokenisation (state carries all context).
enum XmlState {
    StateNormal = 0,
    StateAfterLt,          // just consumed '<'
    StateAfterLtBang,      // just consumed '<!'
    StateAfterLtBangDash,  // just consumed '<!-'
    StateInElementName,    // reading element name (after '<' or '</')
    StateInTag,            // inside tag, between attrs
    StateInAttrName,       // reading attribute identifier
    StateInValueDouble,    // reading "..."
    StateInValueSingle,    // reading '...'
    StateInComment,        // inside <!-- ... -->
    StateAfterCommentDash, // saw one '-' inside comment
    StateAfterCommentDashDash, // saw '--' inside comment
    StateInPI,             // inside <? ... ?>
    StateAfterPIQ,         // saw '?' inside PI
    StateInDoctype,        // inside <! ... > (DOCTYPE, CDATA, ...)
};

bool isNameChar(uint8_t c) {
    // XML name characters (simplified): letter, digit, - _ . : plus any high
    // UTF-8 byte so multibyte names aren't split.
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
           (c >= '0' && c <= '9') || c == '-' || c == '_' ||
           c == '.' || c == ':' || c >= 0x80;
}

void compressSpans(const std::vector<uint8_t> &byteAttr,
                   int64_t len,
                   std::vector<StyleSpan> &out) {
    int64_t k = 0;
    while (k < len) {
        int64_t m = k + 1;
        uint8_t attr = byteAttr[k];
        while (m < len && byteAttr[m] == attr) m++;
        if (!out.empty()) {
            auto &last = out.back();
            if (last.attributeId == attr && last.start + last.length == k) {
                last.length += (m - k);
                k = m;
                continue;
            }
        }
        out.push_back({k, m - k, attr});
        k = m;
    }
}

}

HighlightState XmlHighlighter::initialState() const {
    return {StateNormal};
}

void XmlHighlighter::tokenize(const char *data,
                              int64_t len,
                              const HighlightState &stateIn,
                              std::vector<StyleSpan> &spansOut,
                              HighlightState &stateOut) const {
    int state = stateIn.empty() ? StateNormal : stateIn.back();
    auto s = reinterpret_cast<const uint8_t *>(data);
    std::vector<uint8_t> attr((size_t) len, (uint8_t) XmlDefault);

    int64_t i = 0;
    while (i < len) {
        uint8_t c = s[i];
        bool consume = true;
        switch (state) {
            case StateNormal:
                if (c == '<') {
                    attr[i] = XmlBracket;
                    state = StateAfterLt;
                } else {
                    attr[i] = XmlDefault;
                }
                break;
            case StateAfterLt:
                if (c == '!') {
                    attr[i] = XmlBracket;
                    state = StateAfterLtBang;
                } else if (c == '?') {
                    if (i - 1 >= 0) attr[i - 1] = XmlProcessing;
                    attr[i] = XmlProcessing;
                    state = StateInPI;
                } else if (c == '/') {
                    attr[i] = XmlBracket;
                    state = StateInElementName;
                } else if (isNameChar(c)) {
                    attr[i] = XmlElement;
                    state = StateInElementName;
                } else {
                    attr[i] = XmlDefault;
                    state = StateNormal;
                }
                break;
            case StateAfterLtBang:
                if (c == '-') {
                    attr[i] = XmlBracket;
                    state = StateAfterLtBangDash;
                } else {
                    if (i - 2 >= 0) attr[i - 2] = XmlDoctype;
                    if (i - 1 >= 0) attr[i - 1] = XmlDoctype;
                    attr[i] = XmlDoctype;
                    state = StateInDoctype;
                }
                break;
            case StateAfterLtBangDash:
                if (c == '-') {
                    if (i - 3 >= 0) attr[i - 3] = XmlComment;
                    if (i - 2 >= 0) attr[i - 2] = XmlComment;
                    if (i - 1 >= 0) attr[i - 1] = XmlComment;
                    attr[i] = XmlComment;
                    state = StateInComment;
                } else {
                    if (i - 3 >= 0) attr[i - 3] = XmlDoctype;
                    if (i - 2 >= 0) attr[i - 2] = XmlDoctype;
                    if (i - 1 >= 0) attr[i - 1] = XmlDoctype;
                    attr[i] = XmlDoctype;
                    state = StateInDoctype;
                }
                break;
            case StateInElementName:
                if (isNameChar(c)) {
                    attr[i] = XmlElement;
                } else {
                    state = StateInTag;
                    consume = false;
                }
                break;
            case StateInTag:
                if (c == '>') {
                    attr[i] = XmlBracket;
                    state = StateNormal;
                } else if (c == '/') {
                    attr[i] = XmlBracket;
                } else if (c == '"') {
                    attr[i] = XmlValue;
                    state = StateInValueDouble;
                } else if (c == '\'') {
                    attr[i] = XmlValue;
                    state = StateInValueSingle;
                } else if (isNameChar(c)) {
                    attr[i] = XmlAttrName;
                    state = StateInAttrName;
                } else {
                    attr[i] = XmlDefault;
                }
                break;
            case StateInAttrName:
                if (isNameChar(c)) {
                    attr[i] = XmlAttrName;
                } else {
                    state = StateInTag;
                    consume = false;
                }
                break;
            case StateInValueDouble:
                attr[i] = XmlValue;
                if (c == '"') state = StateInTag;
                break;
            case StateInValueSingle:
                attr[i] = XmlValue;
                if (c == '\'') state = StateInTag;
                break;
            case StateInComment:
                attr[i] = XmlComment;
                if (c == '-') state = StateAfterCommentDash;
                break;
            case StateAfterCommentDash:
                attr[i] = XmlComment;
                if (c == '-') state = StateAfterCommentDashDash;
                else state = StateInComment;
                break;
            case StateAfterCommentDashDash:
                attr[i] = XmlComment;
                if (c == '>') state = StateNormal;
                else if (c == '-') state = StateAfterCommentDashDash;
                else state = StateInComment;
                break;
            case StateInPI:
                attr[i] = XmlProcessing;
                if (c == '?') state = StateAfterPIQ;
                break;
            case StateAfterPIQ:
                attr[i] = XmlProcessing;
                if (c == '>') state = StateNormal;
                else if (c == '?') state = StateAfterPIQ;
                else state = StateInPI;
                break;
            case StateInDoctype:
                attr[i] = XmlDoctype;
                if (c == '>') state = StateNormal;
                break;
        }
        if (consume) i++;
    }

    compressSpans(attr, len, spansOut);
    stateOut = {state};
}

}

#include "gtest/gtest.h"
#include "logic/XmlHighlighter.h"

#include <string>

using namespace std;
using namespace vl;

namespace {

// Flatten spans into a string of attribute-id letters, one per byte, to make
// assertions readable. Byte i's letter is mapped from the attribute id of the
// span covering it.
string attrString(const string &src, const XmlHighlighter &h) {
    vector<StyleSpan> spans;
    HighlightState out;
    h.tokenize(src.data(), (int64_t) src.size(), h.initialState(), spans, out);
    string result(src.size(), '?');
    static const char letter[] = {
        'D',  // Default
        'B',  // Bracket
        'E',  // Element
        'A',  // AttrName
        'V',  // Value
        'C',  // Comment
        'T',  // Doctype
        'P',  // Processing
    };
    for (const auto &s : spans) {
        for (int64_t k = 0; k < s.length; k++)
            result[s.start + k] = letter[s.attributeId];
    }
    return result;
}

}

TEST(XmlHighlighter, simpleTag) {
    XmlHighlighter h;
    //              <foo>bar</foo>
    //              B    BD   B
    //               EEE    BBEEE
    EXPECT_EQ(attrString("<foo>bar</foo>", h),
              "BEEEBDDDBBEEEB");
}

TEST(XmlHighlighter, attributes) {
    XmlHighlighter h;
    string src = "<el a=\"1\" b='two'>";
    // <  el   ' ' a  =  "1"    ' ' b   =   'two'   >
    // B  EE       A   D VVV        A    D  VVVVV   B
    EXPECT_EQ(attrString(src, h),
              "BEEDADVVVDADVVVVVB");
}

TEST(XmlHighlighter, selfClosing) {
    XmlHighlighter h;
    EXPECT_EQ(attrString("<br/>", h), "BEEBB");
}

TEST(XmlHighlighter, closingTag) {
    XmlHighlighter h;
    EXPECT_EQ(attrString("</a>", h), "BBEB");
}

TEST(XmlHighlighter, comment) {
    XmlHighlighter h;
    EXPECT_EQ(attrString("<!-- hi -->", h), "CCCCCCCCCCC");
}

TEST(XmlHighlighter, processingInstruction) {
    XmlHighlighter h;
    EXPECT_EQ(attrString("<?xml version=\"1\"?>", h),
              "PPPPPPPPPPPPPPPPPPP");
}

TEST(XmlHighlighter, doctype) {
    XmlHighlighter h;
    EXPECT_EQ(attrString("<!DOCTYPE html>", h),
              "TTTTTTTTTTTTTTT");
}

TEST(XmlHighlighter, mixed) {
    XmlHighlighter h;
    string src = "text <e a=\"v\"> more";
    EXPECT_EQ(attrString(src, h),
              "DDDDDBEDADVVVBDDDDD");
}

TEST(XmlHighlighter, resumeAcrossChunk) {
    XmlHighlighter h;
    string full = "<foo attr=\"value-with->inside\">body</foo>";
    // Reference: one-shot tokenisation.
    vector<StyleSpan> refSpans;
    HighlightState refEnd;
    h.tokenize(full.data(), full.size(), h.initialState(), refSpans, refEnd);

    // Try every possible split point; results should match one-shot.
    for (size_t split = 0; split < full.size(); split++) {
        vector<StyleSpan> a, b;
        HighlightState mid, end;
        h.tokenize(full.data(), split, h.initialState(), a, mid);
        h.tokenize(full.data() + split, full.size() - split, mid, b, end);

        // Merge b into combined by shifting start.
        vector<StyleSpan> combined = a;
        for (auto s : b) {
            s.start += split;
            if (!combined.empty()) {
                auto &last = combined.back();
                if (last.attributeId == s.attributeId
                    && last.start + last.length == s.start) {
                    last.length += s.length;
                    continue;
                }
            }
            combined.push_back(s);
        }

        ASSERT_EQ(combined.size(), refSpans.size()) << "split=" << split;
        for (size_t i = 0; i < combined.size(); i++) {
            EXPECT_EQ(combined[i].start, refSpans[i].start) << "split=" << split;
            EXPECT_EQ(combined[i].length, refSpans[i].length) << "split=" << split;
            EXPECT_EQ(combined[i].attributeId, refSpans[i].attributeId) << "split=" << split;
        }
        EXPECT_EQ(end, refEnd) << "split=" << split;
    }
}

TEST(XmlHighlighter, commentSplitAcrossChunks) {
    XmlHighlighter h;
    string full = "pre<!-- long comment -->post";
    // Split inside the comment body, state must persist.
    vector<StyleSpan> a, b;
    HighlightState mid, end;
    h.tokenize(full.data(), 10, h.initialState(), a, mid);   // "pre<!-- lo"
    h.tokenize(full.data() + 10, full.size() - 10, mid, b, end);
    // Final state should be back to StateNormal (0).
    ASSERT_FALSE(end.empty());
    EXPECT_EQ(end.back(), 0);
}

//
// Collapsed ranges are skipped by ByteView, so the screen fills past them.
//
#include "gtest/gtest.h"
#include "logic/ByteDocument.h"
#include "logic/ByteView.h"

using namespace std;
using namespace vl;

namespace {
const string content = "a\n<x>\n1\n2\n3\n</x>\nb\nc\nd\n";

HiddenRanges foldX() {
    int64_t b = (int64_t) content.find("<x>");
    int64_t e = (int64_t) (content.find("</x>") + 4);
    return {{b, e}};
}

vector<u32string> rows(ByteView &v) {
    vector<u32string> r;
    for (size_t i = 0; i < v.size(); i++) r.push_back(v[(int) i]);
    return r;
}
}

class FoldTest : public ::testing::TestWithParam<int> {};

TEST_P(FoldTest, fillsPastCollapsed) {
    ByteDocument doc(content.c_str(), content.length(), 0);
    HiddenRanges hidden = foldX();
    ByteView v(&doc);
    v.setHiddenRanges(&hidden);
    v.setScreenLineLen(10);
    v.setScreenLineCount(4);
    v.setWrapMode(GetParam());
    v.fillDeque();
    v.recalcLines();
    vector<u32string> expect{U"a", U"<x>", U"b", U"c"};
    EXPECT_EQ(rows(v), expect);

    EXPECT_EQ(v.scrollDown(), 1);
    vector<u32string> down{U"<x>", U"b", U"c", U"d"};
    EXPECT_EQ(rows(v), down);
    EXPECT_EQ(v.scrollUp(), 1);
    EXPECT_EQ(rows(v), expect);
}

TEST_P(FoldTest, scrollUpOverCollapsed) {
    ByteDocument doc(content.c_str(), content.length(), 0);
    HiddenRanges hidden = foldX();
    ByteView v(&doc);
    v.setHiddenRanges(&hidden);
    v.setScreenLineLen(10);
    v.setScreenLineCount(2);
    v.setWrapMode(GetParam());
    v.gotoByte((int64_t) content.find("b\n"));
    v.fillDeque();
    v.recalcLines();
    vector<u32string> start{U"b", U"c"};
    EXPECT_EQ(rows(v), start);
    EXPECT_EQ(v.scrollUp(), 1);
    vector<u32string> up{U"<x>", U"b"};
    EXPECT_EQ(rows(v), up);
}

TEST_P(FoldTest, startInsideCollapsedShowsHeader) {
    ByteDocument doc(content.c_str(), content.length(), 0);
    HiddenRanges hidden = foldX();
    ByteView v(&doc);
    v.setHiddenRanges(&hidden);
    v.setScreenLineLen(10);
    v.setScreenLineCount(2);
    v.setWrapMode(GetParam());
    v.gotoByte((int64_t) content.find("2\n"));
    v.fillDeque();
    v.recalcLines();
    vector<u32string> expect{U"<x>", U"b"};
    EXPECT_EQ(rows(v), expect);
}

INSTANTIATE_TEST_SUITE_P(Fold, FoldTest, ::testing::Values(0, 1));

TEST(Fold, emptyDocument) {
    string empty;
    ByteDocument doc(empty.c_str(), 0, 0);
    ByteView v(&doc);
    v.setScreenLineLen(10);
    v.setScreenLineCount(3);
    v.fillDeque();
    v.recalcLines();
    EXPECT_EQ(v.size(), 1u);
    EXPECT_EQ(v.scrollDown(), 0);
}

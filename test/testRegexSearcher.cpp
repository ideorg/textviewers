#include "gtest/gtest.h"
#include "logic/RegexSearcher.h"
#include "logic/ByteDocument.h"

using namespace std;
using namespace vl;

namespace {
vector<int64_t> offsets(const vector<RegexMatch> &matches) {
    vector<int64_t> r;
    for (const auto &m : matches) r.push_back(m.offset);
    return r;
}
}

TEST(RegexSearcher, findNextAlternatives) {
    string content = "kojones x Coyones cojones";
    ByteDocument doc(content.c_str(), content.size());
    RegexSearcher s(&doc, "(c|k)o(j|y)ones");
    ASSERT_TRUE(s.isValid());
    auto r = s.findNext(0, nullptr);
    ASSERT_TRUE(r);
    EXPECT_EQ(0, r->offset);
    EXPECT_EQ(7, r->length);
    r = s.findNext(1, nullptr);
    ASSERT_TRUE(r);
    EXPECT_EQ(18, r->offset);
    EXPECT_FALSE(s.findNext(19, nullptr));
}

TEST(RegexSearcher, findPrev) {
    string content = "ab1 ab22 ab333";
    ByteDocument doc(content.c_str(), content.size());
    RegexSearcher s(&doc, "ab\\d+");
    auto r = s.findPrev((int64_t) content.size(), nullptr);
    ASSERT_TRUE(r);
    EXPECT_EQ(9, r->offset);
    EXPECT_EQ(5, r->length);
    // The match must end at or before endOffset.
    r = s.findPrev(8, nullptr);
    ASSERT_TRUE(r);
    EXPECT_EQ(4, r->offset);
    EXPECT_FALSE(s.findPrev(2, nullptr));
}

TEST(RegexSearcher, caseInsensitiveUnicode) {
    string content = "Żółw ŻÓŁW żółw";
    ByteDocument doc(content.c_str(), content.size());
    RegexSearcher sensitive(&doc, "żółw");
    EXPECT_EQ(1u, sensitive.findAll(0, (int64_t) content.size()).size());
    RegexSearcher insensitive(&doc, "żółw", {true, false});
    EXPECT_EQ(3u, insensitive.findAll(0, (int64_t) content.size()).size());
}

TEST(RegexSearcher, wholeWord) {
    string content = "cat concat cats cat";
    ByteDocument doc(content.c_str(), content.size());
    RegexSearcher s(&doc, "cat", {false, true});
    EXPECT_EQ((vector<int64_t>{0, 16}), offsets(s.findAll(0, (int64_t) content.size())));
}

TEST(RegexSearcher, anchorsAtLineEnds) {
    string content = "one two\nthree\nfour five";
    ByteDocument doc(content.c_str(), content.size());
    RegexSearcher s(&doc, "^\\w+$");
    auto all = s.findAll(0, (int64_t) content.size());
    ASSERT_EQ(1u, all.size());
    EXPECT_EQ(8, all[0].offset);
    EXPECT_EQ(5, all[0].length);
}

TEST(RegexSearcher, anchorsInsideRange) {
    // A range starting mid-line: ^ must not match at the range start.
    string content = "xxab\nab";
    ByteDocument doc(content.c_str(), content.size());
    RegexSearcher s(&doc, "^ab");
    EXPECT_EQ((vector<int64_t>{5}), offsets(s.findAll(2, (int64_t) content.size())));
}

TEST(RegexSearcher, invalidPattern) {
    string content = "abc";
    ByteDocument doc(content.c_str(), content.size());
    RegexSearcher s(&doc, "(c|k");
    EXPECT_FALSE(s.isValid());
    EXPECT_FALSE(s.errorString().empty());
    EXPECT_EQ(4, s.errorOffset());
    EXPECT_FALSE(s.findNext(0, nullptr));
    EXPECT_TRUE(s.findAll(0, 3).empty());
    RegexSearcher empty(&doc, "");
    EXPECT_FALSE(empty.isValid());
}

TEST(RegexSearcher, noEmptyMatches) {
    string content = "aa b aaa";
    ByteDocument doc(content.c_str(), content.size());
    RegexSearcher s(&doc, "a*");
    EXPECT_EQ((vector<int64_t>{0, 5}), offsets(s.findAll(0, (int64_t) content.size())));
}

TEST(RegexSearcher, matchesAcrossChunkEdges) {
    string content = "kojones x Coyones cojones";
    ByteDocument doc(content.c_str(), content.size());
    // Chunks of 4 bytes: every match crosses a chunk edge.
    RegexSearcher s(&doc, "o(j|y)ones", {}, 4);
    vector<int64_t> forward;
    int64_t pos = 0;
    while (auto m = s.findNext(pos, nullptr)) {
        forward.push_back(m->offset);
        pos = m->offset + m->length;
    }
    EXPECT_EQ((vector<int64_t>{1, 11, 19}), forward);
    vector<int64_t> backward;
    int64_t end = (int64_t) content.size();
    while (auto m = s.findPrev(end, nullptr)) {
        backward.push_back(m->offset);
        end = m->offset;
    }
    EXPECT_EQ((vector<int64_t>{19, 11, 1}), backward);
}

TEST(RegexSearcher, limits) {
    string content = "ab ab ab";
    ByteDocument doc(content.c_str(), content.size());
    RegexSearcher s(&doc, "ab");
    EXPECT_EQ(3, s.findNext(1, nullptr, 5).value_or(RegexMatch{-1, 0}).offset);
    EXPECT_FALSE(s.findNext(1, nullptr, 4));
    EXPECT_EQ(6, s.findPrev(8, nullptr, 6).value_or(RegexMatch{-1, 0}).offset);
    EXPECT_FALSE(s.findPrev(8, nullptr, 7));
}

TEST(RegexSearcher, cancel) {
    string content(100, 'x');
    ByteDocument doc(content.c_str(), content.size());
    RegexSearcher s(&doc, "y", {}, 10);
    int calls = 0;
    auto r = s.findNext(0, [&](int64_t, int64_t) { return ++calls < 3; });
    EXPECT_FALSE(r);
    EXPECT_EQ(3, calls);
}

#include "gtest/gtest.h"
#include "logic/Searcher.h"
#include "logic/ByteDocument.h"

using namespace std;
using namespace vl;

TEST(Searcher, findNextBasic) {
    string content = "Hello, world! Hello again.";
    ByteDocument doc(content.c_str(), content.size());
    Searcher s(&doc);
    auto r = s.findNext("Hello", 0, nullptr);
    ASSERT_TRUE(r);
    EXPECT_EQ(0, *r);
    r = s.findNext("Hello", 1, nullptr);
    ASSERT_TRUE(r);
    EXPECT_EQ(14, *r);
    r = s.findNext("Hello", 15, nullptr);
    EXPECT_FALSE(r);
}

TEST(Searcher, findNextNotFound) {
    string content = "Hello, world!";
    ByteDocument doc(content.c_str(), content.size());
    Searcher s(&doc);
    EXPECT_FALSE(s.findNext("xyz", 0, nullptr));
}

TEST(Searcher, findNextEmptyPattern) {
    string content = "Hello";
    ByteDocument doc(content.c_str(), content.size());
    Searcher s(&doc);
    EXPECT_FALSE(s.findNext("", 0, nullptr));
}

TEST(Searcher, findNextPatternLongerThanFile) {
    string content = "Hi";
    ByteDocument doc(content.c_str(), content.size());
    Searcher s(&doc);
    EXPECT_FALSE(s.findNext("Hello, world", 0, nullptr));
}

TEST(Searcher, findNextUtf8) {
    string content = "ala ma ąę kota";
    ByteDocument doc(content.c_str(), content.size());
    Searcher s(&doc);
    auto r = s.findNext("ąę", 0, nullptr);
    ASSERT_TRUE(r);
    EXPECT_EQ(content.find("ąę"), (size_t) *r);
}

TEST(Searcher, findNextCrossChunkBoundary) {
    // chunkSize 16; pattern straddles two chunks.
    string content(20, 'x');
    content[14] = 'n';
    content[15] = 'e';
    content[16] = 'e';
    content[17] = 'd';
    ByteDocument doc(content.c_str(), content.size());
    Searcher s(&doc, /*chunkSize=*/16);
    auto r = s.findNext("need", 0, nullptr);
    ASSERT_TRUE(r);
    EXPECT_EQ(14, *r);
}

TEST(Searcher, findNextStartOffsetMidPattern) {
    string content = "abcdef";
    ByteDocument doc(content.c_str(), content.size());
    Searcher s(&doc);
    // Match at offset 1; startOffset 2 is past match start, so no match.
    auto r = s.findNext("bcd", 2, nullptr);
    EXPECT_FALSE(r);
    r = s.findNext("bcd", 1, nullptr);
    ASSERT_TRUE(r);
    EXPECT_EQ(1, *r);
}

TEST(Searcher, findNextProgressCalled) {
    string content(200, 'a');
    content += "needle";
    ByteDocument doc(content.c_str(), content.size());
    Searcher s(&doc, /*chunkSize=*/32);
    int calls = 0;
    auto r = s.findNext("needle", 0, [&](int64_t, int64_t) {
        calls++;
        return true;
    });
    ASSERT_TRUE(r);
    EXPECT_GT(calls, 0);
    EXPECT_EQ(200, *r);
}

TEST(Searcher, findNextCancel) {
    string content(200, 'a');
    content += "needle";
    ByteDocument doc(content.c_str(), content.size());
    Searcher s(&doc, /*chunkSize=*/32);
    int calls = 0;
    auto r = s.findNext("needle", 0, [&](int64_t, int64_t) {
        calls++;
        return false;
    });
    EXPECT_FALSE(r);
    EXPECT_EQ(1, calls);
}

TEST(Searcher, findPrevBasic) {
    string content = "Hello, world! Hello again.";
    ByteDocument doc(content.c_str(), content.size());
    Searcher s(&doc);
    auto r = s.findPrev("Hello", (int64_t) content.size(), nullptr);
    ASSERT_TRUE(r);
    EXPECT_EQ(14, *r);
    // endOffset 14 means match must end at or before 14; first "Hello" ends
    // at 5, so it fits.
    r = s.findPrev("Hello", 14, nullptr);
    ASSERT_TRUE(r);
    EXPECT_EQ(0, *r);
    r = s.findPrev("Hello", 4, nullptr);
    EXPECT_FALSE(r);
}

TEST(Searcher, findPrevMultipleInOneChunk) {
    string content = "xx ab xx ab xx ab xx";
    ByteDocument doc(content.c_str(), content.size());
    Searcher s(&doc);
    auto r = s.findPrev("ab", (int64_t) content.size(), nullptr);
    ASSERT_TRUE(r);
    EXPECT_EQ(15, *r);
    r = s.findPrev("ab", 15, nullptr);
    ASSERT_TRUE(r);
    EXPECT_EQ(9, *r);
    r = s.findPrev("ab", 9, nullptr);
    ASSERT_TRUE(r);
    EXPECT_EQ(3, *r);
    r = s.findPrev("ab", 3, nullptr);
    EXPECT_FALSE(r);
}

TEST(Searcher, findPrevCrossChunkBoundary) {
    string content(20, 'x');
    content[14] = 'n';
    content[15] = 'e';
    content[16] = 'e';
    content[17] = 'd';
    ByteDocument doc(content.c_str(), content.size());
    Searcher s(&doc, /*chunkSize=*/8);
    auto r = s.findPrev("need", (int64_t) content.size(), nullptr);
    ASSERT_TRUE(r);
    EXPECT_EQ(14, *r);
}

TEST(Searcher, findPrevCancel) {
    string content(200, 'a');
    content.insert(0, "needle");
    ByteDocument doc(content.c_str(), content.size());
    Searcher s(&doc, /*chunkSize=*/32);
    int calls = 0;
    auto r = s.findPrev("needle", (int64_t) content.size(), [&](int64_t, int64_t) {
        calls++;
        return false;
    });
    EXPECT_FALSE(r);
    EXPECT_EQ(1, calls);
}

TEST(Searcher, findPrevBom) {
    // UTF-8 BOM at start; firstByte() should be 3, so a match at offset 0
    // (inside the BOM) is unreachable.
    string content = "\xEF\xBB\xBF" "Hello";
    ByteDocument doc(content.c_str(), content.size());
    Searcher s(&doc);
    auto r = s.findNext("Hello", 0, nullptr);
    ASSERT_TRUE(r);
    EXPECT_EQ(3, *r);
}

TEST(Searcher, genomicRepetitive) {
    // ATGC-style short alphabet; naive search degrades, memmem should not.
    string content(10'000, 'A');
    content.replace(9'990, 6, "ATGCAT");
    ByteDocument doc(content.c_str(), content.size());
    Searcher s(&doc);
    auto r = s.findNext("ATGCAT", 0, nullptr);
    ASSERT_TRUE(r);
    EXPECT_EQ(9'990, *r);
}
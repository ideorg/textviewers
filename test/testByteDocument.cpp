//
// Created by andrzej on 8/27/23.
//
#include "gtest/gtest.h"
#include "logic/ByteDocument.h"
#include "misc/util.h"
#include "cpg/utf/UTF.hpp"

using namespace std;

namespace vl {
TEST (ByteDocument, lineIsEmpty) {
    for (int mul = 1; mul <= 3; mul++)
    for (int lineBreaksKind = 0; lineBreaksKind < 3; lineBreaksKind++) {
        int lenBreaks = lineBreaksKind == 2 ? 2 : 1;
        for (int lineCount = 0; lineCount <= 5; lineCount++) {
            vector<int> lineLens(lineCount);
            for (int i = 0; i < lineCount; i++)
                if ((i + lineCount + lineBreaksKind) % 2)
                    lineLens[i] = 10;
                else
                    lineLens[i] = 0;
            lineLens = multiply(lineLens, mul);
            string s = genSampleLineBreaks(lineLens, lineBreaksKind);
            ByteDocument doc(s.c_str(), s.size(), 0);
            int64_t eolExpected = 0;
            int64_t firstLineByte = 0;
            for (int i = 0; i < lineCount; i++) {
                eolExpected = firstLineByte + lineLens[i];
                int64_t next;
                bool addLineBreak;
                if (lineLens.back() == 0)
                    addLineBreak = i < lineLens.size() - 2 || i == lineLens.size() - 1;
                else
                    addLineBreak = i < lineLens.size() - 1;
                if (addLineBreak)
                    next = eolExpected + lenBreaks;
                else
                    next = eolExpected;
                for (int64_t j = firstLineByte; j < eolExpected; j++) {
                    EXPECT_FALSE(doc.lineIsEmpty(j));
                }
                for (int64_t j = eolExpected; j < next; j++) {
                    bool emptyExpected = firstLineByte == eolExpected;
                    bool emptyActual = doc.lineIsEmpty(j);
                    EXPECT_EQ(emptyExpected, emptyActual);
                }
                firstLineByte = next;
            }
        }
    }
}

TEST (ByteDocument, forward) {
    for (int mul = 1; mul <= 3; mul++)
    for (int lineBreaksKind = 0; lineBreaksKind < 3; lineBreaksKind++) {
        int lenBreaks = lineBreaksKind == 2 ? 2 : 1;
        for (int lineCount = 0; lineCount <= 5; lineCount++) {
            vector<int> lineLens(lineCount);
            for (int i = 0; i < lineCount; i++)
                if ((i + lineCount + lineBreaksKind) % 2)
                    lineLens[i] = 10;
                else
                    lineLens[i] = 0;
            lineLens = multiply(lineLens, mul);
            string s = genSampleLineBreaks(lineLens, lineBreaksKind);
            ByteDocument doc(s.c_str(), s.size(), 0);
            int64_t eolExpected = 0;
            int64_t firstLineByte = 0;
            for (int i = 0; i < lineCount; i++) {
                eolExpected = firstLineByte + lineLens[i];
                int64_t next;
                bool addLineBreak;
                if (lineLens.back() == 0)
                    addLineBreak = i < lineLens.size() - 2 || i == lineLens.size() - 1;
                else
                    addLineBreak = i < lineLens.size() - 1;
                if (addLineBreak)
                    next = eolExpected + lenBreaks;
                else
                    next = eolExpected;
                for (int64_t j = firstLineByte; j < eolExpected; j++) {
                    int64_t eolActual = doc.searchEndOfLine(j);
                    EXPECT_EQ(eolExpected, eolActual);
                    EXPECT_THROW(doc.firstOfCRLF(j), std::runtime_error);
                }
                for (int64_t j = eolExpected; j < next; j++) {
                    EXPECT_THROW(doc.searchEndOfLine(j), std::runtime_error);
                    int64_t firstOfCrlf = doc.firstOfCRLF(j);
                    EXPECT_EQ(eolExpected, firstOfCrlf);
                    int64_t actualEnd = doc.skipLineBreak(j);
                    EXPECT_EQ(next, actualEnd);
                }
                firstLineByte = next;
            }
        }
    }
}

TEST (ByteDocument, backward) {
    for (int mul = 1; mul <= 3; mul++)
    for (int lineBreaksKind = 0; lineBreaksKind < 3; lineBreaksKind++) {
        int lenBreaks = lineBreaksKind == 2 ? 2 : 1;
        for (int lineCount = 0; lineCount <= 5; lineCount++) {
            vector<int> lineLens(lineCount);
            for (int i = 0; i < lineCount; i++)
                if ((i + lineCount + lineBreaksKind) % 2)
                    lineLens[i] = 10;
                else
                    lineLens[i] = 0;
            lineLens = multiply(lineLens, mul);
            string s = genSampleLineBreaks(lineLens, lineBreaksKind);
            ByteDocument doc(s.c_str(), s.size(), 0);
            int64_t eolExpected = 0;
            int64_t firstLineByte = 0;
            for (int i = 0; i < lineCount; i++) {
                eolExpected = firstLineByte + lineLens[i];
                int64_t next;
                bool addLineBreak;
                if (lineLens.back() == 0)
                    addLineBreak = i < lineLens.size() - 2 || i == lineLens.size() - 1;
                else
                    addLineBreak = i < lineLens.size() - 1;
                if (addLineBreak)
                    next = eolExpected + lenBreaks;
                else
                    next = eolExpected;
                for (int64_t j = firstLineByte; j < eolExpected; j++) {
                    int64_t startLineActual = doc.gotoBeginLine(j, ByteDocument::elMaybeInside);
                    EXPECT_EQ(firstLineByte, startLineActual);
                    int64_t startLineActual1 = doc.gotoBeginNonEmptyLine(j, ByteDocument::elMaybeInside);
                    EXPECT_EQ(firstLineByte, startLineActual1);
                    EXPECT_THROW(doc.firstOfCRLF(j), std::runtime_error);
                }
                for (int64_t j = eolExpected; j < next; j++) {
                    int64_t eolActual = doc.firstOfCRLF(j);
                    EXPECT_EQ(eolExpected, eolActual);
                    int64_t startLineActual = doc.gotoBeginLine(j, ByteDocument::elTrueEol);
                    EXPECT_EQ(firstLineByte, startLineActual);
                    EXPECT_THROW(doc.gotoBeginNonEmptyLine(j, ByteDocument::elMaybeInside), std::runtime_error);
                }
                firstLineByte = next;
            }
        }
    }
}

const int MAXLEN = 20;

/* test MaxLineLen, for MaxLineLen=40 line breaks will be in multiples of 40 */
TEST (ByteDocumentML, lineIsEmpty) {
    for (int firstLen = 0; firstLen < 2 * MAXLEN + 2; firstLen++) {
        vector<int> lineLens = {firstLen, 99, 0, 15, MAXLEN - 1, 2 * MAXLEN, MAXLEN, 2 * MAXLEN - 1, MAXLEN + 1,
                MAXLEN / 2};
        auto extLineLens = getLineBreaks(lineLens, 0);
        auto maxLineLens = getMaxLineBreaks(extLineLens, MAXLEN);
        string s = genSampleLineBreaks(lineLens, 0);
        ByteDocument doc(s.c_str(), s.size(), MAXLEN);
        int64_t offset = doc.firstByte();
        for (auto p: maxLineLens) {
            int64_t eolExpected = offset + p.len;
            for (int64_t j = offset; j < eolExpected; j++)
                EXPECT_FALSE(doc.lineIsEmpty(j));
            int64_t next = eolExpected + p.breaks;
            for (int64_t j = eolExpected; j < next; j++) {
                bool emptyExpected = offset == eolExpected;
                bool emptyActual = doc.lineIsEmpty(j);
                EXPECT_EQ(emptyExpected, emptyActual);
            }
            offset = next;
        }
    }
}

/*
searchEndOfLine
searchEndOfLineFromStart
skipLineBreak */
TEST (ByteDocumentML, forward) {
    for (int firstLen = 0; firstLen < 2 * MAXLEN + 2; firstLen++) {
        vector<int> lineLens = {firstLen, 99, 0, 15, MAXLEN - 1, 2 * MAXLEN, MAXLEN, 2 * MAXLEN - 1, MAXLEN + 1,
                MAXLEN / 2};
        auto extLineLens = getLineBreaks(lineLens, 0);
        auto maxLineLens = getMaxLineBreaks(extLineLens, MAXLEN);
        string s = genSampleLineBreaks(lineLens, 0);
        ByteDocument doc(s.c_str(), s.size(), MAXLEN);
        int64_t offset = doc.firstByte();
        for (int i=0; i<maxLineLens.size(); i++) {
            auto p = maxLineLens[i];
            int64_t eolExpected = offset + p.len;
            for (int64_t j = offset; j < eolExpected; j++) {
                int64_t eolActual = doc.searchEndOfLine(j);
                ASSERT_EQ(eolExpected, eolActual);
                EXPECT_THROW(doc.firstOfCRLF(j), std::runtime_error);
            }
            int64_t next = eolExpected + p.breaks;
            for (int64_t j = eolExpected; j < next; j++) {
                EXPECT_THROW(doc.searchEndOfLine(j), std::runtime_error);
                int64_t firstOfCrlf = doc.firstOfCRLF(j);
                EXPECT_EQ(eolExpected, firstOfCrlf);
                int64_t actualEnd = doc.skipLineBreak(j);
                EXPECT_EQ(next, actualEnd);
            }
            offset = next;
        }
    }
}

/*
firstOfCRLF
gotoBeginLine
gotoBeginNonEmptyLine */
TEST (ByteDocumentML, backward) {
    for (int firstLen = 0; firstLen < 2 * MAXLEN + 2; firstLen++) {
        vector<int> lineLens = {firstLen, 99, 0, 15, MAXLEN - 1, 2 * MAXLEN, MAXLEN, 2 * MAXLEN - 1, MAXLEN + 1,
                MAXLEN / 2};
        auto extLineLens = getLineBreaks(lineLens, 0);
        auto maxLineLens = getMaxLineBreaks(extLineLens, MAXLEN);
        string s = genSampleLineBreaks(lineLens, 0);
        ByteDocument doc(s.c_str(), s.size(), MAXLEN);
        int64_t offset = doc.firstByte();
        for (auto p: maxLineLens) {
            int64_t eolExpected = offset + p.len;
            for (int64_t j = offset; j < eolExpected; j++) {
                int64_t offsetActual1 = doc.gotoBeginLine(j, ByteDocument::elMaybeInside);
                int64_t offsetActual2 = doc.gotoBeginNonEmptyLine(j, ByteDocument::elMaybeInside);
                ASSERT_EQ(offset, offsetActual1);
                ASSERT_EQ(offset, offsetActual2);
                EXPECT_THROW(doc.firstOfCRLF(j), std::runtime_error);
            }
            int64_t next = eolExpected + p.breaks;
            for (int64_t j = eolExpected; j < next; j++) {
                int64_t offsetActual1 = doc.gotoBeginLine(j, ByteDocument::elTrueEol);
                ASSERT_EQ(offset, offsetActual1);
                EXPECT_THROW(doc.gotoBeginNonEmptyLine(j, ByteDocument::elTrueEol), std::runtime_error);
                int64_t firstOfCrlf = doc.firstOfCRLF(j);
                EXPECT_EQ(eolExpected, firstOfCrlf);
            }
            offset = next;
        }
    }
}

/*
searchEndOfLine
searchEndOfLineFromStart
skipLineBreak */
TEST (ByteDocumentMLunicode, forward) {
    for (int utf8len = 1; utf8len <= 4; utf8len++)
        for (int firstLen = 0; firstLen < 2 * MAXLEN + 2; firstLen++) {
            vector<int> lineLens = {firstLen, 99, 0, 15, MAXLEN - 1, 2 * MAXLEN, MAXLEN, 2 * MAXLEN - 1, MAXLEN + 1,
                    MAXLEN / 2};
            auto extLineLens = getLineBreaks(lineLens, 2);
            auto maxLineLens = getMaxLineBreaks(extLineLens, MAXLEN, utf8len);
            string s = genSampleUnicode(lineLens, utf8len);
            ByteDocument doc(s.c_str(), s.size(), MAXLEN);
            int64_t offset = doc.firstByte();
            for (int i = 0; i < maxLineLens.size(); i++) {
                auto p = maxLineLens[i];
                int64_t eolExpected = offset + p.len;
                for (int64_t j = offset; j < eolExpected;) {
                    int64_t eolActual = doc.searchEndOfLine(j);
                    ASSERT_EQ(eolExpected, eolActual);
                    EXPECT_THROW(doc.firstOfCRLF(j), std::runtime_error);
                    UTF utf;
                    j += utf.one8len(s[j]);
                }
                int64_t next = eolExpected + p.breaks;
                for (int64_t j = eolExpected; j < next; j++) {
                    EXPECT_THROW(doc.searchEndOfLine(j), std::runtime_error);
                    int64_t firstOfCrlf = doc.firstOfCRLF(j);
                    EXPECT_EQ(eolExpected, firstOfCrlf);
                    int64_t actualEnd = doc.skipLineBreak(j);
                    EXPECT_EQ(next, actualEnd);
                }
                offset = next;
            }
        }
}

/*
firstOfCRLF
gotoBeginLine
gotoBeginNonEmptyLine */
TEST (ByteDocumentMLunicode, backward) {
    for (int utf8len = 1; utf8len <= 4; utf8len++)
        for (int firstLen = 0; firstLen < 2 * MAXLEN + 2; firstLen++) {
            vector<int> lineLens = {firstLen, 99, 0, 15, MAXLEN - 1, 2 * MAXLEN, MAXLEN, 2 * MAXLEN - 1, MAXLEN + 1,
                    MAXLEN / 2};
            auto extLineLens = getLineBreaks(lineLens, 2);
            auto maxLineLens = getMaxLineBreaks(extLineLens, MAXLEN, utf8len);
            string s = genSampleUnicode(lineLens, utf8len);
            ByteDocument doc(s.c_str(), s.size(), MAXLEN);
            int64_t offset = doc.firstByte();
            for (int i = 0; i < maxLineLens.size(); i++) {
                auto p = maxLineLens[i];
                assert(offset == p.offset);
                int64_t eolExpected = offset + p.len;
                for (int64_t j = offset; j < eolExpected; j++) {
                    int64_t offsetActual1 = doc.gotoBeginLine(j, ByteDocument::elMaybeInside);
                    int64_t offsetActual2 = doc.gotoBeginNonEmptyLine(j, ByteDocument::elMaybeInside);
                    ASSERT_EQ(offset, offsetActual1);
                    ASSERT_EQ(offset, offsetActual2);
                    EXPECT_THROW(doc.firstOfCRLF(j), std::runtime_error);
                }
                int64_t next = eolExpected + p.breaks;
            for (int64_t j = eolExpected; j < next; j++) {
                int64_t offsetActual1 = doc.gotoBeginLine(j,ByteDocument::elTrueEol);
                ASSERT_EQ(offset, offsetActual1);
                EXPECT_THROW(doc.gotoBeginNonEmptyLine(j,ByteDocument::elTrueEol), std::runtime_error);
                int64_t firstOfCrlf = doc.firstOfCRLF(j);
                EXPECT_EQ(eolExpected, firstOfCrlf);
            }
            offset = next;
        }
    }
}


}
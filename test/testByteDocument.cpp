//
// Created by andrzej on 8/27/23.
//
#include "gtest/gtest.h"
#include "logic/ByteDocument.h"
#include "misc/util.h"

using namespace std;

namespace vl {
TEST (ByteDocument, lineIsEmpty) {
    for (int lineBreakAtEnd = 0; lineBreakAtEnd < 2; lineBreakAtEnd++)
    for (int lineBreaksKind = 0; lineBreaksKind < 3; lineBreaksKind++) {
        int lenBreaks = lineBreaksKind == 2 ? 2 : 1;
        for (int lineCount = 0; lineCount <= 5; lineCount++) {
            vector<int> lineLens(lineCount);
            for (int i = 0; i < lineCount; i++)
                if ((i + lineCount + lineBreaksKind) % 2)
                    lineLens[i] = 10;
                else
                    lineLens[i] = 0;
            string s = genSampleLineBreaks(lineLens, lineBreaksKind, lineBreakAtEnd);
            ByteDocument doc(s.c_str(), s.size(), 0);
            int64_t eolExpected = 0;
            int64_t firstLineByte = 0;
            for (int i = 0; i < lineCount; i++) {
                eolExpected = firstLineByte + lineLens[i];
                int64_t next;
                if (lineBreakAtEnd || i < lineCount - 1)
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
    for (int lineBreakAtEnd = 0; lineBreakAtEnd < 2; lineBreakAtEnd++)
    for (int lineBreaksKind = 0; lineBreaksKind < 3; lineBreaksKind++) {
        int lenBreaks = lineBreaksKind == 2 ? 2 : 1;
        for (int lineCount = 0; lineCount <= 5; lineCount++) {
            vector<int> lineLens(lineCount);
            for (int i = 0; i < lineCount; i++)
                if ((i + lineCount + lineBreaksKind) % 2)
                    lineLens[i] = 10;
                else
                    lineLens[i] = 0;
            string s = genSampleLineBreaks(lineLens, lineBreaksKind, lineBreakAtEnd);
            ByteDocument doc(s.c_str(), s.size(), 0);
            int64_t eolExpected = 0;
            int64_t firstLineByte = 0;
            for (int i = 0; i < lineCount; i++) {
                eolExpected = firstLineByte + lineLens[i];
                int64_t next;
                if (lineBreakAtEnd || i < lineCount - 1)
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
    for (int lineBreakAtEnd = 0; lineBreakAtEnd < 2; lineBreakAtEnd++)
    for (int lineBreaksKind = 0; lineBreaksKind < 3; lineBreaksKind++) {
        int lenBreaks = lineBreaksKind == 2 ? 2 : 1;
        for (int lineCount = 0; lineCount <= 5; lineCount++) {
            vector<int> lineLens(lineCount);
            for (int i = 0; i < lineCount; i++)
                if ((i + lineCount + lineBreaksKind) % 2)
                    lineLens[i] = 10;
                else
                    lineLens[i] = 0;
            string s = genSampleLineBreaks(lineLens, lineBreaksKind, lineBreakAtEnd);
            ByteDocument doc(s.c_str(), s.size(), 0);
            int64_t eolExpected = 0;
            int64_t firstLineByte = 0;
            for (int i = 0; i < lineCount; i++) {
                eolExpected = firstLineByte + lineLens[i];
                int64_t next;
                if (lineBreakAtEnd || i < lineCount - 1)
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
}
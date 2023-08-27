//
// Created by andrzej on 8/27/23.
//
#include "gtest/gtest.h"
#include "logic/ByteDocument.h"
#include "misc/util.h"

using namespace std;
using namespace vl;

TEST (IByteAccess, lineAfter) {
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
            IByteAccess *idoc = &doc;
            auto optLine = idoc->firstLine();
            EXPECT_EQ(lineCount > 0, optLine.has_value());
            int counter = 0;
            int prevOffset = idoc->firstByte();
            while (optLine) {
                auto lpLine = optLine.value();
                EXPECT_EQ(prevOffset, lpLine.offset);
                EXPECT_EQ(lineLens[counter], lpLine.len);
                int currentBreaks = lineBreakAtEnd || counter < lineCount - 1 || lineLens[counter] == 0
                                    ? lenBreaks : 0;
                EXPECT_EQ(lineLens[counter] + currentBreaks, lpLine.fullLen);
                counter++;
                prevOffset += lpLine.fullLen;
                optLine = idoc->lineAfter(lpLine);
            }
        }
    }
}

TEST (IByteAccess, lineBefore) {
    for (int lineBreakAtEnd = 0; lineBreakAtEnd < 2; lineBreakAtEnd++)
        for (int lineBreaksKind = 1/**todo**/; lineBreaksKind < 3; lineBreaksKind++) {
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
            IByteAccess *idoc = &doc;
            auto optLine = idoc->lastLine();
            EXPECT_EQ(optLine.has_value(), lineCount > 0);
            int counter = lineCount-1;
            int nextOffset = idoc->byteCount();
            while (optLine) {
                auto lpLine = optLine.value();
                EXPECT_EQ(nextOffset, lpLine.offset + lpLine.fullLen);
                EXPECT_EQ(lineLens[counter], lpLine.len);
                int currentBreaks = lineBreakAtEnd || counter < lineCount - 1 || lineLens[counter] == 0
                                    ? lenBreaks : 0;
                EXPECT_EQ(lineLens[counter] + currentBreaks, lpLine.fullLen);
                counter--;
                nextOffset = lpLine.offset;
                optLine = idoc->lineBefore(lpLine);
            }
        }
    }
}

TEST (IByteAccess, lineEnclosing) {
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
                        auto lp = doc.lineEnclosing(j);
                        EXPECT_EQ(firstLineByte, lp.offset);
                    }
                    for (int64_t j = eolExpected; j < next; j++) {
                        auto lp = doc.lineEnclosing(j);
                        EXPECT_EQ(firstLineByte, lp.offset);
                    }
                    firstLineByte = next;
                }
            }
        }
}
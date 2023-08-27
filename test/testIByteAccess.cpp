//
// Created by andrzej on 8/27/23.
//
#include "gtest/gtest.h"
#include "logic/ByteDocument.h"
#include "misc/util.h"

using namespace std;
using namespace vl;

TEST (IByteAccess, lineAfter) {
    for (int lineBreakAtEnd = 0; lineBreakAtEnd < 3; lineBreakAtEnd++)
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
            ByteDocument doc(s.c_str(), s.size(), lineBreakAtEnd == 2, 0);
            IByteAccess *idoc = &doc;
            auto optLine = idoc->firstLine();
            EXPECT_EQ(lineCount > 0, optLine.has_value());
            int counter = 0;
            int prevOffset = idoc->firstByte();
            while (optLine) {
                auto lpLine = optLine.value();
                EXPECT_EQ(prevOffset, lpLine.offset);
                EXPECT_EQ(lineLens[counter], lpLine.len);
                int currentBreaks;
                switch (lineBreakAtEnd) {
                    case 0:
                        currentBreaks = counter < lineCount - 1 || lineLens[counter] == 0 ? lenBreaks : 0;
                        break;
                    case 1:
                        currentBreaks = lenBreaks;
                        break;
                    default:
                        if (lineLens.back() == 0)
                            currentBreaks = counter < lineLens.size() - 2 || counter == lineLens.size() - 1 ? lenBreaks : 0;
                        else
                            currentBreaks = counter < lineLens.size() - 1 ? lenBreaks : 0;
                        break;
                }
                EXPECT_EQ(lineLens[counter] + currentBreaks, lpLine.fullLen);
                counter++;
                prevOffset += lpLine.fullLen;
                optLine = idoc->lineAfter(lpLine);
            }
        }
    }
}

TEST (IByteAccess, lineBefore) {
    for (int lineBreakAtEnd = 0; lineBreakAtEnd < 3; lineBreakAtEnd++)
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
            ByteDocument doc(s.c_str(), s.size(), lineBreakAtEnd == 2, 0);
            IByteAccess *idoc = &doc;
            auto optLine = idoc->lastLine();
            EXPECT_EQ(optLine.has_value(), lineCount > 0);
            int counter = lineCount-1;
            int nextOffset = idoc->byteCount();
            while (optLine) {
                auto lpLine = optLine.value();
                EXPECT_EQ(nextOffset, lpLine.offset + lpLine.fullLen);
                if (lineLens[counter] != lpLine.len)
                EXPECT_EQ(lineLens[counter], lpLine.len);
                int currentBreaks;
                switch (lineBreakAtEnd) {
                    case 0:
                        currentBreaks = counter < lineCount - 1 || lineLens[counter] == 0 ? lenBreaks : 0;
                        break;
                    case 1:
                        currentBreaks = lenBreaks;
                        break;
                    default:
                        if (lineLens.back() == 0)
                            currentBreaks = counter < lineLens.size() - 2 || counter == lineLens.size() - 1 ? lenBreaks : 0;
                        else
                            currentBreaks = counter < lineLens.size() - 1 ? lenBreaks : 0;
                        break;
                }
                EXPECT_EQ(lineLens[counter] + currentBreaks, lpLine.fullLen);
                counter--;
                nextOffset = lpLine.offset;
                optLine = idoc->lineBefore(lpLine);
            }
        }
    }
}

TEST (IByteAccess, lineEnclosing) {
    for (int lineBreakAtEnd = 0; lineBreakAtEnd < 3; lineBreakAtEnd++)
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
                ByteDocument doc(s.c_str(), s.size(), lineBreakAtEnd == 2, 0);
                int64_t eolExpected = 0;
                int64_t firstLineByte = 0;
                for (int i = 0; i < lineCount; i++) {
                    eolExpected = firstLineByte + lineLens[i];
                    int64_t next;
                    int currentBreaks;
                    switch (lineBreakAtEnd) {
                        case 0:
                            currentBreaks = i < lineCount - 1 || lineLens[i] == 0 ? lenBreaks : 0;
                            break;
                        case 1:
                            currentBreaks = lenBreaks;
                            break;
                        default:
                            if (lineLens.back() == 0)
                                currentBreaks = i < lineLens.size() - 2 || i == lineLens.size() - 1 ? lenBreaks : 0;
                            else
                                currentBreaks = i < lineLens.size() - 1 ? lenBreaks : 0;
                            break;
                    }
                    if (lineBreakAtEnd || i < lineCount - 1)
                        next = eolExpected + currentBreaks;
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
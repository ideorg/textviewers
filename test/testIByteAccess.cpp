//
// Created by andrzej on 8/27/23.
//
#include "gtest/gtest.h"
#include "logic/ByteDocument.h"
#include "misc/util.h"

using namespace std;
using namespace vl;

TEST (IByteAccess, lineAfter) {
    for (int lineBreaksKind = 0; lineBreaksKind < 3; lineBreaksKind++) {
        int lenBreaks = lineBreaksKind == 2 ? 2 : 1;
        for (int lineCount = 0; lineCount <= 5; lineCount++) {
            vector<int> lineLens(lineCount);
            for (int i = 0; i < lineCount; i++)
                if ((i + lineCount + lineBreaksKind) % 2)
                    lineLens[i] = 10;
                else
                    lineLens[i] = 0;
            string s = genSample(lineLens, lineBreaksKind);
            ByteDocument doc(s.c_str(), s.size(), 0);
            IByteAccess *idoc = &doc;
            auto optFirstLine = idoc->firstLine();
            EXPECT_EQ(optFirstLine.has_value(), lineCount>0);
            if (!optFirstLine)
                continue;
            auto lpFirstLine = optFirstLine.value();
            EXPECT_EQ(lpFirstLine.offset,idoc->firstByte());
            EXPECT_EQ(lpFirstLine.len, lineLens[0]);
            EXPECT_EQ(lpFirstLine.fullLen, lineLens[0] + lenBreaks);
            auto optLineAfter = idoc->lineAfter(lpFirstLine);
        }
    }
}

TEST (IByteAccess, lineBefore) {
    for (int lineBreaksKind = 0; lineBreaksKind < 3; lineBreaksKind++) {
        int lenBreaks = lineBreaksKind == 2 ? 2 : 1;
        for (int lineCount = 0; lineCount <= 5; lineCount++) {
            vector<int> lineLens(lineCount);
            for (int i = 0; i < lineCount; i++)
                if ((i + lineCount + lineBreaksKind) % 2)
                    lineLens[i] = 10;
                else
                    lineLens[i] = 0;
            string s = genSample(lineLens, lineBreaksKind);
            ByteDocument doc(s.c_str(), s.size(), 0);
            IByteAccess *idoc = &doc;
            auto optFirstLine = idoc->lastLine();
            EXPECT_EQ(optFirstLine.has_value(), lineCount>0);
            if (!optFirstLine)
                continue;
            auto lpLastLine = optFirstLine.value();
            EXPECT_EQ(lpLastLine.offset + lpLastLine.fullLen, idoc->byteCount());
            EXPECT_EQ(lpLastLine.len, lineLens.back());
            EXPECT_EQ(lpLastLine.fullLen, lpLastLine.len + lenBreaks);
            auto optLineAfter = idoc->lineAfter(lpLastLine);
        }
    }
}
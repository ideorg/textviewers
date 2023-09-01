//
// Created by andrzej on 9/1/23.
//
#include "gtest/gtest.h"
#include "logic/ByteDocument.h"
#include "misc/util.h"

using namespace std;
using namespace vl;

const int MAXLEN = 20;

TEST (IByteAccessML, lineAfter) {
    for (int utf8len = 1; utf8len <= 4; utf8len++)
        for (int firstLen = 0; firstLen < 2 * MAXLEN + 2; firstLen++) {
            vector<int> lineLens = {firstLen, 99, 0, 15, MAXLEN - 1, 2 * MAXLEN, MAXLEN, 2 * MAXLEN - 1, MAXLEN + 1,
                    MAXLEN / 2};
            auto extLineLens = getLineBreaks(lineLens, 2);
            auto maxLineLens = getMaxLineBreaks(extLineLens, MAXLEN, utf8len);
            string s = genSampleUnicode(lineLens, utf8len);
            ByteDocument doc(s.c_str(), s.size(), MAXLEN);
            IByteAccess *idoc = &doc;
            auto optLine = idoc->firstLine();
            int cnt = 0;
            while (optLine) {
                auto lpLine = optLine.value();
                ASSERT_EQ(maxLineLens[cnt].offset, lpLine.offset);
                ASSERT_EQ(maxLineLens[cnt].len, lpLine.len);
                ASSERT_EQ(maxLineLens[cnt].breaks, lpLine.fullLen - lpLine.len);
                optLine = idoc->lineAfter(lpLine);
                cnt++;
            }
        }
}

TEST (IByteAccessML, lineBefore) {
    for (int utf8len = 1; utf8len <= 4; utf8len++)
        for (int firstLen = 0; firstLen < 2 * MAXLEN + 2; firstLen++) {
            vector<int> lineLens = {firstLen, 99, 0, 15, MAXLEN - 1, 2 * MAXLEN, MAXLEN, 2 * MAXLEN - 1, MAXLEN + 1,
                    MAXLEN / 2};
            auto extLineLens = getLineBreaks(lineLens, 2);
            auto maxLineLens = getMaxLineBreaks(extLineLens, MAXLEN, utf8len);
            string s = genSampleUnicode(lineLens, utf8len);
            ByteDocument doc(s.c_str(), s.size(), MAXLEN);
            IByteAccess *idoc = &doc;
            auto optLine = idoc->lastLine();
            int cnt = maxLineLens.size() - 1;
            while (optLine) {
                auto lpLine = optLine.value();
                ASSERT_EQ(maxLineLens[cnt].offset, lpLine.offset);
                EXPECT_EQ(maxLineLens[cnt].len, lpLine.len);
                ASSERT_EQ(maxLineLens[cnt].breaks, lpLine.fullLen - lpLine.len);
                optLine = idoc->lineBefore(lpLine);
                cnt--;
            }
        }
}
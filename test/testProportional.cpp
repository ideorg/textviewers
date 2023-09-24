//
// Created by andrzej on 8/13/23.
//
#include "misc/util.h"
#include "gtest/gtest.h"
#include "logic/ByteView.h"
#include "logic/ByteDocument.h"

using namespace std;
using namespace vl;

TEST (Proportional, portion) {
    string fname = "../test/data/proportional.txt";
    string content = makeContent(fname);
    auto sizes = getSizes(fname);
    ByteDocument doc(content.c_str(), content.length(), 0);
    ByteView vtest(&doc);
    vtest.setWrapMode(0);
    vtest.setScreenLineLen(8);
    for (int j = 0; j <= doc.byteCount(); j += 20)
        for (int i = 20; i < 40; i += 5) {
            vtest.setScreenLineCount(i);
            vtest.gotoProportional((double)j/doc.byteCount());
            vtest.fillDeque();
            vtest.recalcLines();
            ASSERT_EQ(min((size_t)vtest.screenLineCount(), sizes.size()), vtest.size());
        }
}

TEST (Proportional, reversing) {
    string fname = "../test/data/textviewer.h0";
    string content = makeString(fname);
    auto sizes = getSizesStr(fname);
    ByteDocument doc(content.c_str(), content.length(), 0);
    ByteView vtest(&doc);
    vtest.setWrapMode(0);
    vtest.setScreenLineLen(41);
    vtest.setScreenLineCount(19);
    int64_t previousStartY = 0;
    for (int i = 0; i < doc.byteCount(); i += 10) {
        vtest.gotoProportional((double)i/doc.byteCount());
        vtest.fillDeque();
        vtest.recalcLines();
        int64_t startY = vtest.startY();
        EXPECT_GE(startY, previousStartY);
        previousStartY = startY;
    }
}

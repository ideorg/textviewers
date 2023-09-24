//
// Created by Andrzej Borucki on 2022-10-02
//
#include "misc/util.h"
#include "gtest/gtest.h"
#include "logic/ByteDocument.h"
#include "logic/ByteView.h"

using namespace std;
using namespace vl;

TEST(beginX, UnicodeX10) {
    string content = makeContent("../test/data/utf8.txt");
    ByteDocument doc(content.c_str(), content.length(), 0);
    ByteView vtest(&doc);
    vtest.setScreenLineLen(10);
    vtest.setScreenLineCount(10);
    vtest.setWrapMode(0);
    auto vexpect = makeExpect("../test/expect/utf8_first10w10X10.txt");
    vtest.setStartX(10);
    EXPECT_EQ(vexpect.size(), vtest.size());
    for (int i = 0; i < min(vtest.size(), vexpect.size()); i++)
        EXPECT_EQ(vtest[i], vexpect[i]);
}

TEST(beginX, UnicodeX12) {
    string content = makeContent("../test/data/utf8.txt");
    ByteDocument doc(content.c_str(), content.length(), 0);
    ByteView vtest(&doc);
    vtest.setScreenLineLen(10);
    vtest.setScreenLineCount(10);
    vtest.setWrapMode(0);
    auto vexpect = makeExpect("../test/expect/utf8_first10w10X12.txt");
    vtest.setStartX(12);
    EXPECT_EQ(vexpect.size(), vtest.size());
    for (int i = 0; i < min(vtest.size(), vexpect.size()); i++)
        EXPECT_EQ(vtest[i], vexpect[i]);
}

TEST(beginX, UnicodeX40) {
    string content = makeContent("../test/data/utf8.txt");
    ByteDocument doc(content.c_str(), content.length(), 0);
    ByteView vtest(&doc);
    vtest.setScreenLineLen(10);
    vtest.setScreenLineCount(10);
    vtest.setWrapMode(0);
    auto vexpect = makeExpect("../test/expect/utf8_first10w10X40.txt");
    vtest.setStartX(40);
    EXPECT_EQ(vexpect.size(), vtest.size());
    for (int i = 0; i < min(vtest.size(), vexpect.size()); i++)
        EXPECT_EQ(vtest[i], vexpect[i]);
}

TEST(beginX, cache) {
    string content = makeContent("../test/data/utf8.txt");
    ByteDocument doc(content.c_str(), content.length(), 0);
    ByteView vtest(&doc);
    vtest.setScreenLineLen(10);
    vtest.setScreenLineCount(10);
    vtest.setWrapMode(0);

    //vtest.minLineToCacheX = 0;
    auto vexpect0 = makeExpect("../test/expect/utf8_first10w10.txt");
    auto vexpect10 = makeExpect("../test/expect/utf8_first10w10X10.txt");
    auto vexpect12 = makeExpect("../test/expect/utf8_first10w10X12.txt");
    auto vexpect40 = makeExpect("../test/expect/utf8_first10w10X40.txt");

    vtest.fillDeque();
    vtest.recalcLines();
    EXPECT_EQ(vexpect0.size(), vtest.size());
    for (int i = 0; i < min(vtest.size(), vexpect0.size()); i++)
        EXPECT_EQ(vtest[i], vexpect0[i]);

    vtest.setStartX(10);
    EXPECT_EQ(vexpect10.size(), vtest.size());
    for (int i = 0; i < min(vtest.size(), vexpect10.size()); i++)
        EXPECT_EQ(vtest[i], vexpect10[i]);

    vtest.setStartX(12);
    EXPECT_EQ(vexpect12.size(), vtest.size());
    for (int i = 0; i < min(vtest.size(), vexpect12.size()); i++)
        EXPECT_EQ(vtest[i], vexpect12[i]);
    //only from cache
    vtest.setStartX(12);
    EXPECT_EQ(vexpect12.size(), vtest.size());
    for (int i = 0; i < min(vtest.size(), vexpect12.size()); i++)
        EXPECT_EQ(vtest[i], vexpect12[i]);

    vtest.setStartX(40);
    EXPECT_EQ(vexpect40.size(), vtest.size());
    for (int i = 0; i < min(vtest.size(), vexpect40.size()); i++)
        EXPECT_EQ(vtest[i], vexpect40[i]);

    vtest.setStartX(12);
    EXPECT_EQ(vexpect12.size(), vtest.size());
    for (int i = 0; i < min(vtest.size(), vexpect12.size()); i++)
        EXPECT_EQ(vtest[i], vexpect12[i]);

    vtest.setStartX(10);
    EXPECT_EQ(vexpect10.size(), vtest.size());
    for (int i = 0; i < min(vtest.size(), vexpect10.size()); i++)
        EXPECT_EQ(vtest[i], vexpect10[i]);

    vtest.setStartX(0);
    EXPECT_EQ(vexpect0.size(), vtest.size());
    for (int i = 0; i < min(vtest.size(), vexpect0.size()); i++)
        EXPECT_EQ(vtest[i], vexpect0[i]);
}

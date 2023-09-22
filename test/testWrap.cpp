//
// Created by Andrzej Borucki on 2022-09-11
//
#include "gtest/gtest.h"
#include "logic/ByteDocument.h"
#include "logic/ByteView.h"
#include "misc/util.h"

using namespace std;
using namespace vl;

TEST (Wrap, forward) {
    string content = makeContent("../test/data/midlines.txt");
    ByteDocument doc(content.c_str(), content.length(), 0);
    ByteView vtest(&doc);
    vtest.setScreenLineLen(10);
    vtest.setScreenLineCount(20);
    vtest.setWrapMode(1);
    auto vexpect = makeExpect("../test/expect/midlines_first20wrap10.txt");
    vtest.fillDeque();
    vtest.recalcLines();
    EXPECT_GT(vexpect.size(), 0);
    EXPECT_EQ(vexpect.size(), vtest.size());
    for (int i = 0; i < min(vtest.size(), vexpect.size()); i++)
        EXPECT_EQ(vexpect[i], vtest[i]);
}

TEST(Wrap, backward_short) {
    string content = makeContent("../test/data/shortwrap.txt");
    ByteDocument doc(content.c_str(), content.length(), 0);
    ByteView vexpect(&doc);
    vexpect.setScreenLineLen(7);
    vexpect.setScreenLineCount(20);
    vexpect.setWrapMode(1);
    vexpect.fillDeque();
    vexpect.recalcLines();
    ByteView vtest(&doc);
    vtest.setScreenLineLen(7);
    vtest.setScreenLineCount(vexpect.size() - 1);
    vtest.setWrapMode(1);
    vtest.gotoProportional(1);
    vtest.fillDeque();
    vtest.recalcLines();
    EXPECT_EQ(vexpect.size() - 1, vtest.size());
    for (int i = 0; i < min(vtest.size(), vexpect.size() - 1); i++)
        EXPECT_EQ(vexpect[i + 1], vtest[i]);
}

TEST(Wrap, backward_short_empt) {
    string content = makeContent("../test/data/emshortwrap.txt");
    ByteDocument doc(content.c_str(), content.length(), 0);
    ByteView vexpect(&doc);
    vexpect.setWrapMode(1);
    vexpect.setScreenLineLen(7);
    vexpect.setScreenLineCount(20);
    vexpect.fillDeque();
    vexpect.recalcLines();
    ByteView vtest(&doc);
    vtest.setWrapMode(1);
    vtest.setScreenLineLen(7);
    vtest.setScreenLineCount(vexpect.size() - 1);
    vtest.gotoProportional(1);
    vtest.fillDeque();
    vtest.recalcLines();
    EXPECT_EQ(vexpect.size() - 1, vtest.size());
    for (int i = 0; i < min(vtest.size(), vexpect.size() - 1); i++)
        EXPECT_EQ(vexpect[i + 1], vtest[i]);
}

TEST(Wrap, backward_mid) {
    string content = makeContent("../test/data/midlines.txt");
    ByteDocument doc(content.c_str(), content.length(), 0);
    ByteView vexpect(&doc);
    vexpect.setScreenLineLen(100);
    vexpect.setScreenLineCount(20);
    vexpect.setWrapMode(1);
    vexpect.fillDeque();
    vexpect.recalcLines();
    ByteView vtest(&doc);
    vtest.setScreenLineLen(100);
    vtest.setScreenLineCount(vexpect.size() - 1);
    vtest.gotoProportional(1);
    vtest.fillDeque();
    vtest.recalcLines();
    EXPECT_EQ(vexpect.size() - 1, vtest.size());
    for (int i = 0; i < min(vtest.size(), vexpect.size() - 1); i++)
        EXPECT_EQ(vexpect[i + 1], vtest[i]);
}

TEST(Wrap, backward_short_any) {
    string content = makeContent("../test/data/shortwrap.txt");
    ByteDocument doc(content.c_str(), content.length(), 0);
    ByteView vexpect(&doc);
    vexpect.setScreenLineLen(7);
    vexpect.setScreenLineCount(20);
    vexpect.setWrapMode(1);
    vexpect.fillDeque();
    vexpect.recalcLines();
    ByteView vtest(&doc);
    vtest.setWrapMode(1);
    vtest.setScreenLineLen(7);
    vtest.setScreenLineCount(vexpect.size() - 2);
    vtest.gotoProportional(28.0/(content.length()-1));
    vtest.fillDeque();
    vtest.recalcLines();
    EXPECT_EQ(vexpect.size() - 2, vtest.size());
    for (int i = 0; i < min(vtest.size(), vexpect.size() - 1); i++)
        EXPECT_EQ(vexpect[i + 1], vtest[i]);
}

TEST(Wordwrap, words) {
    string content = makeContent("../test/data/wordwrap.txt");
    ByteDocument doc(content.c_str(), content.length(), 0);
    ByteView vtest(&doc);
    vtest.setScreenLineLen(10);
    vtest.setScreenLineCount(20);
    vtest.setWrapMode(2);
    auto vexpect = makeExpect("../test/expect/wordwrap_first11ww10.txt");
    vtest.fillDeque();
    vtest.recalcLines();
    EXPECT_EQ(vexpect.size(), vtest.size());
    for (int i = 0; i < min(vtest.size(), vexpect.size()); i++)
        EXPECT_EQ(vexpect[i], vtest[i]);
}

TEST (Wrap, scroll) {
    string content = makeString("../test/data/textviewer.h0");
    ByteDocument doc(content.c_str(), content.length(), 0);
    ByteView vtest(&doc);
    vtest.setWrapMode(1);
    vtest.setScreenLineLen(41);
    vtest.setScreenLineCount(19);
    vtest.gotoProportional(0);
    vtest.fillDeque();
    vtest.recalcLines();

    vtest.scrollDown();
    auto down1 = vtest.clone();
    vtest.scrollDown();
    vtest.scrollUp();
    for (int i = 0; i < min(vtest.size(), down1->size()); i++)
        EXPECT_EQ((*down1)[i], vtest[i]);
    vtest.scrollUp();
    for (int i = 1; i < 5; i++) {
        vtest.scrollNDown(i);
        vtest.scrollNUp(i);
    }
}

TEST (WrapMode, change) {
    string fname = "../test/data/textviewer.h0";
    string content = makeString(fname);
    auto sizes = getSizesStr(fname);
    ByteDocument doc(content.c_str(), content.length(), 0);
    ByteView vtest(&doc);
    vtest.setWrapMode(0);
    vtest.setScreenLineLen(41);
    vtest.setScreenLineCount(19);
    vtest.gotoProportional(0);
    vtest.fillDeque();
    vtest.recalcLines();
    vtest.scrollDown();
    auto firstRow0 = vtest.at(0);
    vtest.setWrapMode(1);
    auto firstRow1 = vtest.at(0);
    EXPECT_EQ(firstRow0, firstRow1);
}
//
// Created by andrzej on 8/27/23.
//
#include "gtest/gtest.h"
#include "logic/ByteDocument.h"
#include "logic/ByteView.h"
#include "misc/util.h"

using namespace std;
using namespace vl;

TEST (Max, forward) {
    string content = makeContent("../test/data/max.txt");
    ByteDocument doc(content.c_str(), content.length(), 40);
    auto vexpect = makeExpect("../test/expect/max_first16m40w100.txt");
    ByteView vtest(&doc);
    vtest.setScreenLineLen(100);
    vtest.setScreenLineCount(21);
    vtest.fillDeque();
    vtest.recalcLines();
    EXPECT_GT(vexpect.size(), 0);
    EXPECT_EQ(vexpect.size(), vtest.size());
    for (int i = 0; i < min(vtest.size(), vexpect.size()); i++)
        EXPECT_EQ(vtest[i], vexpect[i]);
}

TEST(Max, backward) {
    string content = makeContent("../test/data/max.txt");
    ByteDocument doc(content.c_str(), content.length(), 40);
    ByteView vexpect(&doc);
    vexpect.setScreenLineLen(100);
    vexpect.setScreenLineCount(21);
    vexpect.fillDeque();
    vexpect.recalcLines();
    ByteView vtest(&doc);
    vtest.setScreenLineLen(100);
    vtest.setScreenLineCount(vexpect.size());
    vtest.gotoProportional(1);
    vtest.fillDeque();
    vtest.recalcLines();
    EXPECT_GT(vexpect.size(), 0);
    EXPECT_EQ(vexpect.size(), vtest.size());
    for (int i = 0; i < min(vtest.size(), vexpect.size()); i++)
        EXPECT_EQ(vtest[i], vexpect[i]);
}

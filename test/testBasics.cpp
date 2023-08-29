//
// Created by andrzej on 8/27/23.
//
#include "gtest/gtest.h"
#include "logic/ByteDocument.h"
#include "logic/ByteView.h"
#include "misc/util.h"

using namespace std;
using namespace vl;

TEST (Width, empties) {
    string content = makeContent("../test/data/empties.txt");
    ByteDocument doc(content.c_str(), content.length(), true);
    auto vexpect = makeExpect("../test/expect/empties_first20w8.txt");
    ByteView vtest(&doc);
    vtest.setScreenLineLen(8);
    vtest.setScreenLineCount(20);
    vtest.fillDeque();
    vtest.recalcLines();
    EXPECT_EQ(vexpect.size(), vtest.size());
    for (int i = 0; i < min(vtest.size(), vexpect.size()); i++)
        EXPECT_EQ(vtest[i], vexpect[i]);
}

TEST(Scroll, width) {
    string content = makeContent("../test/data/midlines.txt");
    ByteDocument doc(content.c_str(), content.length(), true);
    ByteView vtest(&doc);
    vtest.setScreenLineLen(6);
    vtest.setScreenLineCount(3);
    vtest.fillDeque();
    vtest.recalcLines();
    auto orig = vtest.clone();
    for (int i = 0; i < min(vtest.size(), orig->size()); i++)
        EXPECT_EQ(vtest[i], (*orig)[i]);
    vtest.scrollDown();
    vtest.scrollUp();
    for (int i = 0; i < min(vtest.size(), orig->size()); i++)
        EXPECT_EQ(vtest[i], (*orig)[i]);
    vtest.scrollNDown(3);
    vtest.scrollNUp(3);
    for (int i = 0; i < min(vtest.size(), orig->size()); i++)
        EXPECT_EQ(vtest[i], (*orig)[i]);
}


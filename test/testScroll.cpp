//
// Created by Andrzej Borucki on 2022-09-11
//
//Scroll
#include "gtest/gtest.h"
#include "misc/util.h"
#include "logic/ByteDocument.h"
#include "logic/ByteView.h"

using namespace std;
using namespace vl;

TEST(Scroll, width) {
    string content = makeContent("../test/data/midlines.txt");
    ByteDocument doc(content.c_str(), content.length(), 0);
    ByteView vtest(&doc);
    vtest.setScreenLineLen(6);
    vtest.setScreenLineCount(3);
    vtest.setWrapMode(0);
    vtest.fillDeque();
    vtest.recalcLines();
    auto orig = vtest.clone();
    EXPECT_EQ(vtest.size(), orig->size());
    for (int i = 0; i < min(vtest.size(), orig->size()); i++)
        EXPECT_EQ((*orig)[i], vtest[i]);
    vtest.scrollDown();
    vtest.scrollUp();
    EXPECT_EQ(vtest.size(), orig->size());
    for (int i = 0; i < min(vtest.size(), orig->size()); i++)
        EXPECT_EQ((*orig)[i], vtest[i]);
    vtest.scrollPageDown();
    vtest.scrollPageUp();
    EXPECT_EQ(vtest.size(), orig->size());
    for (int i = 0; i < min(vtest.size(), orig->size()); i++)
        EXPECT_EQ((*orig)[i], vtest[i]);
}

TEST(Scroll, wrap) {
    string content = makeContent("../test/data/midlines.txt");
    ByteDocument doc(content.c_str(), content.length(), 0);
    ByteView vtest(&doc);
    vtest.setScreenLineLen(6);
    vtest.setScreenLineCount(3);
    vtest.setWrapMode(1);
    vtest.fillDeque();
    vtest.recalcLines();
    auto orig = vtest.clone();
    for (int i = 0; i < min(vtest.size(), orig->size()); i++)
        EXPECT_EQ(vtest[i], (*orig)[i]);
    vtest.scrollDown();
    vtest.scrollUp();
    EXPECT_EQ(vtest.size(), orig->size());
    for (int i = 0; i < min(vtest.size(), orig->size()); i++)
        EXPECT_EQ(vtest[i], (*orig)[i]);
    vtest.scrollNDown(2);
    vtest.scrollNUp(2);
    for (int i = 0; i < min(vtest.size(), orig->size()); i++)
        EXPECT_EQ(vtest[i], (*orig)[i]);
}
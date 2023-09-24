//
// Created by Andrzej Borucki on 2022-09-11
//

#include "misc/util.h"
#include "gtest/gtest.h"
#include "logic/ByteDocument.h"
#include "logic/ByteView.h"

using namespace std;
using namespace vl;

TEST(Tabs, nowrap) {
    string content = makeContent("../test/data/tabs.txt");
    ByteDocument doc(content.c_str(), content.length(), 0);
    ByteView vtest(&doc);
    vtest.setScreenLineLen(100);
    vtest.setScreenLineCount(10);
    vtest.setWrapMode(0);
    vtest.setmaxTabW(4);
    auto vexpect = makeExpect("../test/expect/tabs.txt");
    vtest.fillDeque();
    vtest.recalcLines();
    EXPECT_EQ(vexpect.size(), vtest.size());
    for (int i = 0; i < min(vtest.size(), vexpect.size()); i++)
        EXPECT_EQ(vtest[i], vexpect[i]);
}

TEST(Tabs, wrap) {
    string content = makeContent("../test/data/tabs.txt");
    ByteDocument doc(content.c_str(), content.length(), 0);
    ByteView vtest(&doc);
    vtest.setScreenLineLen(6);
    vtest.setScreenLineCount(10);
    vtest.setWrapMode(1);
    vtest.setmaxTabW(4);
    auto vexpect = makeExpect("../test/expect/tabs.wrap.txt");
    vtest.fillDeque();
    vtest.recalcLines();
    EXPECT_EQ(vexpect.size(), vtest.size());
    for (int i = 0; i < min(vtest.size(), vexpect.size()); i++)
        EXPECT_EQ(vtest[i], vexpect[i]);
}

TEST(Tabs, wordwrap) {
    string content = makeContent("../test/data/tabs.txt");
    ByteDocument doc(content.c_str(), content.length(), 0);
    ByteView vtest(&doc);
    vtest.setScreenLineLen(6);
    vtest.setScreenLineCount(10);
    vtest.setWrapMode(2);
    vtest.setmaxTabW(4);
    auto vexpect = makeExpect("../test/expect/tabs.wordwrap.txt");
    vtest.fillDeque();
    vtest.recalcLines();
    EXPECT_EQ(vexpect.size(), vtest.size());
    for (int i = 0; i < min(vtest.size(), vexpect.size()); i++)
        EXPECT_EQ(vtest[i], vexpect[i]);
}

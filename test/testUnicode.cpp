//
// Created by Andrzej Borucki on 2022-09-11
//

#include "misc/util.h"
#include "gtest/gtest.h"
#include "logic/ByteDocument.h"
#include "logic/ByteView.h"

using namespace std;
using namespace vl;
TEST(Unicode, width) {
    string content = makeContent("../test/data/utf8.txt");
    ByteDocument doc(content.c_str(), content.length(), 0);
    ByteView vtest(&doc);
    vtest.setScreenLineLen(10);
    vtest.setScreenLineCount(10);
    vtest.setWrapMode(0);
    auto vexpect = makeExpect("../test/expect/utf8_first10w10.txt");
    vtest.fillDeque();
    vtest.recalcLines();
    EXPECT_EQ(vexpect.size(), vtest.size());
    for (int i = 0; i < min(vtest.size(), vexpect.size()); i++)
        EXPECT_EQ(vtest[i], vexpect[i]);
}

TEST(Unicode, wrap) {
    string content = makeContent("../test/data/utf8.txt");
    ByteDocument doc(content.c_str(), content.length(), 0);
    ByteView vtest(&doc);
    vtest.setScreenLineLen(10);
    vtest.setScreenLineCount(20);
    vtest.setWrapMode(1);
    auto vexpect = makeExpect("../test/expect/utf8_first20wrap10.txt");
    vtest.fillDeque();
    vtest.recalcLines();
    EXPECT_EQ(vexpect.size(), vtest.size());
    for (int i = 0; i < min(vtest.size(), vexpect.size()); i++)
        EXPECT_EQ(vtest[i], vexpect[i]);
}

TEST(Unicode, maxline) {
    string content = makeContent("../test/data/utf8.txt");
    ByteDocument doc(content.c_str(), content.length(), 10);
    ByteView vtest(&doc);
    vtest.setScreenLineLen(100);
    vtest.setScreenLineCount(40);
    vtest.setWrapMode(0);
    auto vexpect = makeExpect("../test/expect/utf8_first40m10.txt");
    vtest.fillDeque();
    vtest.recalcLines();
    EXPECT_EQ(vexpect.size(), vtest.size());
    for (int i = 0; i < min(vtest.size(), vexpect.size()); i++)
        EXPECT_EQ(vtest[i], vexpect[i]);
}

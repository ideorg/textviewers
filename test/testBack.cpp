//
// Created by andrzej on 8/27/23.
//
#include "gtest/gtest.h"
#include "logic/ByteDocument.h"
#include "logic/ByteView.h"
#include "misc/util.h"

using namespace std;
using namespace vl;

/* if resize viewer application, if lines takes full screen height*/
TEST(Back, positions) {
    string content = makeContent("../test/data/empties.txt");
    ByteDocument doc(content.c_str(), content.length(), 0);
    ByteView vexpect(&doc);
    vexpect.setScreenLineLen(8);
    vexpect.setScreenLineCount(50); //enough for all lines
    vexpect.gotoProportional(1);
    vexpect.fillDeque();
    vexpect.recalcLines();
    for (int i = 1; i < vexpect.size(); i++) {
        ByteView vtest(&doc);
        vtest.setScreenLineCount(i);
        vtest.gotoProportional(1);
        vtest.fillDeque();
        vtest.recalcLines();
        ASSERT_EQ(vtest[-1], vexpect[-1]);
    }
}

TEST(Back, createResult) {
    string content = makeContent("../test/data/empties.txt");
    ByteDocument doc(content.c_str(), content.length(), 0);
    ByteView vexpect(&doc);
    vexpect.setScreenLineLen(8);
    vexpect.setScreenLineCount(50);
    vexpect.fillDeque();
    vexpect.recalcLines();
    ByteView vtest(&doc);
    vtest.setScreenLineLen(8);
    vtest.setScreenLineCount(vexpect.size());
    vtest.gotoProportional(1);
    vtest.fillDeque();
    vtest.recalcLines();
    EXPECT_EQ(vtest.size(), vexpect.size());
    for (int i = 0; i < min(vtest.size(), vexpect.size()); i++)
        ASSERT_EQ(vtest[i], vexpect[i]);
}



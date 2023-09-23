//
// Created by Andrzej Borucki on 2022-10-04
//
#include "misc/util.h"
#include "gtest/gtest.h"
#include "logic/ByteDocument.h"
#include "logic/ByteView.h"
#include "UTF/UTF.hpp"

using namespace std;
using namespace vl;

TEST(locatePosition, whole) {
    const string names[] = {"shortwrap.txt", "utf8.txt", "midlines.txt"};
    for (int k = 0; k < 3; k++) {
        string content = makeContent(string("../test/data/") + names[k]);
        ByteDocument doc(content.c_str(), content.length(), 0);
        ByteView vtest(&doc);
        vtest.setScreenLineLen(12);
        vtest.setScreenLineCount(20);//enough
        vtest.setWrapMode(0);
        vtest.fillDeque();
        vtest.recalcLines();
        for (int row = 0; row < vtest.size(); row++) {
            u32string dstr = vtest[row];
            int dlen = dstr.size();
            for (int col = 0; col < vtest.screenLineLen(); col++) {
                int64_t filePosition = vtest.filePosition(row, col);
                auto p = vtest.locatePosition(filePosition, row);
                EXPECT_EQ(p.first, row);
                if (col < dlen)
                    EXPECT_EQ(p.second, col);
                else
                    EXPECT_EQ(p.second, dlen);
            }
        }
    }
}

TEST(locatePosition, fromEnd) {
    const string names[] = {"shortwrap.txt", "shortwrapr.txt", "utf8.txt", "midlines.txt"};
    for (int k = 0; k < 3; k++) {
        string content = makeContent(string("../test/data/") + names[k]);
        ByteDocument doc(content.c_str(), content.length(), 0);
        ByteView vtest(&doc);
        vtest.setScreenLineLen(12);
        vtest.setScreenLineCount(3);
        vtest.setWrapMode(0);
        vtest.gotoProportional(1);
        vtest.fillDeque();
        vtest.recalcLines();
        for (int row = 0; row < vtest.size(); row++) {
            u32string dstr = vtest[row];
            UTF utf;
            int dlen = dstr.size();
            for (int col = 0; col < vtest.screenLineLen(); col++) {
                int64_t filePosition = vtest.filePosition(row, col);
                auto p = vtest.locatePosition(filePosition, row);
                EXPECT_EQ(p.first, row);
                if (col < dlen)
                    EXPECT_EQ(p.second, col);
                else
                    EXPECT_EQ(p.second, dlen);
            }
        }
    }
}

TEST(locatePosition, maxLine) {
    const string names[] = {"shortwrap.txt", "utf8.txt", "midlines.txt"};
    for (int k = 0; k < 3; k++) {
        string content = makeContent(string("../test/data/") + names[k]);
        ByteDocument doc(content.c_str(), content.length(), 0);
        ByteView vtest(&doc);
        vtest.setScreenLineLen(12);
        vtest.setScreenLineCount(12);
        vtest.setWrapMode(0);
        vtest.gotoProportional(1);
        vtest.fillDeque();
        vtest.recalcLines();
        for (int row = 0; row < vtest.size(); row++) {
            u32string dstr = vtest[row];
            UTF utf;
            int dlen = dstr.size();
            for (int col = 0; col < vtest.screenLineLen(); col++) {
                int64_t filePosition = vtest.filePosition(row, col);
                auto p = vtest.locatePosition(filePosition, row);
                EXPECT_EQ(p.first, row);
                if (col < dlen)
                    EXPECT_EQ(p.second, col);
                else
                    EXPECT_EQ(p.second, dlen);
            }
        }
    }
}

TEST(locatePosition, wrap1) {
    const string names[] = {"shortwrap.txt", "utf8.txt", "midlines.txt"};
    for (int k = 0; k < 3; k++) {
        string content = makeContent(string("../test/data/") + names[k]);
        ByteDocument doc(content.c_str(), content.length(), 0);
        ByteView vtest(&doc);
        vtest.setScreenLineLen(10);
        vtest.setScreenLineCount(16);
        vtest.setWrapMode(1);
        vtest.gotoProportional(1);
        vtest.fillDeque();
        vtest.recalcLines();
        for (int row = 0; row < vtest.size(); row++) {
            u32string dstr = vtest[row];
            UTF utf;
            int dlen = dstr.size();
            for (int col = 0; col < vtest.screenLineLen(); col++) {
                int64_t filePosition = vtest.filePosition(row, col);
                auto p = vtest.locatePosition(filePosition, row);
                EXPECT_EQ(p.first, row);
                if (col < dlen)
                    EXPECT_EQ(p.second, col);
                else
                    EXPECT_EQ(p.second, dlen);
            }
        }
    }
}


TEST(locatePosition, wrap2) {
    const string names[] = {"shortwrap.txt", "utf8.txt", "midlines.txt"};
    for (int k = 0; k < 3; k++) {
        string content = makeContent(string("../test/data/") + names[k]);
        ByteDocument doc(content.c_str(), content.length(), 0);
        ByteView vtest(&doc);
        vtest.setScreenLineLen(10);
        vtest.setScreenLineCount(16);
        vtest.setWrapMode(2);
        vtest.gotoProportional(1);
        vtest.fillDeque();
        vtest.recalcLines();
        for (int row = 0; row < vtest.size(); row++) {
            u32string dstr = vtest[row];
            UTF utf;
            int dlen = dstr.size();
            for (int col = 0; col < vtest.screenLineLen(); col++) {
                int64_t filePosition = vtest.filePosition(row, col);
                auto p = vtest.locatePosition(filePosition, col>0);
                EXPECT_EQ(p.first, row);
                if (col < dlen)
                    EXPECT_EQ(p.second, col);
                else
                    EXPECT_EQ(p.second, dlen);
            }
        }
    }
}

TEST(locatePosition, beginX) {
    const string names[] = {"shortwrap.txt", "utf8.txt", "midlines.txt"};
    for (int k = 0; k < 3; k++) {
        string content = makeContent(string("../test/data/") + names[k]);
        ByteDocument doc(content.c_str(), content.length(), 0);
        ByteView vtest(&doc);
        vtest.setScreenLineLen(10);
        vtest.setScreenLineCount(10);
        vtest.setWrapMode(0);
        vtest.gotoProportional(1);
        vtest.fillDeque();
        vtest.recalcLines();
        for (int row = 0; row < vtest.size(); row++) {
            u32string dstr = vtest[row];
            UTF utf;
            int dlen = dstr.size();
            for (int col = 0; col < vtest.screenLineLen(); col++) {
                int64_t filePosition = vtest.filePosition(row, col);
                auto p = vtest.locatePosition(filePosition);
                if (col < dlen) {
                    EXPECT_EQ(p.first, row);
                    EXPECT_EQ(p.second, col);
                } else {
                    EXPECT_EQ(p.first, row);
                    EXPECT_EQ(p.second, dlen);
                }
            }
        }
    }
}

TEST(locatePosition, before) {
    string content = makeContent("../test/data/midlines.txt");
    ByteDocument doc(content.c_str(), content.length(), 0);
    ByteView vtest(&doc);
    vtest.setScreenLineLen(10);
    vtest.setScreenLineCount(5);
    vtest.setWrapMode(0);
    vtest.gotoProportional(1);
    vtest.fillDeque();
    vtest.recalcLines();
    auto p = vtest.locatePosition(0);
    EXPECT_EQ(p.first, -1);
    EXPECT_EQ(p.second, 0);
    int64_t begPos = vtest.filePosition(0, 0);
    p = vtest.locatePosition(begPos - 1);
    EXPECT_EQ(p.first, -1);
    EXPECT_EQ(p.second, 0);
}

TEST(locatePosition, beforeWrap) {
    string content = makeContent("../test/data/midlines.txt");
    ByteDocument doc(content.c_str(), content.length(), 0);
    ByteView vtest(&doc);
    vtest.setScreenLineLen(10);
    vtest.setScreenLineCount(5);
    vtest.setWrapMode(1);
    vtest.gotoProportional(1);
    vtest.fillDeque();
    vtest.recalcLines();
    auto p = vtest.locatePosition(0);
    EXPECT_EQ(p.first, -1);
    EXPECT_EQ(p.second, 0);
    int64_t begPos = vtest.filePosition(0, 0);
    p = vtest.locatePosition(begPos - 1);
    EXPECT_EQ(p.first, -1);
    EXPECT_EQ(p.second, 0);
}

TEST(locatePosition, after) {
    string content = makeContent("../test/data/midlines.txt");
    ByteDocument doc(content.c_str(), content.length(), 0);
    ByteView vtest(&doc);
    vtest.setScreenLineLen(10);
    vtest.setScreenLineCount(5);
    vtest.setWrapMode(0);
    vtest.fillDeque();
    vtest.recalcLines();
    auto p = vtest.locatePosition(doc.byteCount());
    EXPECT_EQ(p.first, vtest.screenLineCount());
    EXPECT_EQ(p.second, 0);
}

TEST(locatePosition, afterWrap) {
    string content = makeContent("../test/data/midlines.txt");
    ByteDocument doc(content.c_str(), content.length(), 0);
    ByteView vtest(&doc);
    vtest.setScreenLineLen(10);
    vtest.setScreenLineCount(5);
    vtest.setWrapMode(1);
    vtest.fillDeque();
    vtest.recalcLines();
    auto p = vtest.locatePosition(doc.byteCount());
    EXPECT_EQ(p.first, vtest.screenLineCount());
    EXPECT_EQ(p.second, 0);
}

TEST(filePosition, rowLT0) {
    string content = makeContent("../test/data/midlines.txt");
    ByteDocument doc(content.c_str(), content.length(), 0);
    ByteView vtest(&doc);
    vtest.setScreenLineLen(10);
    vtest.setScreenLineCount(5);
    vtest.setWrapMode(1);
    vtest.fillDeque();
    vtest.recalcLines();
    auto fpos = vtest.filePosition(-1, 5);
    EXPECT_EQ(fpos, 0);
}

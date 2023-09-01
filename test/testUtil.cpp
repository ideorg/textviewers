//
// Created by andrzej on 8/28/23.
//
#include "gtest/gtest.h"
#include "misc/util.h"

using namespace std;


TEST (Util, lineBreaks) {
    vector<int> A = {10, 0, 15};
    deque<LBInfo> Aest = {{0, 10, 1}, {11, 0, 1}, {12, 15, 0}};
    auto Aactual = getLineBreaks(A, 0);
    EXPECT_EQ(Aest, Aactual);

    vector<int> B = {10, 0, 15, 0};
    deque<LBInfo> Best = {{0, 10, 1}, {11, 0, 1}, {12, 15, 0}, {27, 0, 1}};
    auto Bactual = getLineBreaks(B, 0);
    EXPECT_EQ(Best, Bactual);
}

TEST (Util, breakPoints) {
    vector<int64_t> range0actual = computeBreakPoints(39, 80, 40);
    vector<int64_t> range0est = {40};
    EXPECT_EQ(range0est, range0actual);
    vector<int64_t> range1actual = computeBreakPoints(39, 81, 40);
    vector<int64_t> range1est = {40, 80};
    EXPECT_EQ(range1est, range1actual);
    vector<int64_t> range2actual = computeBreakPoints(40, 81, 40);
    vector<int64_t> range2est = {80};
    EXPECT_EQ(range2est, range2actual);
    vector<int64_t> range3actual = computeBreakPoints(40, 80, 40);
    vector<int64_t> range3est = {};
    EXPECT_EQ(range3est, range3actual);
    vector<int64_t> range4actual = computeBreakPoints(41, 80, 40);
    vector<int64_t> range4est = {};
    EXPECT_EQ(range4est, range4actual);
}

TEST (Util, maxLineLen) {
    deque<LBInfo> A = {{0, 60, 1}, {61, 20, 1}, {82, 70, 0}};
    deque<LBInfo> B = {{0, 90, 1}, {91, 110, 1}, {202, 50, 0}};
    deque<LBInfo> C = {{0, 10, 1}, {11, 110, 1}, {122, 50, 0}};
    //maxLineLen = 40
    deque<LBInfo> Aest = {{0, 40, 0}, {40, 20, 1}, {61, 20, 1}, {82, 70, 0}};
    deque<LBInfo> Best = {{0, 40, 0}, {40, 40, 0}, {80, 10, 1}, {91, 69, 0}, {160, 40, 0}, {200, 1, 1}, {202, 50, 0}};
    deque<LBInfo> Cest = {{0, 10, 1}, {11, 69, 0}, {80, 40, 0}, {120, 1, 1}, {122, 50, 0}};

    auto Aactual = getMaxLineBreaks(A, 40);
    auto Bactual = getMaxLineBreaks(B, 40);
    auto Cactual = getMaxLineBreaks(C, 40);

    EXPECT_EQ(Aest, Aactual);
    EXPECT_EQ(Best, Bactual);
    EXPECT_EQ(Cest, Cactual);
}

TEST (Util, divideUnicode) {
    LBInfo A0 = {0, 41, 2};
    vector<int64_t> A0breaks = {20, 40};
    deque<LBInfo> A0est = {{0, 21, 0}, {21, 20, 2}};
    auto A0actual = divideUnicodeToBreaks(A0, A0breaks, 3);
    EXPECT_EQ(A0est, A0actual);

    LBInfo A1 = {143, 40, 2};
    vector<int64_t> A1breaks = {160, 180};
    deque<LBInfo> A1est = {{143, 18, 0}, {161, 21, 0}, {182, 1, 2}};
    auto A1actual = divideUnicodeToBreaks(A1, A1breaks, 3);
    EXPECT_EQ(A1est, A1actual);

    LBInfo A = {18, 10, 2};
    vector<int64_t> Abreaks = {20};
    deque<LBInfo> Aest = {{18, 3, 0}, {21, 7, 2}};
    auto Aactual = divideUnicodeToBreaks(A, Abreaks, 3);
    EXPECT_EQ(Aest, Aactual);

    LBInfo B = {20, 10, 2};
    vector<int64_t> Bbreaks = {20, 40};
    deque<LBInfo> Best = {{20, 0, 0}, {20, 10, 2}};
    auto Bactual = divideUnicodeToBreaks(B, Bbreaks, 3);
    EXPECT_EQ(Best, Bactual);

    LBInfo C = {20, 20, 2};
    vector<int64_t> Cbreaks = {20, 40};
    deque<LBInfo> Cest = {{20, 0, 0}, {20, 20, 2}};
    auto Cactual = divideUnicodeToBreaks(C, Cbreaks, 3);
    EXPECT_EQ(Cest, Cactual);

    LBInfo D = {20, 21, 2};
    vector<int64_t> Dbreaks = {20, 40};
    deque<LBInfo> Dest = {{20, 0, 0}, {20, 21, 2}};
    auto Dactual = divideUnicodeToBreaks(D, Dbreaks, 3);
    EXPECT_EQ(Dest, Dactual);

    LBInfo E = {1, 50, 1};
    vector<int64_t> Ebreaks = {20, 40};
    deque<LBInfo> Eest = {{1, 21, 0}, {22, 18, 0}, {40, 11, 1}};
    auto Eactual = divideUnicodeToBreaks(E, Ebreaks, 3);
    EXPECT_EQ(Eest, Eactual);

    //not realistic case, utfLen=7, lineBeraks difference not queal
    LBInfo F = {3, 100, 1};
    vector<int64_t> Fbreaks = {11, 27, 64};
    deque<LBInfo> Fest = {{3, 14, 0}, {17, 14, 0}, {31, 35, 0}, {66, 37, 1}};
    auto Factual = divideUnicodeToBreaks(F, Fbreaks, 7);
    EXPECT_EQ(Fest, Factual);

    LBInfo G = {3, 64, 1};
    vector<int64_t> Gbreaks = {11, 27, 64};
    deque<LBInfo> Gest = {{3, 14, 0}, {17, 14, 0}, {31, 35, 0}, {66, 1, 1}};
    auto Gactual = divideUnicodeToBreaks(G, Gbreaks, 7);
    EXPECT_EQ(Gest, Gactual);
}

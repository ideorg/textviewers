//
// Created by andrzej on 8/28/23.
//
#include "gtest/gtest.h"
#include "misc/util.h"

using namespace std;


TEST (Util, lineBreaks) {
    vector<int> A = {10, 0, 15};
    vector<pair<int, int>> Asimple = {{10, 1}, {0, 1}, {15, 1}};
    auto AsimpleActual = getLineBreaks(A, 0, 1);
    vector<pair<int, int>> AbutLast = {{10, 1}, {0, 1}, {15, 0}};
    auto AbutLastActual = getLineBreaks(A, 0, 0);
    vector<pair<int, int>> Asmart = {{10, 1}, {0, 1}, {15, 0}};
    auto AsmartActual = getLineBreaks(A, 0, 2);
    EXPECT_EQ(Asimple, AsimpleActual);
    EXPECT_EQ(AbutLast, AbutLastActual);
    EXPECT_EQ(Asmart, AsmartActual);

    vector<int> B = {10, 0, 15, 0};
    vector<pair<int, int>> Bsimple = {{10, 1}, {0, 1}, {15, 1}, {0, 1}};
    auto BsimpleActual = getLineBreaks(B, 0, 1);
    vector<pair<int, int>> BbutLast = {{10, 1}, {0, 1}, {15, 1}, {0, 1}};
    auto BbutLastActual = getLineBreaks(B, 0, 0);
    vector<pair<int, int>> Bsmart = {{10, 1}, {0, 1}, {15, 0}, {0, 1}};
    auto BsmartActual = getLineBreaks(B, 0, 2);
    EXPECT_EQ(Bsimple, BsimpleActual);
    EXPECT_EQ(BbutLast, BbutLastActual);
    EXPECT_EQ(Bsmart, BsmartActual);
}

TEST (Util, breakPoints) {
    vector<int64_t> range0actual = computeBreakPoints(39, 80, 40);
    vector<int64_t> range0est = {40};
    EXPECT_EQ(range0est, range0actual);
    vector<int64_t> range1actual = computeBreakPoints(39, 81, 40);
    vector<int64_t> range1est = {40, 80};
    EXPECT_EQ(range1est, range1actual);
    vector<int64_t> range2actual = computeBreakPoints(40, 81, 40);
    vector<int64_t> range2est = {40, 80};
    EXPECT_EQ(range2est, range2actual);
    vector<int64_t> range3actual = computeBreakPoints(40, 80, 40);
    vector<int64_t> range3est = {40};
    EXPECT_EQ(range3est, range3actual);
    vector<int64_t> range4actual = computeBreakPoints(41, 80, 40);
    vector<int64_t> range4est = {};
    EXPECT_EQ(range4est, range4actual);
}

TEST (Util, maxLineLen) {
    vector<pair<int, int>> A = {{60, 1}, {20, 1}, {70, 0}};
    vector<pair<int, int>> B = {{90, 1}, {110, 1}, {50, 0}};
    vector<pair<int, int>> C = {{10, 1}, {110, 1}, {50, 0}};
    //maxLineLen = 40
    vector<pair<int, int>> Aest = {{40, 0}, {20, 1}, {20, 1}, {70, 0}};
    vector<pair<int, int>> Best = {{40, 0}, {40, 0}, {10, 1}, {69, 0}, {40, 0}, {1, 1}, {50, 0}};
    vector<pair<int, int>> Cest = {{10, 1}, {69, 0}, {40, 0}, {1, 1}, {50, 0}};

    auto Aactual = getMaxLineBreaks(A, 40);
    auto Bactual = getMaxLineBreaks(B, 40);
    auto Cactual = getMaxLineBreaks(C, 40);

    EXPECT_EQ(Aest, Aactual);
    EXPECT_EQ(Best, Bactual);
    EXPECT_EQ(Cest, Cactual);
}
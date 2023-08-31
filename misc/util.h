//
// Created by andrzej on 2022-08-25
//

#ifndef VIEWER_UTIL_H
#define VIEWER_UTIL_H

#include <string>
#include <vector>
#include <deque>

std::vector<std::string> readLines(std::string filename);
std::string makeContent(std::string filename, int nEols = 0, bool rn = false);
std::string makeString(std::string filename, int nEols = 0, bool rn = false);
std::vector<std::u32string> makeExpect(std::string filename);
std::vector<int> getSizes(std::string filename);
std::vector<int> getSizesStr(std::string filename);
std::string makeStringN(std::string filename, int n);
std::vector<int> multiply(std::vector<int>& lineLens, int factor);
std::string genSampleLineBreaks(std::vector<int> lineLens, int lineBreaksKind, int lineBreakAtEnd);
std::string genSampleUnicode(std::vector<int> lineLens, int utf8len);
std::vector<int64_t> computeBreakPoints(int64_t offset, int64_t end, int maxLineLen);

struct LBInfo {
    int offset;
    int len;
    int breaks;
    friend bool operator==(const LBInfo &first, const LBInfo &other) {
        return first.offset == other.offset && first.len == other.len && first.breaks == other.breaks;
    }
};
std::deque<LBInfo> divideUnicodeToBreaks(LBInfo line, std::vector<int64_t> breaks, int utf8len);
std::deque<LBInfo> getLineBreaks(std::vector<int> lineLens, int lineBreaksKind, int lineBreakAtEnd);
std::deque<LBInfo> getMaxLineBreaks(std::deque<LBInfo> lineBreaks, int maxLineLen, int utf8len=1);

#endif //VIEWER_UTIL_H

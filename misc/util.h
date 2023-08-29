//
// Created by andrzej on 2022-08-25
//

#ifndef VIEWER_UTIL_H
#define VIEWER_UTIL_H

#include <string>
#include <vector>

std::vector<std::string> readLines(std::string filename);
std::string makeContent(std::string filename, int nEols = 0, bool rn = false);
std::string makeString(std::string filename, int nEols = 0, bool rn = false);
std::vector<std::u32string> makeExpect(std::string filename);
std::vector<int> getSizes(std::string filename);
std::vector<int> getSizesStr(std::string filename);
std::string makeStringN(std::string filename, int n);
std::vector<int> multiply(std::vector<int>& lineLens, int factor);
std::string genSampleLineBreaks(std::vector<int> lineLens, int lineBreaksKind, int lineBreakAtEnd);

#endif //VIEWER_UTIL_H

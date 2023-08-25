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
std::vector<std::u16string> makeExpect(std::string filename);
std::vector<int> getSizes(std::string filename);
std::vector<int> getSizesStr(std::string filename);
std::string makeStringN(std::string filename, int n);

#endif //VIEWER_UTIL_H

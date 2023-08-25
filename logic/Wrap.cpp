//
// Created by andrzej on 8/23/23.
//

#include "Wrap.h"

using namespace vl;

std::vector<int> Wrap::wrapEnds(std::string_view lineVIew) {
    std::vector<int> vec;
    vec.push_back((int)lineVIew.size());
    return vec;
}

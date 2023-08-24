//
// Created by andrzej on 8/23/23.
//

#include "Wrap.h"

using namespace vl;

int Wrap::wrapCount() {
    return 0;
}

std::vector<int> Wrap::wrapEnds() {
    return std::vector<int>();
}

void Wrap::setScreenLineLen(int screenLineLen) {
    m_screenLineLen = screenLineLen;
}

int Wrap::screenLineLen() {
    return m_screenLineLen;
}

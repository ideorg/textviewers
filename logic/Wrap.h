//
// Created by andrzej on 8/23/23.
//

#ifndef VIEWER_WRAP_H
#define VIEWER_WRAP_H

#include <vector>

namespace vl {

class Wrap {
    int m_screenLineLen = 0;
public:
    void setScreenLineLen(int screenLineLen);
    int screenLineLen();
    int wrapCount();
    std::vector<int> wrapEnds();
};
}

#endif //VIEWER_WRAP_H

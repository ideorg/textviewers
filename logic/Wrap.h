//
// Created by andrzej on 8/23/23.
//

#ifndef VIEWER_WRAP_H
#define VIEWER_WRAP_H

#include <vector>
#include <string_view>

namespace vl {

class Wrap {
    int m_screenLineLen = 0;
public:
    void setScreenLineLen(int screenLineLen);
    int screenLineLen();
    std::vector<int> wrapEnds(std::string_view lineVIew);
};
}

#endif //VIEWER_WRAP_H

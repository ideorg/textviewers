//
// Created by andrzej on 8/23/23.
//

#ifndef VIEWER_ABSTRACTVIEW_H
#define VIEWER_ABSTRACTVIEW_H


#include <cstdint>
#include <vector>
#include "IDeque.h"
#include "Wrap.h"

namespace vl {

struct IndexView {
    int index = 0;
    int wrapIndex = 0;
    int64_t wrapOffset;
};

class AbstractView {
private:
    IDeque *viewDeque;
    int countWrapBefore = 0;
    int countWrapAfter = 0;
    bool m_wrapMode = 0;
protected:
    int m_screenLineCount = 0;
    int64_t start;
    std::vector<IndexView> indexView;
public:
    int scrollDown();
    int scrollUp();
    bool wrapMode();
    virtual void gotoProportional(double relativePos) = 0;
    void fillDeque();
    void recalcLines();
};
}

#endif //VIEWER_ABSTRACTVIEW_H

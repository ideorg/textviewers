//
// Created by andrzej on 8/23/23.
//

#ifndef VIEWER_ABSTRACTVIEW_H
#define VIEWER_ABSTRACTVIEW_H


#include <cstdint>
#include <vector>
#include <string>
#include <memory>
#include "IDeque.h"
#include "Wrap.h"

namespace vl {

struct IndexView {
    int index = 0;
    int wrapIndex = 0;
    int64_t wrapOffset;
};

class AbstractView {
protected:
    IDeque* viewDeque = nullptr;
    int countWrapBefore = 0;
    int countWrapAfter = 0;
    int m_wrapMode = 0;
    int64_t m_startY = 0;
    int64_t m_startX = 0;
    std::vector<IndexView> indexView;
    int m_screenLineCount = 0;
    int m_screenLineLen = 0;
    std::unique_ptr<Wrap> wrap;
    void cloneFields(AbstractView *other);
public:
    AbstractView();
    virtual ~AbstractView();
    void setScreenLineCount(int screenLineCount);
    void setScreenLineLen(int screenLineLen);
    int screenLineCount();
    int screenLineLen();
    size_t size();
    int64_t startX();
    int64_t startY();
    double startYproportional();
    std::u32string at(int n);
    std::u32string operator[](int n);
    int scrollDown();
    int scrollUp();
    int scrollNDown(int n);
    int scrollNUp(int n);
    int wrapMode();
    int setWrapMode(int wrapMode);
    virtual void gotoProportional(double relativePos) = 0;
    void fillDeque();
    void recalcLines();
    bool lastInFile(int row);
    virtual int64_t getMinimum() = 0;
    virtual int64_t getMaximum() = 0;
    virtual int64_t getWindowedMinimum() = 0;
    virtual int64_t getWindowedMaximum() = 0;
    int64_t getRange();
    int64_t getWindowedRange();
    int64_t getScrollRange();
};
}

#endif //VIEWER_ABSTRACTVIEW_H

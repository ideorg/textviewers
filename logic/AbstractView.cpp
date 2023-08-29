//
// Created by andrzej on 8/23/23.
//

#include <string>
#include "AbstractView.h"
#include "DString.h"
#include "ByteDeque.h"

using namespace std;
using namespace vl;

int AbstractView::scrollDown() {
    if (viewDeque->empty())
        return 0;
    if (wrapMode()) {
        if (viewDeque->backAtEnd() && countWrapAfter == 0)
            return 0;
        countWrapAfter--;
        if (countWrapAfter <= 0) {
            viewDeque->pushBack(wrap);
            countWrapAfter = viewDeque->backWrapCount();
        }
        countWrapBefore++;
        if (countWrapBefore == viewDeque->frontWrapCount()) {
            countWrapBefore = 0;
            viewDeque->popFront();
        }
        m_startY = viewDeque->getMinimum();
        return 1;
    } else {
        if (viewDeque->backAtEnd())
            return 0;
        viewDeque->pushBack(wrap);
        viewDeque->popFront();
        m_startY = viewDeque->getMinimum();
        return 1;
    }
}

int AbstractView::scrollUp() {
    if (viewDeque->empty())
        return 0;
    if (wrapMode()) {
        if (viewDeque->frontAtStart() && countWrapBefore == 0)
            return 0;
        countWrapBefore--;
        if (countWrapBefore <= 0) {
            viewDeque->pushFront(wrap);
                countWrapAfter = viewDeque->frontWrapCount();
        }
        countWrapAfter++;
        if (countWrapAfter == viewDeque->backWrapCount()) {
            countWrapAfter = 0;
            viewDeque->popBack();
        }
        m_startY = viewDeque->getMinimum();
        return 1;
    } else {
        if (viewDeque->frontAtStart())
            return 0;
        viewDeque->pushFront(wrap);
        viewDeque->popBack();
        m_startY = viewDeque->getMinimum();
        return 1;
    }
}

bool AbstractView::wrapMode() {
    return m_wrapMode;
}

void AbstractView::fillDeque() {
    viewDeque->clear();
    viewDeque->setFront(m_startY);
    if (wrapMode()) {
        int row = 0;
        while (row < m_screenLineCount) {
            viewDeque->pushBack(wrap);
            row += viewDeque->frontWrapCount();
        }
    }
    else {
        for (int i = 0; i < m_screenLineCount; i++) {
            viewDeque->pushBack(wrap);
            if (viewDeque->backAtEnd())
                break;
        }
    }
}

void AbstractView::recalcLines() {
    indexView.clear();
    for (int i = 0; i < viewDeque->size(); i++) {
        for (int j = 0; j < viewDeque->backWrapCount(); j++) {
            IndexView iv;
            iv.index = i;
            iv.wrapIndex = j;
            iv.wrapOffset = viewDeque->backWrapOffset(j);
            indexView.push_back(iv);
        }
    }
}

size_t AbstractView::size() {
    return indexView.size();
}

std::u32string AbstractView::operator[](int n) {
    return at(n);
}

void AbstractView::setScreenLineCount(int screenLineCount) {
    m_screenLineCount = screenLineCount;
}

void AbstractView::setScreenLineLen(int screenLineLen) {
    m_screenLineLen = screenLineLen;
}

int AbstractView::screenLineCount() {
    return m_screenLineCount;
}

int AbstractView::screenLineLen() {
    return m_screenLineLen;
}

bool AbstractView::lastInFile(int row) {
    auto iv = indexView[row];
    if (iv.index != viewDeque->size() - 1)
        return false;
    if (iv.wrapIndex != viewDeque->backWrapCount() - 1)
        return false;
    return viewDeque->backAtEnd();
}

std::u32string AbstractView::at(int n) {
    if (n < 0)
        n = (int)indexView.size() + n;
    DString dstr;
    auto iv = indexView[n];
    auto lineView = viewDeque->lineAt(iv.index);
    UTF utf;
    return DString::substr(lineView, 0, m_screenLineLen);
}

int64_t AbstractView::getRange() {
    return getMaximum() - getMinimum();
}

int64_t AbstractView::getWindowedRange() {
    return getWindowedMaximum() - getWindowedMinimum();
}

int64_t AbstractView::getScrollRange() {
    return getRange() - getWindowedRange();
}

int AbstractView::scrollNDown(int n) {
    int result = 0;
    for (int i = 0; i < n; i++) {
        int res = scrollDown();
        if (res < 1)
            return result;
        result += res;
    }
    return result;
}

int AbstractView::scrollNUp(int n) {
    int result = 0;
    for (int i = 0; i < n; i++) {
        int res = scrollUp();
        if (res < 1)
            return result;
        result += res;
    }
    return result;
}

int64_t AbstractView::startX() {
    return m_startX;
}

int64_t AbstractView::startY() {
    return m_startY;
}

double AbstractView::startYproportional() {
    return (double)startY() / (double)getScrollRange();
}

void AbstractView::cloneFields(AbstractView *other) {
    other->viewDeque = viewDeque->clone();
    other->countWrapBefore = countWrapBefore;
    other->countWrapAfter = countWrapAfter;
    other->m_wrapMode = m_wrapMode;
    other->m_startY = m_startY;
    other->m_startX = m_startX;
    other->indexView = indexView;
    other->m_screenLineCount = m_screenLineCount;
    other->m_screenLineLen = m_screenLineLen;
    other->wrap = wrap;
}

AbstractView::~AbstractView() {
    delete viewDeque;
}


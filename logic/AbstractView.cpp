//
// Created by andrzej on 8/23/23.
//
#include <string>
#include "AbstractView.h"
#include "ByteDeque.h"
#include "UTF/UTF.hpp"

using namespace std;
using namespace vl;

AbstractView::AbstractView() {
    wrap = make_unique<Wrap>(this);
}

AbstractView::~AbstractView() {
    delete viewDeque;
}
int AbstractView::scrollDown() {
    if (viewDeque->empty())
        return 0;
    if (wrapMode()) {
        if (viewDeque->backAtEnd() && countWrapAfter == 0)
            return 0;
        countWrapAfter--;
        if (countWrapAfter <= 0) {
            viewDeque->pushBack(wrap.get());
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
        viewDeque->pushBack(wrap.get());
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
            viewDeque->pushFront(wrap.get());
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
        viewDeque->pushFront(wrap.get());
        viewDeque->popBack();
        m_startY = viewDeque->getMinimum();
        return 1;
    }
}

int AbstractView::wrapMode() {
    return m_wrapMode;
}

void AbstractView::fillDeque() {
    viewDeque->clear();
    viewDeque->setFront(m_startY);
    if (wrapMode()) {
        assert(countWrapBefore >= 0);
        int row = -countWrapBefore;
        while (row < m_screenLineCount) {
            if (!viewDeque->pushBack(wrap.get()))
                return;
            int wrapCount = viewDeque->backWrapCount();
            row += wrapCount;
        }
        countWrapAfter = row - m_screenLineCount;
        assert(countWrapAfter >= 0);
    }
    else {
        for (int i = 0; i < m_screenLineCount; i++) {
            viewDeque->pushBack(wrap.get());
            if (viewDeque->backAtEnd())
                break;
        }
    }
}

void AbstractView::recalcLines() {
    indexView.clear();
    for (int i = 0; i < viewDeque->size(); i++) {
        int startj = i == 0 ? countWrapBefore : 0;
        for (int j = startj; j < viewDeque->wrapCount(i); j++) {
            IndexView iv;
            iv.index = i;
            iv.wrapIndex = j;
            iv.wrapOffset = viewDeque->wrapOffset(i, j);
            iv.wrapLen = viewDeque->wrapLen(i, j);
            indexView.push_back(iv);
            if (indexView.size() >= m_screenLineCount)
                break;
        }
    }
    assert(indexView.size() <= m_screenLineCount);
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
    auto iv = indexView[n];
    if (startX() >= iv.wrapLen)
        return {};
    auto lineView = viewDeque->lineAt(iv.index);
    auto wrapLineView = string_view(lineView.cbegin() + iv.wrapOffset, iv.wrapLen);
    UTF utf;
    int64_t actual;
    const char *s = utf.forwardNcodes(wrapLineView.cbegin(), startX(), wrapLineView.cend(), actual);
    if (s == wrapLineView.cend())
        return {};
    u32string dstr;
    dstr.resize(screenLineLen());
    assert(s < wrapLineView.cend());
    int width = wrap->fillDString(s, wrapLineView.cend(), dstr);
    dstr.resize(width);
    return dstr;
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

int AbstractView::scrollPageDown() {
    return scrollNDown(screenLineCount());
}

int AbstractView::scrollPageUp() {
    return scrollNUp(screenLineCount());
}

int AbstractView::startX() {
    return m_startX;
}

int64_t AbstractView::startY() {
    return m_startY;
}

int AbstractView::maxTabW() {
    return m_maxTabW;
}

double AbstractView::startYproportional() {
    return (double)startY() / (double)getScrollRange();
}

void AbstractView::cloneFields(AbstractView *other) {
    other->wrap = make_unique<Wrap>(other);
    other->viewDeque = viewDeque->clone();
    other->countWrapBefore = countWrapBefore;
    other->countWrapAfter = countWrapAfter;
    other->m_wrapMode = m_wrapMode;
    other->m_startY = m_startY;
    other->m_startX = m_startX;
    other->indexView = indexView;
    other->m_screenLineCount = m_screenLineCount;
    other->m_screenLineLen = m_screenLineLen;
    *other->wrap = *wrap;
}

void AbstractView::setWrapMode(int wrapMode) {
    m_wrapMode = wrapMode;
    if (wrapMode == 0)
        countWrapBefore = 0;
    fillDeque();
    recalcLines();
}

void AbstractView::setmaxTabW(int maxTabW) {
    m_maxTabW = maxTabW;
    fillDeque();
    recalcLines();
}

void AbstractView::setStartX(int startX) {
    m_startX = startX;
    fillDeque();
    recalcLines();
}

LinePointers AbstractView::getLinePointers(int n) {
    LinePointers result;
    if (n < 0)
        n = indexView.size() + n;
    IndexView iv = indexView[n];
    auto deqLine = viewDeque->lineAt(iv.index);
    result.beginLine = deqLine.cbegin();
    result.wrapPosition = deqLine.cbegin() + iv.wrapOffset;
    result.wrapEnd = result.wrapPosition + iv.wrapLen;
    return result;
}

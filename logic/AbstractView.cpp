//
// Created by andrzej on 8/23/23.
//

#include "AbstractView.h"

using namespace vl;

int AbstractView::scrollDown() {
    if (viewDeque->empty())
        return 0;
    if (wrapMode()) {
        if (viewDeque->backAtEnd() && countWrapAfter == 0)
            return 0;
        countWrapAfter--;
        if (countWrapAfter <= 0) {
            viewDeque->pushBack();
            countWrapAfter = viewDeque->backWrapCount();
        }
        countWrapBefore++;
        if (countWrapBefore == viewDeque->fronWrapCount()) {
            countWrapBefore = 0;
            viewDeque->popFront();
        }
        return 1;
    } else {
        if (viewDeque->backAtEnd())
            return 0;
        viewDeque->pushBack();
        viewDeque->popFront();
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
            viewDeque->pushFront();
                countWrapAfter = viewDeque->fronWrapCount();
        }
        countWrapAfter++;
        if (countWrapAfter == viewDeque->backWrapCount()) {
            countWrapAfter = 0;
            viewDeque->popBack();
        }
        return 1;
    } else {
        if (viewDeque->frontAtStart())
            return 0;
        viewDeque->pushFront();
        viewDeque->popBack();
        return 1;
    }
}

bool AbstractView::wrapMode() {
    return m_wrapMode;
}

void AbstractView::fillDeque() {
    viewDeque->clear();
    viewDeque->setFront(start);
    if (wrapMode()) {
        int row = 0;
        while (row < m_screenLineCount) {
            viewDeque->pushBack();
            row += viewDeque->fronWrapCount();
        }
    }
    else {
        for (int i = 0; i < m_screenLineCount; i++) {
            viewDeque->pushBack();
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

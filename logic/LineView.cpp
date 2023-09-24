//
// Created by andrzej on 8/23/23.
//

#include "LineView.h"
#include "LineDeque.h"
#include "UTF/UTF.hpp"
#include <algorithm>
#include <cmath>
#include <iostream>

using namespace std;
using namespace vl;

LineView::LineView(ILineAccess *lineAccess): m_lineAccess(lineAccess) {
    viewDeque = new LineDeque(m_lineAccess);
}

void LineView::gotoProportional(double relativePos) {
    if (m_lineAccess->lineCount() == 0)
        return;
    else {
        if (wrapMode() == 0) {
            if (m_lineAccess->lineCount() <= m_screenLineCount)
                m_startY = 0;
            else
                m_startY = round((m_lineAccess->lineCount() - m_screenLineCount) * relativePos);
        } else {
            double startInside = beginTail() * relativePos;
            m_startY = (int) startInside;
            auto line = m_lineAccess->lineByIndex(m_startY).value();
            auto v = wrap->wrapEnds(line);
            countWrapBefore = round((startInside - m_startY) * v.size());
        }
    }
}

int64_t LineView::getMaximum() {
    return m_lineAccess->lineCount();
}

int64_t LineView::getMinimum() {
    return 0;
}

int64_t LineView::getWindowedMinimum() {
    if (indexView.empty())
        return getMinimum();
    else
        return viewDeque->getMinimum();
}

int64_t LineView::getWindowedMaximum() {
    if (indexView.empty())
        return getMinimum();
    else
        return viewDeque->getMaximum();
}

LineView *LineView::clone() {
    auto newObj = new LineView(m_lineAccess);
    cloneFields(newObj);
    return newObj;
}

void LineView::cloneFields(LineView *other) {
    AbstractView::cloneFields(other);
    other->m_lineAccess = m_lineAccess;
}

double LineView::beginTail() {
    int n = m_lineAccess->lineCount() - 1;
    if (n < 0)
        return 0;
    if (wrapMode() == 0) {
        return n - (screenLineCount() - 1);
    } else {
        int count = 0;
        auto line = m_lineAccess->lineByIndex(n).value();
        auto v = wrap->wrapEnds(line);
        int countWrap = v.size() - 1;
        while (count < screenLineCount() - 1) {
            countWrap--;
            if (countWrap < 0) {
                n--;
                assert(n >= 0);
                auto opt = m_lineAccess->lineByIndex(n);
                line = opt.value();
                v = wrap->wrapEnds(line);
                countWrap = v.size() - 1;
                if (n == 0)
                    break;
            }
            count++;
        }
        double position = n;
        if (countWrap > 0)
            position += double(countWrap) / v.size();
        return position;
    }
}

FilePosition LineView::filePosition(int row, int col) {
    FilePosition result;
    result.interpretation = 2;
    if (row < 0) {
        result.bytePosition = 0;
        return result;
    }
    IndexView iv = indexView[row];
    result.lineNumber = viewDeque->getFront() + iv.index;
    LinePointers lptrs = getLinePointers(row);
    UTF utf;
    int64_t actual;
    result.offset = utf.forwardNcodes(lptrs.wrapPosition, col, lptrs.wrapEnd, actual) - lptrs.beginLine;
    return result;
}

std::pair<int, int> LineView::locatePosition(FilePosition filePosition, bool preferAfter) {
    if (indexView.empty()) return {0, 0};
    int wholeRowNumber = viewDeque->locateRow(filePosition);
    if (wholeRowNumber < 0)
        return {-1, 0};
    auto deqLine = viewDeque->lineAt(wholeRowNumber);
    if (indexView[0].index > wholeRowNumber)
        return {-1, 0};
    int i = 0;
    while (i < indexView.size() && indexView[i].index < wholeRowNumber) {
        i++;
    }
    if (i == indexView.size())
        return {screenLineCount(), 0};
    if (indexView[i].wrapOffset > filePosition.offset)
        return {-1, 0};
    pair<int, int> p;
    int j = i;
    p.first = -1;
    while (j < indexView.size() && indexView[j].index == wholeRowNumber) {
        int64_t ivoffset = indexView[j].wrapOffset;
        if (ivoffset == filePosition.offset) {
            if (preferAfter && j > i)
                p.first = j - 1;
            else
                p.first = j;
            break;
        }
        if (ivoffset > filePosition.offset) {
            p.first = j - 1;
            break;
        }
        j++;
    }
    if (p.first == -1)
        p.first = j - 1;
    UTF utf;
    p.second = utf.numCodesBetween(deqLine.cbegin() + indexView[p.first].wrapOffset,
                                   deqLine.cbegin() + filePosition.offset);
    return p;
}

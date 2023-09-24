//
// Created by andrzej on 8/23/23.
//

#include "LineView.h"
#include "LineDeque.h"
#include <algorithm>
#include <cmath>
#include <iostream>
#include "DString.h"

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
                auto opt = m_lineAccess->lineByIndex(n);
                if (!opt)
                    break;
                line = opt.value();
                v = wrap->wrapEnds(line);
                countWrap = v.size() - 1;
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
    return result;
}

std::pair<int, int> LineView::locatePosition(FilePosition filePosition, bool preferAfter) {
    return std::pair<int, int>();
}

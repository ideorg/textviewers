//
// Created by andrzej on 8/26/23.
//

#include "ByteView.h"
#include "DString.h"
#include "ByteDeque.h"
#include <memory>
#include <cmath>
#include <iostream>

using namespace vl;
using namespace std;

ByteView::ByteView(IByteAccess *byteAccess) : m_byteAccess(byteAccess) {
    viewDeque = new ByteDeque(m_byteAccess);
}

int64_t ByteView::beginTail() {
    auto opt = m_byteAccess->lastLine();
    if (!opt)
        return m_byteAccess->firstByte();
    auto lp = opt.value();
    if (wrapMode() == 0) {
        for (int i = 0; i < screenLineCount() - 1; i++) {
            auto opt = m_byteAccess->lineBefore(lp);
            if (!opt)
                break;
            lp = opt.value();
        }
        return lp.offset;
    } else {
        int count = 0;
        auto line = m_byteAccess->line(lp);
        auto v = wrap->wrapEnds(line);
        int countWrap = v.size() - 1;
        while (count < screenLineCount() - 1) {
            countWrap--;
            if (countWrap < 0) {
                opt = m_byteAccess->lineBefore(lp);
                if (!opt)
                    break;
                lp = opt.value();
                line = m_byteAccess->line(lp);
                v = wrap->wrapEnds(line);
                countWrap = v.size() - 1;
            }
            count++;
        }
        int64_t position = lp.offset;
        if (countWrap > 0)
            position += v[countWrap - 1];
        return position;
    }
}

void ByteView::gotoProportional(double relativePos) {
    auto startByteInside = (int64_t) ceill((long double) beginTail() * relativePos);
    auto linePoint = m_byteAccess->lineEnclosing(startByteInside);
    m_startY = linePoint.offset;
    if (wrapMode()) {
        wrap->wrapEnds(m_byteAccess->line(linePoint));
        if (startByteInside > m_startY) {
            auto line = m_byteAccess->line(linePoint);
            auto v = wrap->wrapEnds(line);
            v = wrap->wrapEnds(line);
            int64_t wrapOffset = startByteInside - m_startY;
            countWrapBefore = Wrap::find(v, wrapOffset);
        }
    }
}

int64_t ByteView::getMinimum() {
    return m_byteAccess->firstByte();
}

int64_t ByteView::getMaximum() {
    return m_byteAccess->byteCount();
}

int64_t ByteView::getWindowedMinimum() {
    if (indexView.empty())
        return getMinimum();
    else
        return viewDeque->getMinimum();
}

int64_t ByteView::getWindowedMaximum() {
    if (indexView.empty())
        return getMinimum();
    else
        return viewDeque->getMaximum();
}

ByteView *ByteView::clone() {
    auto newObj = new ByteView(m_byteAccess);
    cloneFields(newObj);
    return newObj;
}

void ByteView::cloneFields(ByteView *other) {
    AbstractView::cloneFields(other);
    other->m_byteAccess = m_byteAccess;
}

FilePosition ByteView::filePosition(int row, int col) {
    FilePosition result;
    LinePointers lptrs = getLinePointers(row);
    UTF utf;
    int64_t actual;
    result.bytePosition = m_byteAccess->pointerToOffset(
            utf.forwardNcodes(lptrs.wrapPosition, col, lptrs.lineEnd, actual));
    return result;
}

std::pair<int, int> ByteView::locatePosition(FilePosition filePosition, bool preferAfter) {
    return std::pair<int, int>();
}

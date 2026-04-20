//
// Created by andrzej on 8/26/23.
//
#include "ByteView.h"
#include "ByteDeque.h"
#include <unicode/utf8.h>
#include <memory>
#include <cmath>
#include <iostream>
#include <algorithm>

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
    result.interpretation = 1;
    if (row < 0) {
        result.bytePosition = 0;
        return result;
    }
    LinePointers lptrs = getLinePointers(row);
    int32_t length = (int32_t)(lptrs.wrapEnd - lptrs.wrapPosition);
    int32_t i = 0;
    for (int n = 0; n < col && i < length; n++)
        U8_FWD_1(reinterpret_cast<const uint8_t*>(lptrs.wrapPosition), i, length);
    result.bytePosition = m_byteAccess->pointerToOffset(lptrs.wrapPosition + i);
    return result;
}

std::pair<int, int> ByteView::locatePosition(FilePosition filePosition, bool preferAfter) {
    if (indexView.empty()) return {0, 0};
    int wholeRowNumber = viewDeque->locateRow(filePosition);
    if (wholeRowNumber < 0)
        return {-1, 0};
    if (wholeRowNumber >= viewDeque->size())
        return {screenLineCount(), 0};
    auto deqLine = viewDeque->lineAt(wholeRowNumber);
    if (indexView[0].index > wholeRowNumber)
        return {-1, 0};
    int i = 0;
    while (i < indexView.size() && indexView[i].index < wholeRowNumber) {
        i++;
    }
    if (i == indexView.size())
        return {screenLineCount(), 0};
    int64_t offset = filePosition.bytePosition - m_byteAccess->pointerToOffset(deqLine.cbegin());
    if (indexView[i].wrapOffset > offset)
        return {-1, 0};
    pair<int, int> p;
    int j = i;
    p.first = -1;
    while (j < indexView.size() && indexView[j].index == wholeRowNumber) {
        int64_t ivoffset = indexView[j].wrapOffset;
        if (ivoffset == offset) {
            if (preferAfter && j > i)
                p.first = j - 1;
            else
                p.first = j;
            break;
        }
        if (ivoffset > offset) {
            p.first = j - 1;
            break;
        }
        j++;
    }
    if (p.first == -1)
        p.first = j - 1;
    const char *a = deqLine.cbegin() + indexView[p.first].wrapOffset;
    const char *b = m_byteAccess->ofsetToPointer(filePosition.bytePosition);
    int32_t length = (int32_t)(b - a);
    int32_t ai = 0;
    int64_t count = 0;
    while (ai < length) {
        U8_FWD_1(reinterpret_cast<const uint8_t*>(a), ai, length);
        count++;
    }
    p.second = count;
    return p;
}

FilePosition ByteView::lineStartPosition(int row) {
    FilePosition result;
    result.interpretation = 1;
    if (row < 0) {
        result.bytePosition = 0;
        return result;
    }
    auto iv = indexView[row];
    auto deqLine = viewDeque->lineAt(iv.index);
    result.bytePosition = m_byteAccess->pointerToOffset(deqLine.cbegin());
    return result;
}

FilePosition ByteView::lineEndPosition(int row) {
    FilePosition result;
    result.interpretation = 1;
    if (row < 0) {
        result.bytePosition = 0;
        return result;
    }
    auto iv = indexView[row];
    auto deqLine = viewDeque->lineAt(iv.index);
    result.bytePosition = m_byteAccess->pointerToOffset(deqLine.cend());
    return result;
}

FilePosition ByteView::filePositionAtOffset(int row, int64_t offsetInLine) {
    FilePosition result;
    result.interpretation = 1;
    if (row < 0) {
        result.bytePosition = 0;
        return result;
    }
    auto iv = indexView[row];
    auto deqLine = viewDeque->lineAt(iv.index);
    result.bytePosition = m_byteAccess->pointerToOffset(deqLine.cbegin() + offsetInLine);
    return result;
}

FilePosition ByteView::endPosition() {
    FilePosition result;
    result.interpretation = 1;
    result.bytePosition = m_byteAccess->byteCount();
    return result;
}

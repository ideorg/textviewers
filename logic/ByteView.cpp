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
    viewDeque = make_unique<ByteDeque>(m_byteAccess);
}

int64_t ByteView::beginTail() {
    auto opt = m_byteAccess->lastLine();
    if (!opt)
        return m_byteAccess->firstByte();
    auto lp = opt.value();
    for (int i=0; i<screenLineCount()-1; i++) {
        auto opt = m_byteAccess->lineBefore(lp);
        if (!opt)
            break;
        lp = opt.value();
    }
    return lp.offset;
}

void ByteView::gotoProportional(double relativePos) {
    auto lp =  m_byteAccess->lineEnclosing(ceill((long double)beginTail()*relativePos));
    m_startY = lp.offset;
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

//
// Created by andrzej on 8/26/23.
//

#include "ByteView.h"
#include "DString.h"
#include "ByteDeque.h"
#include <memory>
#include <cmath>

using namespace vl;
using namespace std;

ByteView::ByteView(IByteAccess *byteAccess) : m_byteAccess(byteAccess) {
    viewDeque = make_unique<ByteDeque>(m_byteAccess);
}

void ByteView::gotoProportional(double relativePos) {
    relativePos = max(min(relativePos, 1.0), 0.0);
    int64_t position = floorl((long double) relativePos * (m_byteAccess->byteCount() - 1));
    auto backf = relativePos * (m_screenLineCount - 1);
    int backCount = ceil(backf);
    backNLines(position, backCount);
}

void ByteView::backNLines(int64_t position, int backCount) {
    auto lp = m_byteAccess->lineEnclosing(position);
    for (int i=0; i<backCount; i++) {
        auto opt = m_byteAccess->lineBefore(lp);
        if (!opt)
            break;
        lp = opt.value();
    }
}

int64_t ByteView::getMaximum() {
    return m_byteAccess->byteCount();
}

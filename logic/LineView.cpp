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
    viewDeque = make_unique<LineDeque>(m_lineAccess);
}

void LineView::gotoProportional(double relativePos) {
    if (m_lineAccess->lineCount() == 0)
        return;
    if (m_lineAccess->lineCount() <= m_screenLineCount) {
        m_start = 1;
    } else {
        m_start = floor((m_lineAccess->lineCount()  - m_screenLineCount) * relativePos);
    }
}

int64_t LineView::getMaximum() {
    return m_lineAccess->lineCount();
}

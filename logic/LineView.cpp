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
    relativePos = max(min(relativePos,1.0),0.0);
    int position = floor(relativePos*(m_lineAccess->lineCount() - 1));
    auto backf = relativePos * (m_screenLineCount - 1);
    int backCount = ceill(backf);
    backNLines(position, backCount);
}

void LineView::backNLines(int position, int backCount) {
    m_start = max(position-backCount,0);
}

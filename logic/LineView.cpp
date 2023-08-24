//
// Created by andrzej on 8/23/23.
//

#include "LineView.h"
#include "LineDeque.h"
#include <algorithm>
#include <cmath>
#include <iostream>

using namespace std;
using namespace vl;

void LineView::gotoProportional(double relativePos) {
    Wrap wrap;
    relativePos = max(min(relativePos,1.0),0.0);
    int position = floor(relativePos*(m_lineAccess->lineCount() - 1));
    auto backf = relativePos * (m_screenLineCount - 1);
    int backCount = ceill(backf);
    backNLines(position, backCount);
}

LineView::LineView(ILineAccess *lineAccess): m_lineAccess(lineAccess) {
    viewDeque = make_unique<LineDeque>(m_lineAccess);
}

void LineView::backNLines(int position, int backCount) {
    m_start = max(position-backCount,0);
}

string LineView::at(int n) {
    auto iv = indexView[n];
    auto lineView = m_lineAccess->line(iv.index);
    if (lineView)
        return string(*lineView);
    else return "";
}

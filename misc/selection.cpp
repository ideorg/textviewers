//
// Created by andrzej on 10/4/22.
//

#include "selection.h"

bool Selection::charSelected(std::pair<int, int> point, vl::AbstractView *vr) {
    vl::FilePosition pos = vr->filePosition(point.first, point.second);
    return pos.between(selBegin, selEnd);
}

void Selection::compute(vl::AbstractView *vr) {
    if (secondPos.ge(firstPos)) {
        selBegin = firstPos;
        selEnd = secondPos;
    } else {
        selBegin = secondPos;
        selEnd = firstPos;
    }
    selBeginScreen = vr->locatePosition(selBegin, false);
    selEndScreen = vr->locatePosition(selEnd, false);
}

int Selection::selColBeg(int row, vl::AbstractView *vr) {
    if (row >= selBeginScreen.first && row <= selEndScreen.first) {
        if (row == selBeginScreen.first)
            return selBeginScreen.second;
        else
            return 0;
    } else
        return -1;
}

int Selection::selColEnd(int row, vl::AbstractView *vr) {
    if (row >= selBeginScreen.first && row <= selEndScreen.first) {
        if (row == selEndScreen.first)
            return selEndScreen.second;
        else
            return -1;
    } else
        return -1;
}

void Selection::setFirst(std::pair<int, int> pos, vl::AbstractView *vr) {
    firstPos = vr->filePosition(pos.first, pos.second);
    secondPos = firstPos;
    compute(vr);
}

void Selection::setSecond(std::pair<int, int> pos, vl::AbstractView *vr) {
    secondPos = vr->filePosition(pos.first, pos.second);
    compute(vr);
}

QByteArray Selection::get() {
    if (selEnd.gt(selBegin))
        return {};//return QByteArray(doc->addr + selBegin, selEnd - selBegin);
    else
        return {};
}

//
// Created by andrzej on 10/4/22.
//

#include "selection.h"
#include "logic/IByteAccess.h"

bool Selection::charSelected(std::pair<int, int> point, vl::AbstractView *view) {
    vl::FilePosition pos = view->filePosition(point.first, point.second);
    return pos.between(selBegin, selEnd);
}

void Selection::compute(vl::AbstractView *view) {
    if (secondPos.ge(firstPos)) {
        selBegin = firstPos;
        selEnd = secondPos;
    } else {
        selBegin = secondPos;
        selEnd = firstPos;
    }
    selBeginScreen = view->locatePosition(selBegin, false);
    selEndScreen = view->locatePosition(selEnd, false);
}

int Selection::selColBeg(int row, vl::AbstractView *view) {
    if (row >= selBeginScreen.first && row <= selEndScreen.first) {
        if (row == selBeginScreen.first)
            return selBeginScreen.second;
        else
            return 0;
    } else
        return -1;
}

int Selection::selColEnd(int row, vl::AbstractView *view) {
    if (row >= selBeginScreen.first && row <= selEndScreen.first) {
        if (row == selEndScreen.first)
            return selEndScreen.second;
        else
            return -1;
    } else
        return -1;
}

void Selection::setFirst(std::pair<int, int> pos, vl::AbstractView *view) {
    firstPos = view->filePosition(pos.first, pos.second);
    secondPos = firstPos;
    compute(view);
}

void Selection::setSecond(std::pair<int, int> pos, vl::AbstractView *view) {
    secondPos = view->filePosition(pos.first, pos.second);
    compute(view);
}

QByteArray Selection::get() {
    if (selEnd.gt(selBegin))
        return {};//return QByteArray(doc->addr + selBegin, selEnd - selBegin);
    else
        return {};
}

void Selection::setDocument(vl::IBaseAccess *doc) {
    this->m_doc = doc;
    int8_t interpretation;
    if (dynamic_cast<vl::IByteAccess*>(doc))
        interpretation = 1;
    else
        interpretation = 2;
    selBegin.interpretation = selEnd.interpretation =
            firstPos.interpretation = secondPos.interpretation = interpretation;
}

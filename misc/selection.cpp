//
// Created by andrzej on 10/4/22.
//

#include "selection.h"
#include "logic/IByteAccess.h"
#include "logic/ILineAccess.h"
#include "logic/Wrap.h"

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

void Selection::selectWord(std::pair<int, int> pos, vl::AbstractView *view) {
    auto dstr = view->at(pos.first);
    int col = pos.second;
    if (col < 0 || col >= (int)dstr.size()) return;
    uint32_t cl = vl::Wrap::codeClass(dstr[col]);
    if (cl == 0 || cl == (uint32_t)-1) return;
    int startCol = col;
    while (startCol > 0 && vl::Wrap::codeClass(dstr[startCol - 1]) == cl)
        startCol--;
    int endCol = col;
    while (endCol < (int)dstr.size() && vl::Wrap::codeClass(dstr[endCol]) == cl)
        endCol++;
    firstPos = view->filePosition(pos.first, startCol);
    secondPos = view->filePosition(pos.first, endCol);
    compute(view);
}

void Selection::selectLogicalLine(int row, vl::AbstractView *view) {
    firstPos = view->lineStartPosition(row);
    secondPos = view->lineEndPosition(row);
    compute(view);
}

QByteArray Selection::get() {
    if (selEnd.gt(selBegin)) {
        auto sv = m_doc->getBytes(selBegin, selEnd);
        return QByteArray(sv.cbegin(), sv.size());
    } else
        return {};
}

void Selection::setDocument(vl::IBaseAccess *doc) {
    this->m_doc = doc;
    int8_t interpretation;
    if (dynamic_cast<vl::IByteAccess*>(doc))
        interpretation = 1;
    else
        interpretation = 2;
    selBegin = selEnd = firstPos = secondPos = vl::FilePosition{};
    selBegin.interpretation = selEnd.interpretation =
            firstPos.interpretation = secondPos.interpretation = interpretation;
}

void Selection::selectAll(vl::AbstractView *view) {
    firstPos = vl::FilePosition{};
    firstPos.interpretation = selBegin.interpretation;
    secondPos = view->endPosition();
    compute(view);
}

int64_t Selection::selectionSize() {
    if (selEnd.gt(selBegin)) {
        if (selBegin.interpretation == 1) {
            return selEnd.bytePosition - selBegin.bytePosition;
        } else {
            auto sv = m_doc->getBytes(selBegin, selEnd);
            return sv.size();
        }
    }
    return 0;
}

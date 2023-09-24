//
// Created by andrzej on 8/26/23.
//

#include <cassert>
#include "ByteDeque.h"

using namespace vl;

ByteDeque::ByteDeque(IByteAccess *byteAccess) : m_byteAccess(byteAccess)  {
}

bool ByteDeque::empty() {
    return deq.empty();
}

bool ByteDeque::frontAtStart() {
    return m_byteAccess->isFirstInFile(deq.front().linePoints);
}

bool ByteDeque::pushFront(Wrap *wrap) {
    BDequeElem elem;
    if (deq.empty())
        elem.linePoints = m_byteAccess->lineEnclosing(m_startByte);
    else {
        auto opt = m_byteAccess->lineBefore(deq.front().linePoints);
        if (!opt)
            return false;
        elem.linePoints = opt.value();
    }
    elem.wrapEnds = wrap->wrapEnds(m_byteAccess->line(elem.linePoints));
    deq.push_front(elem);
    return true;
}

void ByteDeque::popFront() {
    assert(!deq.empty());
    deq.pop_front();
}

int ByteDeque::frontWrapCount() {
    return (int) deq.front().wrapEnds.size();
}

bool ByteDeque::backAtEnd() {
    return m_byteAccess->isLastInFile(deq.back().linePoints);
}

bool ByteDeque::pushBack(Wrap *wrap) {
    BDequeElem elem;
    if (deq.empty())
        elem.linePoints = m_byteAccess->lineEnclosing(m_startByte);
    else {
        auto opt = m_byteAccess->lineAfter(deq.back().linePoints);
        if (opt)
            elem.linePoints = opt.value();
        else
            return false;
    }
    elem.wrapEnds = wrap->wrapEnds(m_byteAccess->line(elem.linePoints));
    deq.push_back(elem);
    return true;
}

void ByteDeque::popBack() {
    assert(!deq.empty());
    deq.pop_back();
}

int ByteDeque::backWrapCount() {
    return (int) deq.back().wrapEnds.size();
}

void ByteDeque::setFront(int64_t start) {
    m_startByte = start;
    assert(deq.empty());
}

void ByteDeque::clear() {
    deq.clear();
}

int ByteDeque::size() {
    return (int)deq.size();
}

int64_t ByteDeque::wrapOffset(int n, int i) {
    if (i==0)
        return 0;
    else
        return deq[n].wrapEnds[i-1];
}

int ByteDeque::wrapLen(int n, int i) {
    return deq[n].wrapEnds[i] - wrapOffset(n, i);
}

int64_t ByteDeque::backWrapOffset(int i) {
    return (int) deq.back().wrapEnds[i];
}

int ByteDeque::wrapCount(int n) {
    return deq[n].wrapEnds.size();
}

std::string_view ByteDeque::lineAt(int n) {
    return m_byteAccess->line(deq[n].linePoints);
}

int64_t ByteDeque::getMinimum() {
    auto lp = deq.front().linePoints;
    return lp.offset;
}

int64_t ByteDeque::getWrapMinimum(int wrapIndex) {
    auto elem = deq.front();
    if (wrapIndex > 0)
        return elem.linePoints.offset + elem.wrapEnds[wrapIndex - 1];
    else
        return elem.linePoints.offset;
}

int64_t ByteDeque::getMaximum() {
    auto lp = deq.back().linePoints;
    return lp.offset + lp.fullLen;
}

int64_t ByteDeque::getWrapMaximum(int wrapIndex) {
    auto elem = deq.front();
    return elem.linePoints.offset + elem.wrapEnds[wrapIndex];
}

IDeque *ByteDeque::clone() {
    auto newObj = new ByteDeque(m_byteAccess);
    newObj->m_startByte = m_startByte;
    newObj->deq = deq;
    return newObj;
}

int ByteDeque::locateRow(FilePosition position) {
    if (deq.empty())
        return -1;
    if (deq.front().linePoints.offset > position.bytePosition)
        return -1;
    if (deq.back().linePoints.offset + deq.back().linePoints.len < position.bytePosition)
        return size();
    int n = 0;
    while (n<deq.size()) {
        auto &lp = deq[n].linePoints;
        if (lp.offset == position.bytePosition)
            return n;
        if (lp.offset > position.bytePosition)
            return n-1;
        n++;
    }
    return n-1;
}

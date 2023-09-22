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

void ByteDeque::pushFront(Wrap *wrap) {
    BDequeElem elem;
    if (deq.empty())
        elem.linePoints = m_byteAccess->lineEnclosing(m_startByte);
    else
        elem.linePoints = m_byteAccess->lineBefore(deq.front().linePoints).value();
    elem.wrapEnds = wrap->wrapEnds(m_byteAccess->line(elem.linePoints));
    deq.push_front(elem);
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

void ByteDeque::pushBack(Wrap *wrap) {
    BDequeElem elem;
    if (deq.empty())
        elem.linePoints = m_byteAccess->lineEnclosing(m_startByte);
    else
        elem.linePoints = m_byteAccess->lineAfter(deq.back().linePoints).value();
    elem.wrapEnds = wrap->wrapEnds(m_byteAccess->line(elem.linePoints));
    deq.push_back(elem);
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

int64_t ByteDeque::backWrapOffset(int i) {
    return (int) deq.back().wrapEnds[i];
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

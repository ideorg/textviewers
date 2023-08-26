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
    return m_byteAccess->isFirstInFile(deq[0].linePoints);
}

void ByteDeque::pushFront(const Wrap &wrap) {
    BDequeElem elem;
    if (deq.empty())
        elem.linePoints = m_byteAccess->lineEnclosing(m_startByte);
    else
        elem.linePoints = m_byteAccess->lineBefore(deq[0].linePoints).value();
    elem.wrapEnds = wrap.wrapEnds(m_byteAccess->line(elem.linePoints));
    deq.push_front(elem);
}

void ByteDeque::popFront() {
    assert(!deq.empty());
    deq.pop_front();
}

int ByteDeque::frontWrapCount() {
    return (int) deq[0].wrapEnds.size();
}

bool ByteDeque::backAtEnd() {
    return m_byteAccess->isLastInFile(deq.back().linePoints);
}

void ByteDeque::pushBack(const Wrap &wrap) {
    BDequeElem elem;
    if (deq.empty())
        elem.linePoints = m_byteAccess->lineEnclosing(m_startByte);
    else
        elem.linePoints = m_byteAccess->lineAfter(deq.back().linePoints).value();
    elem.wrapEnds = wrap.wrapEnds(m_byteAccess->line(elem.linePoints));
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

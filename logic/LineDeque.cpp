//
// Created by andrzej on 8/24/23.
//

#include <cassert>
#include "LineDeque.h"

using namespace vl;

LineDeque::LineDeque(ILineAccess *lineAccess) : m_lineAccess(lineAccess) {
}

bool LineDeque::empty() {
    return deq.empty();
}

bool LineDeque::frontAtStart() {
    return deq[0].index == 0;
}

void LineDeque::pushFront(const Wrap &wrap) {
    LDequeElem elem;
    if (deq.empty())
        elem.index = m_startLine;
    else
        elem.index = deq[0].index - 1;
    elem.wrapEnds = wrap.wrapEnds(m_lineAccess->lineByIndex(elem.index).value());
    deq.push_front(elem);
}

void LineDeque::popFront() {
    assert(!deq.empty());
    deq.pop_front();
}

int LineDeque::frontWrapCount() {
    return (int) deq[0].wrapEnds.size();
}

bool LineDeque::backAtEnd() {
    int lastIndex = deq.back().index;
    return m_lineAccess->isLastInFile(lastIndex);
}

void LineDeque::pushBack(const Wrap &wrap) {
    LDequeElem elem;
    if (deq.empty())
        elem.index = m_startLine;
    else
        elem.index = deq.back().index + 1;
    elem.wrapEnds = wrap.wrapEnds(m_lineAccess->lineByIndex(elem.index).value());
    deq.push_back(elem);
}

void LineDeque::popBack() {
    assert(!deq.empty());
    deq.pop_back();
}

int LineDeque::backWrapCount() {
    return (int) deq.back().wrapEnds.size();
}

void LineDeque::setFront(int64_t start) {
    m_startLine = (int)start;
    assert(deq.empty());
}

void LineDeque::clear() {
    deq.clear();
}

int LineDeque::size() {
    return (int)deq.size();
}

int64_t LineDeque::backWrapOffset(int i) {
    return (int) deq.back().wrapEnds[i];
}

std::string_view LineDeque::lineAt(int n) {
    return m_lineAccess->lineByIndex(deq[n].index).value();
}

int64_t LineDeque::getMinimum() {
    return deq.front().index;
}

int64_t LineDeque::getWrapMinimum(int wrapIndex) {
    return getMinimum();
}

int64_t LineDeque::getMaximum() {
    return deq.back().index + 1;
}

int64_t LineDeque::getWrapMaximum(int wrapIndex) {
    return getMaximum();
}

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

bool LineDeque::pushFront(Wrap *wrap) {
    LDequeElem elem;
    if (deq.empty())
        elem.index = m_startLine;
    else
        elem.index = deq[0].index - 1;
    auto opt = m_lineAccess->lineByIndex(elem.index);
    if (opt)
        return false;
    elem.wrapEnds = wrap->wrapEnds(opt.value());
    deq.push_front(elem);
    return true;
}

void LineDeque::popFront() {
    assert(!deq.empty());
    deq.pop_front();
}

int LineDeque::frontWrapCount() {
    return (int) deq.front().wrapEnds.size();
}

bool LineDeque::backAtEnd() {
    int lastIndex = deq.back().index;
    return m_lineAccess->isLastInFile(lastIndex);
}

bool LineDeque::pushBack(Wrap *wrap) {
    LDequeElem elem;
    if (deq.empty())
        elem.index = m_startLine;
    else
        elem.index = deq.back().index + 1;
    auto opt = m_lineAccess->lineByIndex(elem.index);
    if (opt)
        elem.wrapEnds = wrap->wrapEnds(opt.value());
    else
        return false;
    deq.push_back(elem);
    return true;
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

int64_t LineDeque::getFront() {
    return m_startLine;
}

void LineDeque::clear() {
    deq.clear();
}

int LineDeque::size() {
    return (int)deq.size();
}

int64_t LineDeque::wrapOffset(int n, int i) {
    if (i==0)
        return 0;
    else
        return deq[n].wrapEnds[i-1];
}

int LineDeque::wrapLen(int n, int i) {
    return deq[n].wrapEnds[i] - wrapOffset(n, i);
}

int64_t LineDeque::backWrapOffset(int i) {
    return (int) deq.back().wrapEnds[i];
}

int LineDeque::wrapCount(int n) {
    return deq[n].wrapEnds.size();
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

IDeque *LineDeque::clone() {
    auto newObj = new LineDeque(m_lineAccess);
    newObj->m_startLine = m_startLine;
    newObj->deq = deq;
    return newObj;
}

int LineDeque::locateRow(FilePosition position) {
    if (deq.empty())
        return -1;
    if (deq.front().index > position.lineNumber)
        return -1;
    if (deq.back().index < position.lineNumber)
        return size();
    assert(deq.front().index == m_startLine);
    return position.lineNumber - m_startLine;
}

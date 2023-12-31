//
// Created by andrzej on 8/26/23.
//

#ifndef VIEWERS_BYTEDEQUE_H
#define VIEWERS_BYTEDEQUE_H

#include <cstdint>
#include "IDeque.h"
#include "IByteAccess.h"
#include <deque>

namespace vl {

struct BDequeElem {
    LinePoints linePoints;
    std::vector<int64_t> wrapEnds;
};

class ByteDeque : public IDeque {
    std::deque<BDequeElem> deq;
    IByteAccess* m_byteAccess;
    int64_t m_startByte = 0;
public:
    explicit ByteDeque(IByteAccess* byteAccess);
    IDeque *clone() override;
    bool empty() override;
    int64_t getMinimum() override;
    int64_t getWrapMinimum(int wrapIndex) override;
    int64_t getMaximum() override;
    int64_t getWrapMaximum(int wrapIndex) override;
    bool frontAtStart() override;
    bool pushFront(Wrap *wrap) override;
    void popFront() override;
    int frontWrapCount() override;
    bool backAtEnd() override;
    bool pushBack(Wrap *wrap) override;
    void popBack() override;
    int backWrapCount() override;
    void setFront(int64_t start) override;
    int64_t getFront() override;
    void clear() override;
    int size() override;
    int64_t wrapOffset(int n, int i) override;
    int wrapLen(int n, int i) override;
    int64_t backWrapOffset(int i) override;
    std::string_view lineAt(int n) override;
    int locateRow(FilePosition position) override;
    int wrapCount(int n) override;
};
}

#endif //VIEWERS_BYTEDEQUE_H

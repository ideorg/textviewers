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
    std::vector<int> wrapEnds;
};

class ByteDeque : public IDeque {
    std::deque<BDequeElem> deq;
    IByteAccess* m_byteAccess;
    int64_t m_startByte = 0;
public:
    explicit ByteDeque(IByteAccess* byteAccess);
    bool empty() override;
    bool frontAtStart() override;
    void pushFront(const Wrap &wrap) override;
    void popFront() override;
    int frontWrapCount() override;
    bool backAtEnd() override;
    void pushBack(const Wrap &wrap) override;
    void popBack() override;
    int backWrapCount() override;
    void setFront(int64_t start) override;
    void clear() override;
    int size() override;
    int64_t backWrapOffset(int i) override;
    std::string_view lineAt(int n) override;
};
}

#endif //VIEWERS_BYTEDEQUE_H

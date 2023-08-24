//
// Created by andrzej on 8/24/23.
//

#ifndef VIEWER_LINEDEQUE_H
#define VIEWER_LINEDEQUE_H

#include <cstdint>
#include "IDeque.h"
#include "ILineAccess.h"
#include <deque>
#include <vector>

namespace vl {

struct LDequeElem {
    int index;
    std::vector<int> wrapOffsets;
};

class LineDeque : public IDeque {
    std::deque<LDequeElem> deq;
    ILineAccess* m_lineAccess;
    int m_startLine = 0;
public:
    explicit LineDeque(ILineAccess* lineAccess);
    bool empty() override;
    bool frontAtStart() override;
    void pushFront() override;
    void popFront() override;
    int frontWrapCount() override;
    bool backAtEnd() override;
    void pushBack() override;
    void popBack() override;
    int backWrapCount() override;
    void setFront(int64_t start) override;
    void clear() override;
    int size() override;
    int64_t backWrapOffset(int i) override;
};
}

#endif //VIEWER_LINEDEQUE_H

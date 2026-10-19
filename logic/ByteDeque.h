//
// Created by andrzej on 8/26/23.
//

#ifndef VIEWERS_BYTEDEQUE_H
#define VIEWERS_BYTEDEQUE_H

#include <cstdint>
#include "IDeque.h"
#include "IByteAccess.h"
#include <deque>
#include <map>
#include <optional>

namespace vl {

struct BDequeElem {
    LinePoints linePoints;
    std::vector<int64_t> wrapEnds;
};

// Collapsed byte ranges, begin -> end. A line starting strictly inside
// (begin, end) is hidden: the deque skips it, so the screen fills with the
// lines after the range.
using HiddenRanges = std::map<int64_t, int64_t>;

class ByteDeque : public IDeque {
    std::deque<BDequeElem> deq;
    IByteAccess* m_byteAccess;
    int64_t m_startByte = 0;
    const HiddenRanges *m_hidden = nullptr;
    std::optional<std::pair<int64_t, int64_t>> covering(int64_t lineStart);
    LinePoints visibleEnclosing(int64_t position);
    std::optional<LinePoints> visibleAfter(const LinePoints &lp);
    std::optional<LinePoints> visibleBefore(const LinePoints &lp);
public:
    explicit ByteDeque(IByteAccess* byteAccess);
    void setHiddenRanges(const HiddenRanges *hidden) { m_hidden = hidden; }
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

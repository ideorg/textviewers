//
// Created by andrzej on 8/23/23.
//

#ifndef VIEWER_IDEQUE_H
#define VIEWER_IDEQUE_H

#include <vector>
#include <string_view>
#include <cassert>
#include "Wrap.h"

namespace vl {

union FilePosition {
    struct {
        int64_t bytePosition = 0;
        int8_t interpretation = 0; //problem with endianness
    };
    struct {
        int lineNumber;
        int offset;
    };

    bool ge(FilePosition other) const {
        assert(interpretation > 0);
        int64_t a, b, c;
        if (interpretation == 1) {
            a = bytePosition;
            b = other.bytePosition;
        } else {
            a = ((int64_t) lineNumber << 32) + offset;
            b = ((int64_t) other.lineNumber << 32) + other.offset;
        }
        return a >= b;
    }

    bool gt(FilePosition other) const {
        assert(interpretation > 0);
        int64_t a, b, c;
        if (interpretation == 1) {
            a = bytePosition;
            b = other.bytePosition;
        } else {
            a = ((int64_t) lineNumber << 32) + offset;
            b = ((int64_t) other.lineNumber << 32) + other.offset;
        }
        return a > b;
    }

    bool between(FilePosition begin, FilePosition end) const {
        assert(interpretation > 0);
        int64_t a, b, c;
        if (interpretation == 1) {
            a = bytePosition;
            b = begin.bytePosition;
            c = end.bytePosition;
        } else {
            a = ((int64_t) lineNumber << 32) + offset;
            b = ((int64_t) begin.lineNumber << 32) + begin.offset;
            c = ((int64_t) end.lineNumber << 32) + end.offset;
        }
        return a >= b && a < c;
    }
};

class IDeque {
public:
    virtual ~IDeque() = default;
    virtual IDeque *clone() = 0;
    virtual bool empty() = 0;
    virtual int64_t getMinimum() = 0;
    virtual int64_t getWrapMinimum(int wrapIndex) = 0;
    virtual int64_t getMaximum() = 0;
    virtual int64_t getWrapMaximum(int wrapIndex) = 0;
    virtual bool frontAtStart() = 0;
    virtual bool pushFront(Wrap *wrap) = 0;
    virtual void popFront() = 0;
    virtual int frontWrapCount() = 0;
    virtual bool backAtEnd() = 0;
    virtual bool pushBack(Wrap *wrap) = 0;
    virtual void popBack() = 0;
    virtual int backWrapCount() = 0;
    virtual void setFront(int64_t start) = 0;
    virtual int64_t getFront() = 0;
    virtual void clear() = 0;
    virtual int size() = 0;
    virtual int64_t wrapOffset(int n, int i) = 0;
    virtual int wrapLen(int n, int i) = 0;
    virtual int64_t backWrapOffset(int i) = 0;
    virtual std::string_view lineAt(int n) = 0;
    virtual int wrapCount(int n) = 0;
    virtual int locateRow(FilePosition position) = 0;
};
}
#endif //VIEWER_IDEQUE_H

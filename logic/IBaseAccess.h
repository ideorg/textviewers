//
// Created by andrzej on 8/26/23.
//
#ifndef VIEWERS_IBASEACCESS_H
#define VIEWERS_IBASEACCESS_H

#include <cassert>

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

class IBaseAccess {
public:
    virtual ~IBaseAccess()= default;
    virtual std::string_view getBytes(FilePosition from, FilePosition to) = 0;
};
}

#endif //VIEWERS_IBASEACCESS_H

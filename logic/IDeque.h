//
// Created by andrzej on 8/23/23.
//

#ifndef VIEWER_IDEQUE_H
#define VIEWER_IDEQUE_H

#include <vector>
#include <string_view>
#include "Wrap.h"

namespace vl {

class IDeque {
public:
    virtual ~IDeque() = default;
    virtual IDeque* clone() = 0;
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
    virtual void clear() = 0;
    virtual int size() = 0;
    virtual int64_t wrapOffset(int n, int i) = 0;
    virtual int wrapLen(int n, int i) = 0;
    virtual int64_t backWrapOffset(int i) = 0;
    virtual std::string_view lineAt(int n) = 0;
    virtual int wrapCount(int n) = 0;
};
}

#endif //VIEWER_IDEQUE_H

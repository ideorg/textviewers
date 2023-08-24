//
// Created by andrzej on 8/23/23.
//

#ifndef VIEWER_IDEQUE_H
#define VIEWER_IDEQUE_H

namespace vl {

class IDeque {
public:
    virtual bool empty() = 0;
    virtual bool frontAtStart() = 0;
    virtual void pushFront() = 0;
    virtual void popFront() = 0;
    virtual int frontWrapCount() = 0;
    virtual bool backAtEnd() = 0;
    virtual void pushBack() = 0;
    virtual void popBack() = 0;
    virtual int backWrapCount() = 0;
    virtual void setFront(int64_t start) = 0;
    virtual void clear() = 0;
    virtual int size() = 0;
    virtual int64_t backWrapOffset(int i) = 0;
};
}

#endif //VIEWER_IDEQUE_H

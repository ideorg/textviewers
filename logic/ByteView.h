//
// Created by andrzej on 8/26/23.
//

#ifndef VIEWERS_BYTEVIEW_H
#define VIEWERS_BYTEVIEW_H

#include "AbstractView.h"
#include "IByteAccess.h"

namespace vl {

class ByteView : public AbstractView {
    IByteAccess *m_byteAccess;
    int64_t beginTail();
protected:
    void cloneFields(ByteView *other);
public:
    explicit ByteView(IByteAccess *byteAccess);
    void gotoProportional(double relativePos) override;
    int64_t getMinimum() override;
    int64_t getMaximum() override;
    int64_t getWindowedMinimum() override;
    int64_t getWindowedMaximum() override;
    ByteView *clone();
    FilePosition filePosition(int row, int col) override;
    std::pair<int, int> locatePosition(FilePosition filePosition, bool preferAfter = false) override;
};
}

#endif //VIEWERS_BYTEVIEW_H

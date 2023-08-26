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
    void backNLines(int64_t position, int backCount);
public:
    explicit ByteView(IByteAccess *byteAccess);
    void gotoProportional(double relativePos) override;
    int64_t getMaximum() override;
};
}

#endif //VIEWERS_BYTEVIEW_H

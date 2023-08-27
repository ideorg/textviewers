//
// Created by andrzej on 8/23/23.
//

#ifndef VIEWER_LINEVIEW_H
#define VIEWER_LINEVIEW_H

#include "AbstractView.h"
#include "ILineAccess.h"

namespace vl {

class LineView : public AbstractView {
    ILineAccess *m_lineAccess;
public:
    explicit LineView(ILineAccess *lineAccess);
    void gotoProportional(double relativePos) override;
    int64_t getMinimum() override;
    int64_t getMaximum() override;
    int64_t getWindowedMinimum() override;
    int64_t getWindowedMaximum() override;
};
}

#endif //VIEWER_LINEVIEW_H

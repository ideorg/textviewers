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
    double beginTail();
public:
    explicit LineView(ILineAccess *lineAccess);
    void gotoProportional(double relativePos) override;
    int64_t getMinimum() override;
    int64_t getMaximum() override;
    int64_t getWindowedMinimum() override;
    int64_t getWindowedMaximum() override;
    FilePosition filePosition(int row, int col) override;
    std::pair<int, int> locatePosition(FilePosition filePosition, bool preferAfter = false) override;
};
}

#endif //VIEWER_LINEVIEW_H

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
    void backNLines(int position, int backCount);
    int m_firstLine = 0;
public:
    LineView(ILineAccess *lineAccess);
    void gotoProportional(double relativePos) override;
};
}

#endif //VIEWER_LINEVIEW_H
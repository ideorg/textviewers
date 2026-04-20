#include "TextViewer.h"

#include <QPainter>
#include <QKeyEvent>
#include <QFontDatabase>
#include <QHBoxLayout>
#include <QProxyStyle>
#include <QDebug>

namespace {

// QProxyStyle that flips SH_ScrollBar_LeftClickAbsolutePosition on so a
// left click on the scrollbar track jumps the slider to the click point
// (centered on the cursor) instead of paging toward it one screen at a time.
// Other style hints fall through to the application style.
class JumpToClickStyle : public QProxyStyle {
public:
    using QProxyStyle::QProxyStyle;
    int styleHint(StyleHint hint, const QStyleOption *option = nullptr,
                  const QWidget *widget = nullptr,
                  QStyleHintReturn *returnData = nullptr) const override {
        if (hint == QStyle::SH_ScrollBar_LeftClickAbsolutePosition)
            return 1;
        return QProxyStyle::styleHint(hint, option, widget, returnData);
    }
};

}


namespace wid {
TextViewer::TextViewer(const char *addr, int64_t fileSize, QWidget *parent) :
        QWidget(parent) {
    setWindowTitle(tr("Viewer"));
    paintArea = new PaintArea(addr, fileSize, this);
    paintArea->setCursor(Qt::IBeamCursor);
    auto *vLayout = new QVBoxLayout;
    auto *hLayout = new QHBoxLayout;
    hscroll = new QScrollBar(this);
    hscroll->setOrientation(Qt::Horizontal);
    vscroll = new QScrollBar(this);
    auto *jumpStyle = new JumpToClickStyle(hscroll->style());
    jumpStyle->setParent(this);
    hscroll->setStyle(jumpStyle);
    vscroll->setStyle(jumpStyle);
    hLayout->addWidget(paintArea);
    hLayout->addWidget(vscroll);
    vLayout->addLayout(hLayout);
    vLayout->addWidget(hscroll);
    setLayout(vLayout);
    connect(hscroll, &QScrollBar::valueChanged, this, &TextViewer::hscrollChanged);
    connect(vscroll, &QScrollBar::valueChanged, this, &TextViewer::vscrollChanged);
    connect(paintArea, &PaintArea::sizeChanged, this, &TextViewer::sizeChanged);
    connect(paintArea, &PaintArea::scrollHChanged, this, &TextViewer::setHScrollChange);
    connect(paintArea, &PaintArea::scrollVChanged, this, &TextViewer::setVScrollChange);
    setFocusProxy(paintArea);
}

void TextViewer::setData(const char *addr, int64_t fileSize) {
    paintArea->setData(addr, fileSize);
}

void TextViewer::hscrollChanged() {
    paintArea->setHorizontal(hscroll->value());
}


void TextViewer::setWrapMode(int mode) {
    paintArea->setWrapMode(mode);
    hscroll->setVisible(paintArea->tv->wrapMode()==0);
}

void TextViewer::setKind(int kind) {
    paintArea->setKind(kind);
}

vl::IByteAccess *TextViewer::byteAccess() {
    return paintArea->byteAccess();
}

void TextViewer::showMatch(int64_t offset, int64_t length) {
    paintArea->showMatch(offset, length);
}

void TextViewer::setFontSize(qreal pt) {
    paintArea->setFontSize(pt);
}

qreal TextViewer::fontSize() const {
    return paintArea->fontSize();
}

void TextViewer::setMaxTabW(int w) {
    paintArea->setMaxTabW(w);
}

int TextViewer::maxTabW() const {
    return paintArea->maxTabW();
}

void TextViewer::setHighlighter(vl::IHighlighter *h, HighlightColors colors) {
    paintArea->setHighlighter(h, std::move(colors));
}

void TextViewer::vscrollChanged() {
    long double relative;
    if (vscroll->maximum() > 0)
        relative = (long double) (vscroll->value()) / vscroll->maximum();
    else
        relative = 0;
    paintArea->setVertical(relative);
}

void TextViewer::sizeChanged() {
    double factor;
    int64_t range64 = paintArea->tv->getScrollRange();
    if (range64 < MAXVSCROLL) {
        vscroll->setMaximum((int) range64);
        factor = 1;
    } else {
        vscroll->setMaximum(MAXVSCROLL);
        factor = (double) range64 / MAXVSCROLL;
    }
    vscroll->setPageStep(paintArea->tv->getWindowedRange()/factor);
}

void TextViewer::setHScrollChange() {
    disconnect(hscroll, &QScrollBar::valueChanged, this, &TextViewer::hscrollChanged);
    hscroll->setValue(paintArea->tv->startX());
    connect(hscroll, &QScrollBar::valueChanged, this, &TextViewer::hscrollChanged);
}

void TextViewer::setVScrollChange() {
    disconnect(vscroll, &QScrollBar::valueChanged, this, &TextViewer::vscrollChanged);
    vscroll->setValue(paintArea->tv->startYproportional()*vscroll->maximum());
    connect(vscroll, &QScrollBar::valueChanged, this, &TextViewer::vscrollChanged);
}

}
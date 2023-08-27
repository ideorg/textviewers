#include "TextViewer.h"

#include <QPainter>
#include <QKeyEvent>
#include <QFontDatabase>
#include <QHBoxLayout>
#include "UTF/UTF.hpp"
#include <QDebug>

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
    hLayout->addWidget(paintArea);
    hLayout->addWidget(vscroll);
    vLayout->addLayout(hLayout);
    vLayout->addWidget(hscroll);
    setLayout(vLayout);
    connect(hscroll, &QScrollBar::valueChanged, this, &TextViewer::hscrollChanged);
    connect(vscroll, &QScrollBar::valueChanged, this, &TextViewer::vscrollChanged);
    connect(paintArea, &PaintArea::sizeChanged, this, &TextViewer::sizeChanged);
}

void TextViewer::setData(const char *addr, int64_t fileSize) {
    paintArea->setData(addr, fileSize);
}

void TextViewer::hscrollChanged() {
    paintArea->setHorizontal(hscroll->value());
}


void TextViewer::setWrapMode(int mode) {
    paintArea->setWrapMode(mode);
}

void TextViewer::setKind(int kind) {
    paintArea->setKind(kind);
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

}
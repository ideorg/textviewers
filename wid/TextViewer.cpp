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

}
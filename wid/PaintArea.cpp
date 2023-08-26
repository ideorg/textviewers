//
// Created by Andrzej Borucki on 2022-09-13
//

#include <QPainter>
#include <QFontDatabase>
#include <cmath>
#include <QElapsedTimer>
#include <QDebug>
#include <QMenu>
#include "PaintArea.h"
#include <QApplication>
#include <QClipboard>
#include <QMimeData>
#include "UTF/UTF.hpp"
#include "logic/ByteView.h"

namespace wid {

void PaintArea::paintEvent(QPaintEvent *event) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::TextAntialiasing);
    QRect R = event->rect();
    bool oneCharRepaint = R.width() == ceil(fontWidth);
    QString pilcrow = QChar(182);
    QPen pen(Qt::black);
    pen.setWidth(1);
    painter.setPen(pen);

    QString qstr = updateCaretPos();
    if (oneCharRepaint) {

    } else {
        for (int i = 0; i < tv->size(); i++) {
            drawSelBackground(painter, i);
            QRect R(0, i * fontHeight, this->rect().width(), fontHeight);
            QString qstr = QString::fromUcs4(tv->at(i).c_str(), tv->at(i).size());
            if (qstr.isEmpty() && tv->lastInFile(i)) {
                QPen pen1(Qt::gray);
                painter.setPen(pen1);
                painter.drawText(R, Qt::AlignLeft, pilcrow);
            } else
                painter.drawText(R, Qt::AlignLeft, qstr);
        }
        int y = tv->size() * fontHeight;
        painter.fillRect(0, y, QWidget::width(), QWidget::height() - y, Qt::white);
    }

    if (drawCaret && hasFocus()) {
        QPen pen(Qt::black);
        pen.setWidth(2);
        painter.setPen(pen);
        auto p = toScreenPos(caretPos, false);
        painter.drawLine(p.second, p.first + 1, p.second, p.first + fontHeight - 1);
    }
}

QString PaintArea::updateCaretPos() {
    if (caretPos.first >= tv->size()) {
        caretPos.first = tv->size() - 1;
        caretPos.second = width() / fontWidth;
    }
    UTF utf;
    auto dstr = tv->at(caretPos.first);
    QString qstr;
    if (dstr.size() <= caretPos.second) {
        caretPos.second = dstr.size();
        qstr = ' ';
    } else {
        char32_t d = dstr[caretPos.second];
        qstr = QString::fromUcs4(&d, 1);
    }
    return qstr;
}

QColor PaintArea::getSelColor() {
    QColor selColor(0xa6, 0xd2, 0xff);
    return selColor;
}

PaintArea::PaintArea(const char *addr, int64_t fileSize, QWidget *parent) : QWidget(parent) {
    QFont font = QFontDatabase::systemFont(QFontDatabase::FixedFont);
    font.setPointSizeF(10.5);
    QFontMetricsF fm(font, this);
    fontWidth = fm.horizontalAdvance("0");
    fontHeight = fm.height();
    this->setFont(font);
    doc = new vl::ByteDocument(addr, fileSize);
    tv = new vl::ByteView(doc);
    connect(&timer, &QTimer::timeout, this, &PaintArea::doBlinkMethod);
}

PaintArea::~PaintArea() {
    delete doc;
}

void PaintArea::setData(const char *addr, int64_t fileSize) {
    delete doc;
    doc = new vl::ByteDocument(addr, fileSize);
    delete tv;
    tv = new vl::ByteView(doc);
    setSize(width(), height());
    update();
}

void PaintArea::setSize(int width, int height) {
    tv->setScreenLineCount(floor((double) height / fontHeight));
    double fw = (double) width / fontWidth;
    tv->setScreenLineLen(tv->wrapMode() ? floor(fw) : ceil(fw));
    tv->fillDeque();
    tv->recalcLines();
    Q_EMIT sizeChanged();
}

void PaintArea::resizeEvent(QResizeEvent *event) {
    setSize(event->size().width(), event->size().height());
}

void PaintArea::wheelVertical(int delta) {
    update();
    Q_EMIT scrollVChanged();
}

void PaintArea::setHorizontal(int beginX) {
    update();
}

void PaintArea::setVertical(int64_t position) {
    update();
}

void PaintArea::wheelHorizontal(int delta) {
    update();
    Q_EMIT scrollVChanged();
}

void PaintArea::doBlinkMethod() {
    drawCaret = !drawCaret;
    auto p = toScreenPos(caretPos, false);
    repaint(p.second, p.first, ceil(fontWidth), ceil(fontHeight));
}

std::pair<int, int> PaintArea::toCharPos(QPoint point, bool smart) {
    int px = smart ? point.x() + (int) (fontWidth / 2) : point.x();
    int x = floor(px / fontWidth);
    int y = floor(point.y() / fontHeight);
    return std::make_pair(y, x);
}

std::pair<int, int> PaintArea::toScreenPos(std::pair<int, int> point, bool smart) {
    int x = (int) (point.second * fontWidth);
    int y = (int) (point.first * fontHeight);
    return std::make_pair(y, smart ? x - (int) (fontWidth / 2) : x);
}


void PaintArea::mousePressEvent(QMouseEvent *event) {
    QWidget::mousePressEvent(event);
    if (event->button() == Qt::LeftButton) {
        setFocus();
        trySetCaret(event->pos());
        update();
    }
}

void PaintArea::mouseReleaseEvent(QMouseEvent *event) {
    auto cp = toCharPos(event->pos(), true);
    if (charInseideArea(cp)) {
        update();
    }
}

void PaintArea::mouseMoveEvent(QMouseEvent *event) {
    auto cp = toCharPos(event->pos(), true);
    if (charInseideArea(cp)) {
        update();
    }
    QWidget::mouseMoveEvent(event);
}

#if QT_CONFIG(wheelevent)

void PaintArea::wheelEvent(QWheelEvent *event) {
    int delta = event->angleDelta().y() / 40;
    if (event->modifiers() == Qt::ShiftModifier)
        wheelHorizontal(delta);
    else
        wheelVertical(delta);
}

//set caret on char containing point or on row and end of line
void PaintArea::trySetCaret(QPoint point) {
    caretPos = toCharPos(point, true);
    timer.start(500);
    update();
}

void PaintArea::setWrapMode(int mode) {
    double fw = (double) width() / fontWidth;
    tv->setScreenLineLen(tv->wrapMode() ? floor(fw) : ceil(fw));
    update();
}

void PaintArea::contextMenuEvent(QContextMenuEvent *event) {
    QMenu menu(this);
    QAction *actionCopy = menu.addAction("Copy");
    QAction *selectedAction = menu.exec(event->globalPos());

    if (selectedAction == actionCopy) {
        QClipboard *clipboard = QGuiApplication::clipboard();
        QMimeData *mimeData = new QMimeData();
        clipboard->setMimeData(mimeData);
    }
}

//smart "<=" in second<=screenLineLen
bool PaintArea::charInseideArea(std::pair<int, int> cp) {
    return cp.first >= 0 && cp.first < tv->screenLineCount() && cp.second >= 0 && cp.second <= tv->screenLineLen();
}

void PaintArea::drawSelBackground(QPainter &painter, int row) {
    painter.fillRect(0, row * fontHeight, QWidget::width(), (row + 1) * fontHeight, Qt::white);
}

#endif
}

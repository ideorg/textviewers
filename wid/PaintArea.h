//
// Created by Andrzej Borucki on 2022-09-13
//

#ifndef VIEWER_PAINTAREA_H
#define VIEWER_PAINTAREA_H

#include "logic/LineView.h"
#include "logic/LineIndexedDocument.h"
#include "logic/ByteView.h"
#include <QWidget>
#include <QPaintEvent>
#include <QTimer>

namespace wid {
enum class LogicKind {BytePos, LineIndex};
class PaintArea : public QWidget {
Q_OBJECT
    LogicKind logicKind;
    qreal fontWidth, fontHeight;
    QTimer timer;
    bool drawCaret = true;
    std::pair<int,int> caretPos;
    void doBlinkMethod();
    static QColor getSelColor();
    QString updateCaretPos();
    std::pair<int,int> toCharPos(QPoint point, bool smart);
    std::pair<int,int> toScreenPos(std::pair<int,int> point, bool smart);
protected:
    void contextMenuEvent(QContextMenuEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
#if QT_CONFIG(wheelevent)
    void wheelEvent(QWheelEvent *event) override;
#endif
public:
    explicit PaintArea(const char *addr, int64_t fileSize, QWidget *parent);
    void setData(const char *addr, int64_t fileSize);
    void setSize(int width, int height);
    void setWrapMode(int mode);
    ~PaintArea() override;
    void wheelVertical(int delta);
    void wheelHorizontal(int delta);
    void trySetCaret(QPoint point);
    void setHorizontal(int beginX);
    void setVertical(int64_t position);
    vl::ByteView *tv = nullptr;
    vl::Interface *doc = nullptr;
    bool charInseideArea(std::pair<int,int> cp);
    void drawSelBackground(QPainter &painter, int row);
Q_SIGNALS:
    void sizeChanged();
    void scrollHChanged();
    void scrollVChanged();
};
}

#endif //VIEWER_PAINTAREA_H

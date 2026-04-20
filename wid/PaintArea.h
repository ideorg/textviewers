//
// Created by Andrzej Borucki on 2022-09-13
//

#ifndef VIEWER_PAINTAREA_H
#define VIEWER_PAINTAREA_H

#include "logic/LineView.h"
#include "logic/LineIndexedDocument.h"
#include "logic/ByteView.h"
#include "logic/IByteAccess.h"
#include "misc/selection.h"
#include <QWidget>
#include <QPaintEvent>
#include <QTimer>
#include <QElapsedTimer>
#include <optional>

namespace wid {

class PaintArea : public QWidget {
Q_OBJECT
    int logicKind = 0;
    qreal fontWidth, fontHeight;
    QTimer timer;
    QTimer autoScrollTimer;
    bool selecting = false;
    QPoint lastMousePos;
    QElapsedTimer sinceDoubleClick;
    bool drawCaret = true;
    Selection selection;
    std::pair<int,int> caretPos;
    std::optional<vl::FilePosition> m_selAnchor;
    void moveCaret(int newRow, int newCol, bool shift);
    void doBlinkMethod();
    void doAutoScroll();
    static QColor getSelColor();
    QString updateCaretPos();
    void copyToClipboard();
    std::pair<int,int> toCharPos(QPoint point, bool smart);
    std::pair<int,int> toScreenPos(std::pair<int,int> point, bool smart);
    void applyFont(qreal pt);
    qreal m_fontSize = 10.5;
    const char *m_addr;
    int64_t m_fileSize;
protected:
    void contextMenuEvent(QContextMenuEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
#if QT_CONFIG(wheelevent)
    void wheelEvent(QWheelEvent *event) override;
#endif
public:
    explicit PaintArea(const char *addr, int64_t fileSize, QWidget *parent);
    void setData(const char *addr, int64_t fileSize);
    void setSize(int width, int height);
    void setKind(int kind);
    void setWrapMode(int wrapMode);
    ~PaintArea() override;
    void wheelVertical(int delta);
    void wheelHorizontal(int delta);
    void trySetCaret(QPoint point);
    void setHorizontal(int startX);
    void setVertical(double relativePos);
    vl::AbstractView *tv = nullptr;
    vl::IBaseAccess *doc = nullptr;
    vl::IByteAccess *byteAccess();
    void showMatch(int64_t offset, int64_t length);
    void setFontSize(qreal pt);
    qreal fontSize() const { return m_fontSize; }
    void setMaxTabW(int w);
    int maxTabW() const;
    bool charInseideArea(std::pair<int,int> cp);
    void drawSelBackground(QPainter &painter, int row);
Q_SIGNALS:
    void sizeChanged();
    void scrollHChanged();
    void scrollVChanged();
};
}

#endif //VIEWER_PAINTAREA_H

//
// Created by Andrzej Borucki on 2022-09-13
//

#ifndef VIEWER_PAINTAREA_H
#define VIEWER_PAINTAREA_H

#include "logic/LineView.h"
#include "logic/LineIndexedDocument.h"
#include "logic/ByteView.h"
#include "logic/IByteAccess.h"
#include "logic/RegexSearcher.h"
#include "logic/Searcher.h"
#include "misc/selection.h"
#include <qce/IHighlighter.h>
#include <qce/IFoldingProvider.h>
#include <qce/FoldPairing.h>
#include <QWidget>
#include <set>
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
    qce::IHighlighter *m_highlighter = nullptr;
    qce::IFoldingProvider *m_folding = nullptr;
    int64_t m_preludeBytes = 64 * 1024;
    int64_t m_foldMaxBytes = 1024 * 1024;
    QVector<qce::FoldRange> m_foldCache;
    int64_t m_foldCacheBegin = 0;
    int64_t m_foldCacheEnd = 0;
    vl::HiddenRanges m_collapsed;  // collapsed folds, begin -> end byte
    void refillAfterFoldChange();
    std::vector<qreal> m_rowY;
    qreal m_rowYEnd = 0;
    int64_t m_progressiveBudget = 0;
    int64_t m_lastViewportSig = -1;
    QTimer m_settleTimer;
    bool clickOnGutter(QMouseEvent *event);
    void computeRowLayout();
    int rowForVisualY(int pixelY) const;
    void onSettleTick();
    void paintHighlighted(QPainter &painter);
    void paintPlain(QPainter &painter);
    qreal gutterPx() const { return m_folding ? 16.0 : 0.0; }
    void paintGutter(QPainter &painter, int row, bool hasFoldBegin, bool collapsed);
    // Search results are marked only in the visible rows.
    struct MatchCols { int begin; int end; bool current; };
    std::string m_markPattern;
    vl::SearchOptions m_markOptions;
    // Compiled once per pattern and document, not on every paint.
    std::unique_ptr<vl::RegexSearcher> m_markRegex;
    vl::IByteAccess *m_markRegexDoc = nullptr;
    std::optional<std::pair<int64_t, int64_t>> m_currentMatch;
    std::vector<std::vector<MatchCols>> m_rowMatches;
    void computeMatchCols();
    void drawMatchBackground(QPainter &painter, int row);
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
    void setSearchMarks(const std::string &pattern, const vl::SearchOptions &options);
    void clearSearchMarks();
    std::optional<std::pair<int64_t, int64_t>> selectionBytes();
    QByteArray selectedBytes(int64_t maxSize);
    int64_t topByte();
    void setFontSize(qreal pt);
    qreal fontSize() const { return m_fontSize; }
    void setMaxTabW(int w);
    int maxTabW() const;
    void setHighlighter(qce::IHighlighter *h);
    void setFoldingProvider(qce::IFoldingProvider *f);
    void setFoldMaxBytes(int64_t n);
    int64_t foldMaxBytes() const { return m_foldMaxBytes; }
    bool charInseideArea(std::pair<int,int> cp);
    void drawSelBackground(QPainter &painter, int row);
Q_SIGNALS:
    void sizeChanged();
    void scrollHChanged();
    void scrollVChanged();
};
}

#endif //VIEWER_PAINTAREA_H

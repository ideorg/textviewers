//
// Created by Andrzej Borucki on 2022-09-13
//

#include <QPainter>
#include <QFontDatabase>
#include <cmath>
#include <QElapsedTimer>
#include <QDebug>
#include <QMenu>
#include <QKeyEvent>
#include "PaintArea.h"
#include <QApplication>
#include <QClipboard>
#include <QMimeData>
#include <QMessageBox>
#include <unicode/utf8.h>
#include "logic/ByteView.h"
#include "logic/ChangeableDocument.h"

namespace wid {

void PaintArea::paintEvent(QPaintEvent *event) {
    selection.compute(tv);
    computeRowLayout();
    QPainter painter(this);
    painter.setRenderHint(QPainter::TextAntialiasing);
    QRect R = event->rect();
    bool oneCharRepaint = R.width() == ceil(fontWidth);
    QPen pen(Qt::black);
    pen.setWidth(1);
    painter.setPen(pen);

    QString qstr = updateCaretPos();
    if (oneCharRepaint) {
        if (selection.charSelected(caretPos, tv))
            painter.fillRect(R, getSelColor());
        else
            painter.fillRect(R, Qt::white);
        painter.drawText(R, Qt::AlignLeft, qstr);
    } else {
        if (m_highlighter && byteAccess()) {
            paintHighlighted(painter);
        } else {
            paintPlain(painter);
        }
    }

    if (drawCaret && hasFocus()) {
        if (caretPos.first >= 0 && caretPos.first < (int) m_rowY.size()
            && m_rowY[caretPos.first] >= 0) {
            auto p = toScreenPos(caretPos, false);
            painter.fillRect(QRectF(p.second, p.first + 1, 2, fontHeight - 2), Qt::black);
        }
    }
}

void PaintArea::computeRowLayout() {
    int total = (int) tv->size();
    m_rowY.assign(total, -1.0);
    int visIdx = 0;
    auto *bytes = byteAccess();
    for (int r = 0; r < total; r++) {
        bool hidden = false;
        if (bytes && !m_collapsedBegins.empty()) {
            auto lp = tv->getLinePointers(r);
            int64_t wrapStart = bytes->pointerToOffset(lp.wrapPosition);
            for (const auto &F : m_foldCache) {
                if (!m_collapsedBegins.count((int64_t) F.beginByte)) continue;
                if ((int64_t) F.beginByte < wrapStart
                    && wrapStart < (int64_t) F.endByte) {
                    hidden = true;
                    break;
                }
            }
        }
        if (!hidden) {
            m_rowY[r] = visIdx * fontHeight;
            visIdx++;
        }
    }
    m_rowYEnd = visIdx * fontHeight;
}

int PaintArea::rowForVisualY(int pixelY) const {
    if (pixelY < 0) return -1;
    int last = -1;
    for (int r = 0; r < (int) m_rowY.size(); r++) {
        if (m_rowY[r] < 0) continue;
        if (m_rowY[r] > pixelY) break;
        last = r;
    }
    return last;
}

void PaintArea::paintPlain(QPainter &painter) {
    QString pilcrow = QChar(182);
    QPen pen(Qt::black);
    pen.setWidth(1);
    painter.setPen(pen);
    qreal gx = gutterPx();
    for (int i = 0; i < tv->size(); i++) {
        if (m_rowY[i] < 0) continue;
        qreal ry = m_rowY[i];
        drawSelBackground(painter, i);
        std::u32string dstr = tv->at(i);
        if (dstr.empty() && tv->lastInFile(i)) {
            QRectF R(gx, ry, this->rect().width() - gx, fontHeight);
            QPen pen1(Qt::gray);
            painter.setPen(pen1);
            painter.drawText(R, Qt::AlignLeft, pilcrow);
            painter.setPen(pen);
        } else {
            qreal x = gx;
            size_t start = 0;
            while (start < dstr.size()) {
                bool isHigh = (dstr[start] >= 0x10000);
                size_t end = start;
                while (end < dstr.size() && (dstr[end] >= 0x10000) == isHigh) {
                    end++;
                }
                QString segment = QString::fromUcs4(dstr.c_str() + start, end - start);
                qreal segmentWidth = (end - start) * fontWidth;
                QRectF R(x, ry, segmentWidth, fontHeight);
                if (isHigh) {
                    painter.drawText(R, Qt::AlignLeft | Qt::AlignVCenter, segment);
                } else {
                    painter.drawText(R, Qt::AlignLeft, segment);
                }
                x += segmentWidth;
                start = end;
            }
        }
        paintGutter(painter, i, false, false);
    }
    painter.fillRect(QRectF(0, m_rowYEnd, QWidget::width(),
                            QWidget::height() - m_rowYEnd), Qt::white);
}

void PaintArea::paintHighlighted(QPainter &painter) {
    auto *bytes = byteAccess();
    if (!bytes || tv->size() == 0) {
        paintPlain(painter);
        return;
    }

    // Visible byte range [topByte, endByte). For no-wrap mode with very long
    // logical lines the raw wrapEnd can be many megabytes past topByte even
    // though we only render `screenLineLen` codepoints per row - cap endByte
    // at what can actually be visible so tokenize/fold-scan stay bounded.
    auto lp0 = tv->getLinePointers(0);
    int64_t topByte = bytes->pointerToOffset(lp0.beginLine);
    auto lpLast = tv->getLinePointers((int) tv->size() - 1);
    int64_t rawEnd = bytes->pointerToOffset(lpLast.wrapEnd);
    int64_t visibleBytesCap = topByte +
                              (int64_t) tv->screenLineCount()
                              * (int64_t) tv->screenLineLen() * 4;
    int64_t endByte = std::min(rawEnd, visibleBytesCap);

    int64_t preludeStart = std::max(bytes->firstByte(), topByte - m_preludeBytes);
    int64_t tokenBufLen = endByte - preludeStart;
    if (tokenBufLen <= 0) {
        paintPlain(painter);
        return;
    }
    const char *buf = bytes->ofsetToPointer(preludeStart);

    QVector<qce::StyleSpan> tokens;
    qce::HighlightState endState;
    m_highlighter->tokenizeBytes(buf, (qsizetype) tokenBufLen,
                                 m_highlighter->initialState(),
                                 tokens, endState);
    const auto &palette = m_highlighter->attributes();

    if (m_folding) {
        bool cacheValid = m_foldCacheEnd > 0
            && m_foldCacheBegin <= preludeStart
            && std::min(bytes->byteCount(), endByte + m_foldMaxBytes) <= m_foldCacheEnd;
        if (!cacheValid) {
            int64_t scanBegin = std::max(bytes->firstByte(),
                                         topByte - 2 * m_preludeBytes);
            int64_t scanEnd = std::min(bytes->byteCount(),
                                       endByte + 2 * m_foldMaxBytes);
            const char *scanBuf = bytes->ofsetToPointer(scanBegin);
            int64_t scanLen = scanEnd - scanBegin;
            QVector<qce::FoldMarker> markers;
            qce::HighlightState foldEnd;
            m_folding->foldersInBytes(scanBuf, (qsizetype) scanLen,
                                      m_highlighter->initialState(),
                                      markers, foldEnd);
            for (auto &m : markers) m.column += (int) scanBegin;
            QVector<qce::FoldRange> raw = qce::pairFolds(markers, (qsizetype) m_foldMaxBytes);
            m_foldCache.clear();
            m_foldCache.reserve(raw.size());
            for (const auto &r : raw) {
                int64_t relBegin = (int64_t) r.beginByte - scanBegin;
                int64_t relEnd = (int64_t) r.endByte - scanBegin;
                if (relBegin < 0 || relEnd > scanLen) continue;
                bool multiline = false;
                for (int64_t k = relBegin; k < relEnd; k++) {
                    if (scanBuf[k] == '\n') { multiline = true; break; }
                }
                if (multiline) m_foldCache.push_back(r);
            }
            m_foldCacheBegin = scanBegin;
            m_foldCacheEnd = scanEnd;
        }
    } else {
        m_foldCache.clear();
        m_foldCacheBegin = m_foldCacheEnd = 0;
    }

    int tokIdx = 0;
    auto attrAt = [&](int64_t offRel) -> int {
        while (tokIdx < tokens.size()
               && offRel >= tokens[tokIdx].start + tokens[tokIdx].length) {
            tokIdx++;
        }
        if (tokIdx < tokens.size() && offRel >= tokens[tokIdx].start)
            return tokens[tokIdx].attributeId;
        return -1;
    };

    int screenLen = tv->screenLineLen();
    int maxTab = std::max(1, tv->maxTabW());
    QString pilcrow = QChar(182);
    qreal gx = gutterPx();

    std::vector<int64_t> foldBegins;
    foldBegins.reserve(m_foldCache.size());
    for (const auto &fr : m_foldCache) foldBegins.push_back((int64_t) fr.beginByte);
    std::sort(foldBegins.begin(), foldBegins.end());

    for (int r = 0; r < (int) tv->size(); r++) {
        if (m_rowY[r] < 0) continue;
        qreal ry = m_rowY[r];
        drawSelBackground(painter, r);
        auto lp = tv->getLinePointers(r);
        int64_t wrapStart = bytes->pointerToOffset(lp.wrapPosition);
        int64_t wrapEnd = bytes->pointerToOffset(lp.wrapEnd);
        auto u8 = reinterpret_cast<const uint8_t *>(buf);

        bool hasFoldBegin = false;
        bool collapsed = false;
        if (!foldBegins.empty()) {
            auto it = std::lower_bound(foldBegins.begin(), foldBegins.end(), wrapStart);
            if (it != foldBegins.end() && *it < wrapEnd) {
                hasFoldBegin = true;
                collapsed = m_collapsedBegins.count(*it) > 0;
            }
        }
        paintGutter(painter, r, hasFoldBegin, collapsed);

        if (wrapEnd == wrapStart && tv->lastInFile(r)) {
            QRectF R(gx, ry, this->rect().width() - gx, fontHeight);
            QPen pen1(Qt::gray);
            painter.setPen(pen1);
            painter.drawText(R, Qt::AlignLeft, pilcrow);
            continue;
        }

        // Skip startX codepoints of this wrap segment.
        int64_t i = wrapStart - preludeStart;
        int64_t wrapEndRel = wrapEnd - preludeStart;
        for (int n = 0; n < tv->startX() && i < wrapEndRel; n++) {
            int32_t k = 0;
            UChar32 dummy;
            U8_NEXT(u8 + i, k, (int32_t) (wrapEndRel - i), dummy);
            i += k;
        }

        struct Glyph { char32_t c; int attr; };
        std::vector<Glyph> glyphs;
        glyphs.reserve(screenLen);
        int width = 0;
        while (i < wrapEndRel && width < screenLen) {
            int curAttr = attrAt(i);
            if (u8[i] == '\t') {
                glyphs.push_back({U' ', curAttr});
                width++;
                while (width < screenLen && (width % maxTab) != 0) {
                    glyphs.push_back({U' ', curAttr});
                    width++;
                }
                i++;
            } else {
                int32_t k = 0;
                UChar32 c;
                U8_NEXT(u8 + i, k, (int32_t) (wrapEndRel - i), c);
                glyphs.push_back({c < 0 ? (char32_t) 0xFFFD : (char32_t) c, curAttr});
                i += k;
                width++;
            }
        }

        QFont baseFont = painter.font();
        QFont boldFont = baseFont;
        boldFont.setBold(true);
        auto attrStyle = [&](int id) -> qce::TextAttribute {
            if (id < 0 || id >= palette.size()) return {};
            return palette[id];
        };
        qreal x = gx;
        size_t start = 0;
        while (start < glyphs.size()) {
            int attr = glyphs[start].attr;
            bool isHigh = (glyphs[start].c >= 0x10000);
            qce::TextAttribute st = attrStyle(attr);
            size_t end = start + 1;
            while (end < glyphs.size()
                   && glyphs[end].attr == attr
                   && (glyphs[end].c >= 0x10000) == isHigh) {
                end++;
            }
            std::u32string seg;
            seg.reserve(end - start);
            for (size_t k = start; k < end; k++) seg.push_back(glyphs[k].c);
            QString qseg = QString::fromUcs4(seg.data(), (int) seg.size());
            qreal segW = (end - start) * fontWidth;
            QRectF R(x, ry, segW, fontHeight);
            painter.setPen(st.foreground.isValid() ? st.foreground : QColor(Qt::black));
            painter.setFont(st.bold ? boldFont : baseFont);
            if (isHigh)
                painter.drawText(R, Qt::AlignLeft | Qt::AlignVCenter, qseg);
            else
                painter.drawText(R, Qt::AlignLeft, qseg);
            x += segW;
            start = end;
        }
        painter.setFont(baseFont);
        if (hasFoldBegin && collapsed) {
            qreal stubW = fontWidth * 3.5;
            QRectF stubR(x + fontWidth * 0.25, ry + 1, stubW, fontHeight - 2);
            painter.fillRect(stubR, QColor(0xe0, 0xe0, 0xe0));
            painter.setPen(QColor(0x60, 0x60, 0x60));
            painter.drawRect(stubR);
            painter.setPen(QColor(0x30, 0x30, 0x30));
            painter.drawText(stubR, Qt::AlignCenter, QStringLiteral("\u2026"));
        }
    }
    painter.fillRect(QRectF(0, m_rowYEnd, QWidget::width(),
                            QWidget::height() - m_rowYEnd), Qt::white);
}

void PaintArea::setHighlighter(qce::IHighlighter *h) {
    m_highlighter = h;
    update();
}

void PaintArea::setFoldingProvider(qce::IFoldingProvider *f) {
    m_folding = f;
    m_foldCache.clear();
    m_foldCacheBegin = m_foldCacheEnd = 0;
    m_collapsedBegins.clear();
    update();
}

bool PaintArea::clickOnGutter(QMouseEvent *event) {
    if (!m_folding) return false;
    if (event->pos().x() >= (int) gutterPx()) return false;
    int row = rowForVisualY(event->pos().y());
    if (row < 0 || row >= (int) tv->size()) return true;
    auto *bytes = byteAccess();
    if (!bytes) return true;
    auto lp = tv->getLinePointers(row);
    int64_t wrapStart = bytes->pointerToOffset(lp.wrapPosition);
    int64_t wrapEnd = bytes->pointerToOffset(lp.wrapEnd);
    int64_t candidate = -1;
    for (const auto &fr : m_foldCache) {
        int64_t bb = (int64_t) fr.beginByte;
        if (bb < wrapStart || bb >= wrapEnd) continue;
        if (candidate < 0 || bb < candidate) candidate = bb;
    }
    if (candidate < 0) return true;
    auto it = m_collapsedBegins.find(candidate);
    if (it == m_collapsedBegins.end()) m_collapsedBegins.insert(candidate);
    else m_collapsedBegins.erase(it);
    update();
    return true;
}

void PaintArea::setFoldMaxBytes(int64_t n) {
    if (n <= 0) n = 1;
    m_foldMaxBytes = n;
    m_foldCache.clear();
    update();
}

QString PaintArea::updateCaretPos() {
    if (caretPos.first >= tv->size()) {
        caretPos.first = tv->size() - 1;
        caretPos.second = width() / fontWidth;
    }
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
    applyFont(m_fontSize);
    setData(addr, fileSize);
    setMouseTracking(true);
    connect(&timer, &QTimer::timeout, this, &PaintArea::doBlinkMethod);
    connect(&autoScrollTimer, &QTimer::timeout, this, &PaintArea::doAutoScroll);
}

void PaintArea::applyFont(qreal pt) {
    QFont font = QFontDatabase::systemFont(QFontDatabase::FixedFont);
    font.setPointSizeF(pt);
    font.setHintingPreference(QFont::PreferNoHinting);
    font.setStyleStrategy(QFont::StyleStrategy(QFont::PreferAntialias | QFont::PreferOutline));
    QFontMetricsF fm(font, this);
    fontWidth = fm.horizontalAdvance("0");
    fontHeight = fm.height();
    this->setFont(font);
    m_fontSize = pt;
}

void PaintArea::setFontSize(qreal pt) {
    applyFont(pt);
    setSize(width(), height());
    update();
}

void PaintArea::setMaxTabW(int w) {
    tv->setmaxTabW(w);
    update();
}

int PaintArea::maxTabW() const {
    return tv->maxTabW();
}

PaintArea::~PaintArea() {
    delete doc;
}

void PaintArea::setData(const char *addr, int64_t fileSize) {
    m_addr = addr;
    m_fileSize = fileSize;
    delete doc;
    delete tv;
    switch(logicKind) {
        case 0:
            doc = new vl::ByteDocument(addr, fileSize);
            tv = new vl::ByteView(dynamic_cast<vl::IByteAccess*>(doc));
            break;
        case 1:
            doc = new vl::LineIndexedDocument(addr, fileSize);
            tv = new vl::LineView(dynamic_cast<vl::ILineAccess*>(doc));
            break;
        default:
            doc = new vl::ChangeableDocument(addr, fileSize);
            tv = new vl::LineView(dynamic_cast<vl::ILineAccess*>(doc));
    }
    setSize(width(), height());
    selection.setDocument(doc);
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
    if (delta > 0)
        tv->scrollNUp(delta);
    else if (delta < 0)
        tv->scrollNDown(-delta);
    update();
    Q_EMIT scrollVChanged();

}

void PaintArea::setHorizontal(int startX) {
    if (tv->wrapMode() > 0)
        return;
    tv->setStartX(startX);
    tv->fillDeque();
    tv->recalcLines();
    update();
    Q_EMIT scrollHChanged();
}

void PaintArea::setVertical(double relativePos) {
    tv->gotoProportional(relativePos);
    tv->fillDeque();
    tv->recalcLines();
    update();
}

vl::IByteAccess *PaintArea::byteAccess() {
    return dynamic_cast<vl::IByteAccess*>(doc);
}

void PaintArea::showMatch(int64_t offset, int64_t length) {
    auto *bytes = byteAccess();
    if (!bytes) return;
    if (auto *bv = dynamic_cast<vl::ByteView*>(tv)) {
        bv->gotoByte(offset);
    } else {
        int64_t total = bytes->byteCount();
        double rel = total > 0 ? (double)offset / (double)total : 0.0;
        tv->gotoProportional(rel);
    }
    tv->fillDeque();
    tv->recalcLines();

    vl::FilePosition begin{};
    begin.interpretation = 1;
    begin.bytePosition = offset;
    vl::FilePosition end{};
    end.interpretation = 1;
    end.bytePosition = offset + length;

    if (tv->wrapMode() == 0) {
        auto startScreen = tv->locatePosition(begin, false);
        auto endScreen = tv->locatePosition(end, false);
        if (startScreen.first >= 0 && startScreen.first < (int)tv->size()
            && startScreen.first == endScreen.first) {
            int oldStartX = tv->startX();
            int startLogical = startScreen.second + oldStartX;
            int endLogical = endScreen.second + oldStartX;
            int matchCp = endLogical - startLogical;
            int fullCols = (int)(width() / fontWidth);
            int newStartX = 0;
            if (startLogical + matchCp > fullCols)
                newStartX = startLogical + matchCp - fullCols;
            if (newStartX != oldStartX) {
                tv->setStartX(newStartX);
                Q_EMIT scrollHChanged();
            }
        }
    }

    selection.setRange(begin, end, tv);
    update();
    Q_EMIT scrollVChanged();
}

void PaintArea::wheelHorizontal(int delta) {
    setHorizontal(std::max(0, tv->startX() - delta));
}

void PaintArea::doBlinkMethod() {
    drawCaret = !drawCaret;
    auto p = toScreenPos(caretPos, false);
    repaint(p.second, p.first, ceil(fontWidth), ceil(fontHeight));
}

std::pair<int, int> PaintArea::toCharPos(QPoint point, bool smart) {
    int adjusted = point.x() - (int) gutterPx();
    int px = smart ? adjusted + (int) (fontWidth / 2) : adjusted;
    int x = px < 0 ? 0 : floor(px / fontWidth);
    int y;
    if (m_rowY.empty()) {
        y = floor(point.y() / fontHeight);
    } else {
        int mapped = rowForVisualY(point.y());
        y = mapped < 0 ? floor(point.y() / fontHeight) : mapped;
    }
    return std::make_pair(y, x);
}

std::pair<int, int> PaintArea::toScreenPos(std::pair<int, int> point, bool smart) {
    int x = (int) gutterPx() + (int) (point.second * fontWidth);
    qreal y;
    if (point.first >= 0 && point.first < (int) m_rowY.size()
        && m_rowY[point.first] >= 0) {
        y = m_rowY[point.first];
    } else {
        y = point.first * fontHeight;
    }
    return std::make_pair((int) y, smart ? x - (int) (fontWidth / 2) : x);
}

void PaintArea::mousePressEvent(QMouseEvent *event) {
    QWidget::mousePressEvent(event);
    if (event->button() == Qt::LeftButton) {
        if (clickOnGutter(event)) return;
        setFocus();
        auto cp = toCharPos(event->pos(), true);
        if (!charInseideArea(cp)) return;
        if (sinceDoubleClick.isValid() &&
            sinceDoubleClick.elapsed() <= QApplication::doubleClickInterval()) {
            sinceDoubleClick.invalidate();
            trySetCaret(event->pos());
            selection.selectLogicalLine(cp.first, tv);
            update();
            return;
        }
        selecting = true;
        lastMousePos = event->pos();
        grabMouse();
        trySetCaret(event->pos());
        selection.setFirst(cp, tv);
        m_selAnchor.reset();
        update();
    }
}

void PaintArea::mouseDoubleClickEvent(QMouseEvent *event) {
    QWidget::mouseDoubleClickEvent(event);
    if (event->button() != Qt::LeftButton) return;
    auto cp = toCharPos(event->pos(), true);
    if (!charInseideArea(cp)) return;
    if (selecting) {
        selecting = false;
        autoScrollTimer.stop();
        releaseMouse();
    }
    trySetCaret(event->pos());
    selection.selectWord(cp, tv);
    sinceDoubleClick.start();
    update();
}

void PaintArea::mouseReleaseEvent(QMouseEvent *event) {
    if (event->button() != Qt::LeftButton) return;
    if (!selecting) return;
    selecting = false;
    autoScrollTimer.stop();
    releaseMouse();
    auto cp = toCharPos(event->pos(), true);
    if (charInseideArea(cp)) {
        selection.setSecond(cp, tv);
        update();
    }
}

void PaintArea::mouseMoveEvent(QMouseEvent *event) {
    if (m_folding && event->pos().x() < (int) gutterPx())
        setCursor(Qt::PointingHandCursor);
    else
        setCursor(Qt::IBeamCursor);
    if (!selecting) return;
    lastMousePos = event->pos();
    auto cp = toCharPos(event->pos(), true);

    bool needsScroll = false;
    if (event->pos().y() < 0 || event->pos().y() >= height())
        needsScroll = true;
    if (tv->wrapMode() == 0 && (event->pos().x() < 0 || event->pos().x() >= width()))
        needsScroll = true;

    if (needsScroll) {
        if (!autoScrollTimer.isActive())
            autoScrollTimer.start(50);
    } else {
        autoScrollTimer.stop();
        if (charInseideArea(cp)) {
            selection.setSecond(cp, tv);
            update();
        }
    }
    QWidget::mouseMoveEvent(event);
}

void PaintArea::doAutoScroll() {
    if (!selecting) {
        autoScrollTimer.stop();
        return;
    }

    bool scrolled = false;

    // Vertical scrolling
    if (lastMousePos.y() < 0) {
        tv->scrollUp();
        scrolled = true;
    } else if (lastMousePos.y() >= height()) {
        tv->scrollDown();
        scrolled = true;
    }

    // Horizontal scrolling (only when not wrapping)
    if (tv->wrapMode() == 0) {
        if (lastMousePos.x() < 0 && tv->startX() > 0) {
            tv->setStartX(tv->startX() - 1);
            tv->fillDeque();
            tv->recalcLines();
            scrolled = true;
        } else if (lastMousePos.x() >= width()) {
            tv->setStartX(tv->startX() + 1);
            tv->fillDeque();
            tv->recalcLines();
            scrolled = true;
        }
    }

    if (scrolled) {
        // Update selection to edge
        auto cp = toCharPos(lastMousePos, true);
        if (lastMousePos.y() < 0)
            cp.first = 0;
        else if (lastMousePos.y() >= height())
            cp.first = tv->size() - 1;
        if (cp.first >= 0 && cp.first < (int)tv->size()) {
            if (lastMousePos.x() < 0)
                cp.second = 0;
            else if (lastMousePos.x() >= width())
                cp.second = tv->screenLineLen();
            selection.setSecond(cp, tv);
        }
        update();
        Q_EMIT scrollVChanged();
        Q_EMIT scrollHChanged();
    }
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

void PaintArea::setWrapMode(int wrapMode) {
    double fw = (double) width() / fontWidth;
    tv->setWrapMode(wrapMode);
    tv->setScreenLineLen(tv->wrapMode() ? floor(fw) : ceil(fw));
    update();
}

void PaintArea::contextMenuEvent(QContextMenuEvent *event) {
    QMenu menu(this);
    QAction *actionCopy = menu.addAction("Copy");
    QAction *selectedAction = menu.exec(event->globalPos());

    if (selectedAction == actionCopy) {
        copyToClipboard();
    }
}

void PaintArea::copyToClipboard() {
    const int64_t COPY_WARN  = 10 * 1024 * 1024;        // 10 MB
    const int64_t COPY_LIMIT = 1024LL * 1024 * 1024;    // 1 GB
    int64_t size = selection.selectionSize();
    auto formatBytes = [](int64_t n) {
        QString s = QString::number(n);
        for (int i = s.length() - 3; i > 0; i -= 3) s.insert(i, '\'');
        return s;
    };
    if (size > COPY_LIMIT) {
        QMessageBox::warning(this, "Copy refused",
            QString("Selection size: %1 bytes.\nToo large to copy to the clipboard.")
                .arg(formatBytes(size)));
        return;
    }
    bool doCopy = true;
    if (size > COPY_WARN) {
        QMessageBox msgBox(this);
        msgBox.setWindowTitle("Copy confirmation");
        msgBox.setText(QString("Selection size: %1 bytes.\nCopy to clipboard?")
                           .arg(formatBytes(size)));
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::Cancel);
        msgBox.setDefaultButton(QMessageBox::Cancel);
        doCopy = (msgBox.exec() == QMessageBox::Yes);
    }
    if (doCopy) {
        QClipboard *clipboard = QGuiApplication::clipboard();
        QByteArray data = selection.get();
        QMimeData *mimeData = new QMimeData();
        mimeData->setData("text/plain", data);
        clipboard->setMimeData(mimeData);
    }
}

//smart "<=" in second<=screenLineLen
bool PaintArea::charInseideArea(std::pair<int, int> cp) {
    assert(tv->size() <= tv->screenLineCount());
    return cp.first >= 0 && cp.first < (int)tv->size() && cp.second >= 0 && cp.second <= tv->screenLineLen();
}

void PaintArea::drawSelBackground(QPainter &painter, int row) {
    int selColBeg = selection.selColBeg(row, tv);
    int selColEnd = selection.selColEnd(row, tv);
    qreal gx = gutterPx();
    qreal y = (row >= 0 && row < (int) m_rowY.size() && m_rowY[row] >= 0)
                  ? m_rowY[row]
                  : row * fontHeight;
    qreal h = fontHeight;
    qreal w = QWidget::width() - gx;
    if (selColBeg == -1) {
        if (selColEnd == -1)
            painter.fillRect(QRectF(gx, y, w, h), Qt::white);
        else
            assert(false);
    } else if (selColBeg == 0) {
        if (selColEnd == -1)
            painter.fillRect(QRectF(gx, y, w, h), getSelColor());
        else {
            qreal endX = selColEnd * fontWidth;
            painter.fillRect(QRectF(gx, y, endX, h), getSelColor());
            painter.fillRect(QRectF(gx + endX, y, w - endX, h), Qt::white);
        }
    } else {
        qreal begX = selColBeg * fontWidth;
        if (selColEnd == -1) {
            painter.fillRect(QRectF(gx, y, begX, h), Qt::white);
            painter.fillRect(QRectF(gx + begX, y, w - begX, h), getSelColor());
        } else {
            qreal endX = selColEnd * fontWidth;
            painter.fillRect(QRectF(gx, y, begX, h), Qt::white);
            painter.fillRect(QRectF(gx + begX, y, endX - begX, h), getSelColor());
            painter.fillRect(QRectF(gx + endX, y, w - endX, h), Qt::white);
        }
    }
}

void PaintArea::paintGutter(QPainter &painter, int row, bool hasFoldBegin, bool collapsed) {
    qreal gx = gutterPx();
    if (gx <= 0) return;
    qreal y = (row >= 0 && row < (int) m_rowY.size() && m_rowY[row] >= 0)
                  ? m_rowY[row]
                  : row * fontHeight;
    painter.fillRect(QRectF(0, y, gx, fontHeight), QColor(0xf0, 0xf0, 0xf0));
    if (!hasFoldBegin) return;
    QPen savedPen = painter.pen();
    QPen pen(QColor(0x20, 0x20, 0x20));
    pen.setWidthF(std::max<qreal>(1.2, gx / 14.0));
    pen.setCapStyle(Qt::RoundCap);
    pen.setJoinStyle(Qt::MiterJoin);
    painter.setPen(pen);
    painter.setRenderHint(QPainter::Antialiasing, true);
    qreal cx = gx * 0.5;
    qreal cy = y + fontHeight * 0.5;
    qreal s = std::min(gx, (qreal) fontHeight) * 0.28;
    QPointF p1, p2, p3;
    if (collapsed) {
        p1 = {cx - s * 0.5, cy - s};
        p2 = {cx + s * 0.5, cy};
        p3 = {cx - s * 0.5, cy + s};
    } else {
        p1 = {cx - s, cy - s * 0.5};
        p2 = {cx,     cy + s * 0.5};
        p3 = {cx + s, cy - s * 0.5};
    }
    painter.drawLine(p1, p2);
    painter.drawLine(p2, p3);
    painter.setRenderHint(QPainter::Antialiasing, false);
    painter.setPen(savedPen);
}

void PaintArea::setKind(int kind) {
    logicKind = kind;
    setData(m_addr, m_fileSize);
}

void PaintArea::moveCaret(int newRow, int newCol, bool shift) {
    if (shift && !m_selAnchor)
        m_selAnchor = tv->filePosition(caretPos.first, caretPos.second);
    else if (!shift)
        m_selAnchor.reset();

    int viewRows = (int) tv->size();
    if (newRow < 0) {
        for (int k = 0; k < -newRow; k++) tv->scrollUp();
        newRow = 0;
        Q_EMIT scrollVChanged();
    } else if (viewRows > 0 && newRow >= viewRows) {
        for (int k = 0; k < newRow - viewRows + 1; k++) tv->scrollDown();
        newRow = (int) tv->size() - 1;
        if (newRow < 0) newRow = 0;
        Q_EMIT scrollVChanged();
    }

    int maxCol = (int) (width() / fontWidth);
    if (newCol < 0) {
        tv->setStartX(std::max(0, tv->startX() + newCol));
        newCol = 0;
        Q_EMIT scrollHChanged();
    } else if (newCol > maxCol) {
        tv->setStartX(tv->startX() + (newCol - maxCol));
        newCol = maxCol;
        Q_EMIT scrollHChanged();
    }

    caretPos = {newRow, newCol};

    if (shift && m_selAnchor) {
        auto caretFP = tv->filePosition(newRow, newCol);
        selection.setRange(*m_selAnchor, caretFP, tv);
    } else {
        selection.setFirst(caretPos, tv);
    }
    update();
}

void PaintArea::keyPressEvent(QKeyEvent *event) {
    bool ctrl = event->modifiers() & Qt::ControlModifier;
    bool shift = event->modifiers() & Qt::ShiftModifier;
    switch (event->key()) {
        case Qt::Key_Up:
            moveCaret(caretPos.first - 1, caretPos.second, shift);
            break;
        case Qt::Key_Down:
            moveCaret(caretPos.first + 1, caretPos.second, shift);
            break;
        case Qt::Key_Left:
            moveCaret(caretPos.first, caretPos.second - 1, shift);
            break;
        case Qt::Key_Right:
            moveCaret(caretPos.first, caretPos.second + 1, shift);
            break;
        case Qt::Key_PageUp:
            moveCaret(caretPos.first - tv->screenLineCount(), caretPos.second, shift);
            break;
        case Qt::Key_PageDown:
            moveCaret(caretPos.first + tv->screenLineCount(), caretPos.second, shift);
            break;
        case Qt::Key_Home:
            if (ctrl) {
                if (shift && !m_selAnchor)
                    m_selAnchor = tv->filePosition(caretPos.first, caretPos.second);
                else if (!shift)
                    m_selAnchor.reset();
                tv->gotoProportional(0);
                tv->fillDeque();
                tv->recalcLines();
                if (tv->startX() != 0) {
                    tv->setStartX(0);
                    Q_EMIT scrollHChanged();
                }
                caretPos = {0, 0};
                if (shift && m_selAnchor) {
                    auto caretFP = tv->filePosition(0, 0);
                    selection.setRange(*m_selAnchor, caretFP, tv);
                } else {
                    selection.setFirst(caretPos, tv);
                }
                update();
                Q_EMIT scrollVChanged();
            } else {
                if (tv->startX() != 0) {
                    tv->setStartX(0);
                    Q_EMIT scrollHChanged();
                }
                moveCaret(caretPos.first, 0, shift);
            }
            break;
        case Qt::Key_End:
            if (ctrl) {
                if (shift && !m_selAnchor)
                    m_selAnchor = tv->filePosition(caretPos.first, caretPos.second);
                else if (!shift)
                    m_selAnchor.reset();
                tv->gotoProportional(1);
                tv->fillDeque();
                tv->recalcLines();
                int lastRow = std::max(0, (int) tv->size() - 1);
                int lastCol = (int) tv->at(lastRow).size();
                caretPos = {lastRow, lastCol};
                if (shift && m_selAnchor) {
                    auto caretFP = tv->filePosition(lastRow, lastCol);
                    selection.setRange(*m_selAnchor, caretFP, tv);
                } else {
                    selection.setFirst(caretPos, tv);
                }
                update();
                Q_EMIT scrollVChanged();
            } else {
                int row = caretPos.first;
                int endCol = 0;
                if (row >= 0 && row < (int) tv->size())
                    endCol = (int) tv->at(row).size();
                moveCaret(row, endCol, shift);
            }
            break;
        case Qt::Key_C:
            if (ctrl)
                copyToClipboard();
            break;
        case Qt::Key_A:
            if (ctrl) {
                selection.selectAll(tv);
                m_selAnchor.reset();
                update();
            }
            break;
        default:
            QWidget::keyPressEvent(event);
    }
}

#endif
}

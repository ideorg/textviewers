#include "TextViewer.h"

#include <QPainter>
#include <QKeyEvent>
#include <QFontDatabase>
#include <QHBoxLayout>
#include <QProxyStyle>
#include <QDebug>
#include <QFutureWatcher>
#include <QtConcurrent/QtConcurrentRun>

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
    // No base style: QProxyStyle(QStyle*) would take ownership of the
    // application style and delete it together with the viewer.
    auto *jumpStyle = new JumpToClickStyle;
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
    m_findTimer.setInterval(50);
    connect(&m_findTimer, &QTimer::timeout, this, [this] {
        if (m_finding && m_findDone) emit findProgress(m_findDone->load(), m_findRange);
    });
}

TextViewer::~TextViewer() {
    stopFind();
}

void TextViewer::setData(const char *addr, int64_t fileSize) {
    stopFind();
    paintArea->setData(addr, fileSize);
}

void TextViewer::stopFind() {
    if (!m_finding) return;
    m_findCancel->store(true, std::memory_order_relaxed);
    m_findFuture.waitForFinished();
    m_finding = false;
    m_findTimer.stop();
    ++m_findId;  // its watcher's end is let go
}

void TextViewer::cancelFind() {
    if (!m_finding) return;
    stopFind();
    emit findFinished(FindOutcome::Cancelled);
}

void TextViewer::find(const std::string &pattern, const vl::SearchOptions &options, bool backwards,
                      bool wrap) {
    stopFind();
    vl::IByteAccess *bytes = byteAccess();
    if (!bytes || pattern.empty()) {
        emit findFinished(FindOutcome::NotFound);
        return;
    }
    // From the current result (or selection), otherwise from the top of the view.
    auto sel = selectionBytes();
    int64_t from = sel ? (backwards ? sel->first : sel->second) : topByte();
    int64_t first = bytes->firstByte();
    int64_t total = bytes->byteCount();
    int64_t pLen = (int64_t) pattern.size();
    vl::SearchOptions opts = options;

    auto cancel = std::make_shared<std::atomic<bool>>(false);
    auto done = std::make_shared<std::atomic<int64_t>>(0);
    m_findCancel = cancel;
    m_findDone = done;
    // Both passes together cover the file once.
    m_findRange = wrap ? total - first : (backwards ? from - first : total - from);
    m_findFuture = QtConcurrent::run([=]() -> FindHit {
        int64_t passOffset = 0;
        auto progress = [&](int64_t d, int64_t) {
            done->store(passOffset + d, std::memory_order_relaxed);
            return !cancel->load(std::memory_order_relaxed);
        };
        if (opts.regex) {
            // The second pass covers the matches the first one could not reach;
            // one crossing the starting point may be missed.
            vl::RegexSearcher regex(bytes, pattern, opts);
            std::optional<vl::RegexMatch> hit;
            if (!backwards) {
                hit = regex.findNext(from, progress);
                if (!hit && wrap && !cancel->load()) {
                    passOffset = total - from;
                    return {regex.findNext(first, progress, from), true};
                }
            } else {
                hit = regex.findPrev(from, progress);
                if (!hit && wrap && !cancel->load()) {
                    passOffset = from - first;
                    return {regex.findPrev(total, progress, from), true};
                }
            }
            return {hit, false};
        }
        vl::Searcher searcher(bytes);
        std::optional<int64_t> hit;
        bool wrapped = false;
        if (!backwards) {
            hit = searcher.findNext(pattern, from, progress, opts);
            if (!hit && wrap && !cancel->load()) {
                passOffset = total - from;
                wrapped = true;
                hit = searcher.findNext(pattern, first, progress, opts, from + pLen - 1);
            }
        } else {
            hit = searcher.findPrev(pattern, from, progress, opts);
            if (!hit && wrap && !cancel->load()) {
                passOffset = from - first;
                wrapped = true;
                hit = searcher.findPrev(pattern, total, progress, opts,
                                        std::max(first, from - pLen + 1));
            }
        }
        if (!hit) return {std::nullopt, false};
        return {vl::RegexMatch{*hit, pLen}, wrapped};
    });
    m_finding = true;
    m_findTimer.start();
    const unsigned id = ++m_findId;
    auto *watcher = new QFutureWatcher<FindHit>(this);
    connect(watcher, &QFutureWatcher<FindHit>::finished, this, [this, watcher, id, cancel] {
        watcher->deleteLater();
        if (id != m_findId) return;  // stopped, or another search since
        m_finding = false;
        m_findTimer.stop();
        FindHit hit = watcher->result();
        if (cancel->load()) {
            emit findFinished(FindOutcome::Cancelled);
        } else if (hit.match) {
            showMatch(hit.match->offset, hit.match->length);
            emit findFinished(hit.wrapped ? FindOutcome::Wrapped : FindOutcome::Found);
        } else {
            emit findFinished(FindOutcome::NotFound);
        }
    });
    watcher->setFuture(m_findFuture);
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

void TextViewer::setSearchMarks(const std::string &pattern, const vl::SearchOptions &options) {
    paintArea->setSearchMarks(pattern, options);
}

void TextViewer::clearSearchMarks() {
    paintArea->clearSearchMarks();
}

std::optional<std::pair<int64_t, int64_t>> TextViewer::selectionBytes() {
    return paintArea->selectionBytes();
}

QByteArray TextViewer::selectedBytes(int64_t maxSize) {
    return paintArea->selectedBytes(maxSize);
}

int64_t TextViewer::topByte() {
    return paintArea->topByte();
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

void TextViewer::setHighlighter(qce::IHighlighter *h) {
    paintArea->setHighlighter(h);
}

void TextViewer::setFoldingProvider(qce::IFoldingProvider *f) {
    paintArea->setFoldingProvider(f);
}

void TextViewer::setFoldMaxBytes(int64_t n) {
    paintArea->setFoldMaxBytes(n);
}

int64_t TextViewer::foldMaxBytes() const {
    return paintArea->foldMaxBytes();
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
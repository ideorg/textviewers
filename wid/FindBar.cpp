#include "FindBar.h"
#include "TextViewer.h"

#include <QApplication>
#include <QElapsedTimer>
#include <QFutureWatcher>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QProgressBar>
#include <QShortcut>
#include <QStyle>
#include <QTimer>
#include <QToolButton>
#include <QToolTip>
#include <QtConcurrent/QtConcurrentRun>

namespace wid {

FindBar::FindBar(TextViewer *viewer, QWidget *scope) : QWidget(scope), m_viewer(viewer) {
    setObjectName("findBar");
    auto *layout = new QHBoxLayout(this);
    layout->setContentsMargins(8, 6, 8, 6);
    layout->setSpacing(5);
    auto button = [this](const QString &text, const QString &tip, bool toggle = false) {
        auto *b = new QToolButton(this);
        b->setText(text);
        b->setToolTip(tip);
        b->setAutoRaise(true);
        b->setCheckable(toggle);
        return b;
    };
    auto *field = new QWidget(this);
    field->setObjectName("findField");
    field->setStyleSheet("#findField { border: 1px solid palette(mid); border-radius: 3px; background: palette(base); }");
    auto *fieldLayout = new QHBoxLayout(field);
    fieldLayout->setContentsMargins(5, 1, 3, 1);
    fieldLayout->setSpacing(1);
    fieldLayout->addWidget(new QLabel(QString::fromUtf8("⌕"), field));
    m_query = new QLineEdit(field);
    m_query->setObjectName("findQuery");
    m_query->setFrame(false);
    m_query->setClearButtonEnabled(true);
    m_query->setPlaceholderText(tr("Search"));
    fieldLayout->addWidget(m_query, 1);
    m_wrap = button(QString::fromUtf8("↩"), tr("Wrap around"), true);
    m_wrap->setChecked(true);
    m_case = button("Cc", tr("Match case"), true);
    m_words = button("W", tr("Whole words"), true);
    m_regex = button(".*", tr("Regular expression"), true);
    for (auto *b : {m_wrap, m_case, m_words, m_regex}) fieldLayout->addWidget(b);
    field->setMinimumWidth(260);
    field->setMaximumWidth(360);
    layout->addWidget(field, 1);
    // No count of results (the file may be too large to know it): only the
    // error of an invalid pattern.
    m_status = new QLabel(this);
    m_status->setObjectName("findStatus");
    layout->addWidget(m_status);
    // The progress of a long F3 search, in the bar rather than in a dialog: on
    // Wayland a dialog shown and closed at once leaves its activation token
    // unused, and GNOME keeps the busy cursor for seconds.
    m_progress = new QProgressBar(this);
    m_progress->setObjectName("findProgress");
    m_progress->setRange(0, 1000);
    m_progress->setTextVisible(false);
    m_progress->setFixedWidth(120);
    m_progress->hide();
    layout->addWidget(m_progress);
    m_stop = button(QString(), tr("Stop searching (Escape)"));
    m_stop->setIcon(style()->standardIcon(QStyle::SP_BrowserStop));
    m_stop->hide();
    layout->addWidget(m_stop);
    m_previous = button(QString(), tr("Previous match (Shift+F3)"));
    m_previous->setIcon(style()->standardIcon(QStyle::SP_ArrowUp));
    m_next = button(QString(), tr("Next match (F3)"));
    m_next->setIcon(style()->standardIcon(QStyle::SP_ArrowDown));
    layout->addWidget(m_previous);
    layout->addWidget(m_next);
    layout->addStretch();
    auto *close = button(QString(), tr("Close search (Escape)"));
    close->setIcon(style()->standardIcon(QStyle::SP_DialogCloseButton));
    layout->addWidget(close);

    connect(close, &QToolButton::clicked, this, &FindBar::closeSearch);
    connect(m_stop, &QToolButton::clicked, this, [this] {
        if (m_cancel) m_cancel->store(true);
    });
    connect(m_previous, &QToolButton::clicked, this, [this] { findNext(true); });
    connect(m_next, &QToolButton::clicked, this, [this] { findNext(); });
    connect(m_query, &QLineEdit::textChanged, this, &FindBar::updateMarks);
    connect(m_query, &QLineEdit::returnPressed, this, [this] { findNext(); });
    for (auto *b : {m_case, m_words, m_regex})
        connect(b, &QToolButton::toggled, this, &FindBar::updateMarks);
    auto *backwards = new QShortcut(QKeySequence(Qt::SHIFT | Qt::Key_Return), m_query);
    connect(backwards, &QShortcut::activated, this, [this] { findNext(true); });

    auto addShortcut = [scope](const QKeySequence &key) {
        auto *shortcut = new QShortcut(key, scope);
        shortcut->setContext(Qt::WidgetWithChildrenShortcut);
        return shortcut;
    };
    connect(addShortcut(QKeySequence::Find), &QShortcut::activated, this, &FindBar::showSearch);
    connect(addShortcut(QKeySequence(Qt::Key_F3)), &QShortcut::activated, this, [this] { findNext(); });
    connect(addShortcut(QKeySequence(Qt::SHIFT | Qt::Key_F3)), &QShortcut::activated, this,
            [this] { findNext(true); });
    // Only while the bar is shown, so Escape otherwise reaches the window (e.g. to close a viewer).
    m_escape = addShortcut(QKeySequence(Qt::Key_Escape));
    m_escape->setEnabled(false);
    connect(m_escape, &QShortcut::activated, this, &FindBar::closeSearch);
    hide();
}

FindBar::~FindBar() {
    // The search reads the viewer's data, which goes away with the host.
    cancelSearch();
}

void FindBar::cancelSearch() {
    if (m_cancel) m_cancel->store(true, std::memory_order_relaxed);
    m_future.waitForFinished();
}

void FindBar::showEvent(QShowEvent *event) {
    m_escape->setEnabled(true);
    QWidget::showEvent(event);
}

void FindBar::hideEvent(QHideEvent *event) {
    m_escape->setEnabled(false);
    QWidget::hideEvent(event);
}

vl::SearchOptions FindBar::options() const {
    vl::SearchOptions opts;
    opts.caseInsensitive = !m_case->isChecked();
    opts.wholeWord = m_words->isChecked();
    opts.regex = m_regex->isChecked();
    return opts;
}

bool FindBar::checkPattern() {
    m_status->clear();
    m_query->setToolTip(QString());
    bool valid = true;
    if (m_regex->isChecked() && !m_query->text().isEmpty()) {
        QByteArray pattern = m_query->text().toUtf8();
        vl::RegexSearcher regex(m_viewer->byteAccess(), pattern.toStdString(), options());
        if (!regex.isValid()) {
            m_status->setText(tr("Invalid pattern"));
            QString error = QString::fromStdString(regex.errorString());
            // The offset is in bytes of UTF-8; shown as a position in characters.
            int position = QString::fromUtf8(pattern.left((int) regex.errorOffset())).size();
            m_query->setToolTip(tr("%1 at position %2").arg(error).arg(position + 1));
            valid = false;
        }
    }
    m_next->setEnabled(valid);
    m_previous->setEnabled(valid);
    return valid;
}

void FindBar::showSearch() {
    // The selected text is searched for, if it is one short line.
    QByteArray selected = m_viewer->selectedBytes(1024);
    if (!selected.isEmpty() && !selected.contains('\n') && !selected.contains('\r'))
        m_query->setText(QString::fromUtf8(selected));
    show();
    updateMarks();
    m_query->setFocus();
    m_query->selectAll();
}

void FindBar::closeSearch() {
    // Escape during a search stops it; the next one closes the bar.
    if (m_busy) {
        if (m_cancel) m_cancel->store(true);
        return;
    }
    hide();
    m_viewer->clearSearchMarks();
    m_viewer->setFocus();
}

void FindBar::updateMarks() {
    if (isHidden()) return;
    if (!checkPattern()) {
        m_viewer->clearSearchMarks();
        return;
    }
    m_viewer->setSearchMarks(m_query->text().toUtf8().toStdString(), options());
}

void FindBar::showNotFound() {
    QWidget *anchor = isVisible() ? static_cast<QWidget *>(m_query) : static_cast<QWidget *>(m_viewer);
    QToolTip::showText(anchor->mapToGlobal(QPoint(0, anchor->height())),
                       tr("\"%1\" not found").arg(m_query->text()), anchor);
}

void FindBar::findNext(bool backwards) {
    if (m_busy) return;
    std::string pattern = m_query->text().toUtf8().toStdString();
    if (pattern.empty()) {
        showSearch();
        return;
    }
    vl::IByteAccess *bytes = m_viewer->byteAccess();
    if (!bytes || !checkPattern()) return;

    // From the current result (or selection), otherwise from the top of the view.
    auto sel = m_viewer->selectionBytes();
    int64_t from = sel ? (backwards ? sel->first : sel->second) : m_viewer->topByte();
    int64_t first = bytes->firstByte();
    int64_t total = bytes->byteCount();
    int64_t pLen = (int64_t) pattern.size();
    bool wrap = m_wrap->isChecked();
    vl::SearchOptions opts = options();

    m_cancel = std::make_shared<std::atomic<bool>>(false);
    auto cancel = m_cancel;
    auto done = std::make_shared<std::atomic<int64_t>>(0);
    // Both passes together cover the file once.
    int64_t range = wrap ? total - first : (backwards ? from - first : total - from);
    m_future = QtConcurrent::run([=]() -> std::optional<vl::RegexMatch> {
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
                    hit = regex.findNext(first, progress, from);
                }
            } else {
                hit = regex.findPrev(from, progress);
                if (!hit && wrap && !cancel->load()) {
                    passOffset = from - first;
                    hit = regex.findPrev(total, progress, from);
                }
            }
            return hit;
        }
        vl::Searcher searcher(bytes);
        std::optional<int64_t> hit;
        if (!backwards) {
            hit = searcher.findNext(pattern, from, progress, opts);
            if (!hit && wrap && !cancel->load()) {
                passOffset = total - from;
                hit = searcher.findNext(pattern, first, progress, opts, from + pLen - 1);
            }
        } else {
            hit = searcher.findPrev(pattern, from, progress, opts);
            if (!hit && wrap && !cancel->load()) {
                passOffset = from - first;
                hit = searcher.findPrev(pattern, total, progress, opts,
                                        std::max(first, from - pLen + 1));
            }
        }
        if (!hit) return std::nullopt;
        return vl::RegexMatch{*hit, pLen};
    });

    m_busy = true;
    m_shownForSearch = false;
    // Progress only for a search that takes a while, so a near result does
    // not flash it. With the bar hidden (F3) it is shown for the search.
    auto started = std::make_shared<QElapsedTimer>();
    started->start();
    auto *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, [this, started, done, range] {
        if (m_progress->isHidden() && started->elapsed() >= 300) {
            if (isHidden()) {
                m_shownForSearch = true;
                show();
            }
            m_progress->show();
            m_stop->show();
        }
        if (range > 0)
            m_progress->setValue((int) std::min<int64_t>(1000, 1000 * done->load() / range));
    });
    timer->start(50);

    auto *watcher = new QFutureWatcher<std::optional<vl::RegexMatch>>(this);
    connect(watcher, &QFutureWatcher<std::optional<vl::RegexMatch>>::finished, this,
            [this, watcher, timer, cancel] {
                m_busy = false;
                timer->stop();
                timer->deleteLater();
                m_progress->hide();
                m_stop->hide();
                m_progress->setValue(0);
                // Shown only for the search: hidden again, unless the user went into it.
                if (m_shownForSearch && !isAncestorOf(QApplication::focusWidget()))
                    hide();
                m_shownForSearch = false;
                watcher->deleteLater();
                auto result = watcher->result();
                if (result) {
                    m_viewer->showMatch(result->offset, result->length);
                    updateMarks();
                } else if (!cancel->load()) {
                    showNotFound();
                }
            });
    watcher->setFuture(m_future);
}

}

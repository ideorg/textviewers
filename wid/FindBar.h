#ifndef VIEWER_FINDBAR_H
#define VIEWER_FINDBAR_H

#include <QWidget>
#include <QFuture>
#include <atomic>
#include <memory>
#include <optional>
#include "logic/RegexSearcher.h"
#include "logic/Searcher.h"

class QLabel;
class QLineEdit;
class QToolButton;
class QShortcut;

namespace wid {

class TextViewer;

// Search bar for TextViewer, looking like the one of qceditor's editor.
// Occurrences are marked only in the visible rows (no count: the file may be
// too large to know it); F3 / Shift+F3 search the file in the background with
// a cancellable progress dialog.
//
// The host puts the bar in its layout (above the viewer). Ctrl+F, F3,
// Shift+F3 and, while the bar is shown, Escape work when the focus is in
// `scope` or its children.
class FindBar : public QWidget {
    Q_OBJECT
public:
    FindBar(TextViewer *viewer, QWidget *scope);
    ~FindBar() override;
    void showSearch();
    void findNext(bool backwards = false);
    void closeSearch();
protected:
    void showEvent(QShowEvent *event) override;
    void hideEvent(QHideEvent *event) override;
private:
    vl::SearchOptions options() const;
    // Checks a regular expression; shows its error and returns false if invalid.
    bool checkPattern();
    void updateMarks();
    void showNotFound();
    TextViewer *m_viewer;
    QLineEdit *m_query;
    QToolButton *m_wrap;
    QToolButton *m_case;
    QToolButton *m_words;
    QToolButton *m_regex;
    QLabel *m_status;
    QToolButton *m_previous;
    QToolButton *m_next;
    QShortcut *m_escape;
    std::shared_ptr<std::atomic<bool>> m_cancel;
    QFuture<std::optional<vl::RegexMatch>> m_future;
    bool m_busy = false;
};

}

#endif // VIEWER_FINDBAR_H

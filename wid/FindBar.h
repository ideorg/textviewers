#ifndef VIEWER_FINDBAR_H
#define VIEWER_FINDBAR_H

#include <QWidget>
#include <QElapsedTimer>
#include "logic/RegexSearcher.h"
#include "logic/Searcher.h"
#include "TextViewer.h"

class QLabel;
class QProgressBar;
class QLineEdit;
class QToolButton;
class QShortcut;

namespace wid {

// Search bar for TextViewer, looking like the one of qceditor's editor.
// Occurrences are marked only in the visible rows (no count: the file may be
// too large to know it); F3 / Shift+F3 search the file in the background
// (TextViewer::find()), a long search showing its progress in the bar,
// stopped by Escape.
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
    // Stops a running search and waits for it, e.g. before the viewer's data
    // is replaced.
    void cancelSearch();
protected:
    void showEvent(QShowEvent *event) override;
    void hideEvent(QHideEvent *event) override;
private:
    vl::SearchOptions options() const;
    // Checks a regular expression; shows its error and returns false if invalid.
    bool checkPattern();
    void updateMarks();
    void showNotFound();
    void searchProgress(qint64 done, qint64 total);
    void searchFinished(TextViewer::FindOutcome outcome);
    TextViewer *m_viewer;
    QLineEdit *m_query;
    QToolButton *m_wrap;
    QToolButton *m_case;
    QToolButton *m_words;
    QToolButton *m_regex;
    QLabel *m_status;
    QProgressBar *m_progress;
    QToolButton *m_stop;
    bool m_shownForSearch = false;
    QToolButton *m_previous;
    QToolButton *m_next;
    QShortcut *m_escape;
    QElapsedTimer m_started;  // of the search running
    bool m_busy = false;
};

}

#endif // VIEWER_FINDBAR_H

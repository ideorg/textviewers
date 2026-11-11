#ifndef VIEWER_TEXTVIEWER_H
#define VIEWER_TEXTVIEWER_H

#include <QWidget>
#include <QScrollBar>
#include <QFuture>
#include <QTimer>
#include <atomic>
#include <memory>
#include "PaintArea.h"

namespace wid {
    class TextViewer : public QWidget {
    Q_OBJECT
        QScrollBar *vscroll;
        QScrollBar *hscroll;
        PaintArea *paintArea = nullptr;
    public:
        // How a search of find() ended.
        enum class FindOutcome { Found, Wrapped, NotFound, Cancelled };
        Q_ENUM(FindOutcome)

        explicit TextViewer(const char *addr, int64_t fileSize, QWidget *parent = nullptr);
        // Stops a search running: it reads the data.
        ~TextViewer() override;
        // Stops a search running first.
        void setData(const char *addr, int64_t fileSize);
        void setWrapMode(int mode);
        void setKind(int kind);
        vl::IByteAccess *byteAccess();
        void showMatch(int64_t offset, int64_t length);
        // Marks the occurrences of pattern in the visible rows.
        void setSearchMarks(const std::string &pattern, const vl::SearchOptions &options);
        void clearSearchMarks();
        // Seeks pattern in the background: after the selection (before it,
        // backwards), otherwise from the top of the view; around the end
        // (start) when wrap. A match found is shown (showMatch()). A search
        // still running is cancelled first. findProgress() comes while it
        // runs, findFinished() at the end (Wrapped: found in the part after
        // going around).
        void find(const std::string &pattern, const vl::SearchOptions &options, bool backwards,
                  bool wrap = true);
        // Stops a running search, waits for it and tells findFinished(Cancelled).
        void cancelFind();
        bool isFinding() const { return m_finding; }
        std::optional<std::pair<int64_t, int64_t>> selectionBytes();
        QByteArray selectedBytes(int64_t maxSize);
        int64_t topByte();
        void setFontSize(qreal pt);
        qreal fontSize() const;
        void setMaxTabW(int w);
        int maxTabW() const;
        void setHighlighter(qce::IHighlighter *h);
        void setFoldingProvider(qce::IFoldingProvider *f);
        void setFoldMaxBytes(int64_t n);
        int64_t foldMaxBytes() const;
    signals:
        // `done` of `total` bytes sought so far.
        void findProgress(qint64 done, qint64 total);
        void findFinished(wid::TextViewer::FindOutcome outcome);
    private:
        struct FindHit {
            std::optional<vl::RegexMatch> match;
            bool wrapped = false;
        };
        // Stops a running search and waits for it, telling nobody.
        void stopFind();
        const int MAXVSCROLL = 10000;
        std::shared_ptr<std::atomic<bool>> m_findCancel;
        std::shared_ptr<std::atomic<int64_t>> m_findDone;
        int64_t m_findRange = 0;
        QFuture<FindHit> m_findFuture;
        QTimer m_findTimer;     // findProgress()
        unsigned m_findId = 0;  // of the search whose end is awaited
        bool m_finding = false;
        void hscrollChanged();
        void vscrollChanged();
        void sizeChanged();
        void setHScrollChange();
        void setVScrollChange();
    };
}
#endif // VIEWER_TEXTVIEWER_H
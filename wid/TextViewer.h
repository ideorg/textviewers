#ifndef VIEWER_TEXTVIEWER_H
#define VIEWER_TEXTVIEWER_H

#include <QWidget>
#include <QScrollBar>
#include "PaintArea.h"

namespace wid {
    class TextViewer : public QWidget {
    Q_OBJECT
        QScrollBar *vscroll;
        QScrollBar *hscroll;
        PaintArea *paintArea = nullptr;
    public:
        explicit TextViewer(const char *addr, int64_t fileSize, QWidget *parent = nullptr);
        void setData(const char *addr, int64_t fileSize);
        void setWrapMode(int mode);
        void setKind(int kind);
        vl::IByteAccess *byteAccess();
        void showMatch(int64_t offset, int64_t length);
        // Marks the occurrences of pattern in the visible rows.
        void setSearchMarks(const std::string &pattern, const vl::SearchOptions &options);
        void clearSearchMarks();
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
    private:
        const int MAXVSCROLL = 10000;
        void hscrollChanged();
        void vscrollChanged();
        void sizeChanged();
        void setHScrollChange();
        void setVScrollChange();
    };
}
#endif // VIEWER_TEXTVIEWER_H
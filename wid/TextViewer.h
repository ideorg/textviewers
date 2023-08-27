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
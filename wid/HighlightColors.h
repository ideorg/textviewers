#pragma once

#include <QColor>
#include <QVector>

namespace wid {

struct AttrStyle {
    QColor foreground;
    bool   bold      = false;
    bool   italic    = false;
    bool   underline = false;
};

// Palette mapping attribute ids from a vl::IHighlighter to display styles.
// Built once per highlighter instance and handed to PaintArea.
class HighlightColors {
public:
    void setStyle(int attributeId, AttrStyle style);
    AttrStyle style(int attributeId) const;
    QColor foreground(int attributeId) const;
    bool bold(int attributeId) const;
    void resize(int n) { m_styles.resize(n); }
    int size() const { return (int) m_styles.size(); }

    // Factory: the palette used for the built-in XML highlighter.
    static HighlightColors xmlDefault();

private:
    QVector<AttrStyle> m_styles;
};

}

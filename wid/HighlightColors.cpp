#include "HighlightColors.h"

#include "logic/XmlHighlighter.h"

namespace wid {

void HighlightColors::setStyle(int attributeId, AttrStyle style) {
    if (attributeId < 0) return;
    if (attributeId >= m_styles.size()) m_styles.resize(attributeId + 1);
    m_styles[attributeId] = style;
}

AttrStyle HighlightColors::style(int attributeId) const {
    if (attributeId < 0 || attributeId >= m_styles.size()) return {};
    return m_styles[attributeId];
}

QColor HighlightColors::foreground(int attributeId) const {
    AttrStyle s = style(attributeId);
    return s.foreground.isValid() ? s.foreground : Qt::black;
}

bool HighlightColors::bold(int attributeId) const {
    return style(attributeId).bold;
}

HighlightColors HighlightColors::xmlDefault() {
    HighlightColors p;
    p.resize(vl::XmlAttrCount);
    p.setStyle(vl::XmlDefault,    {Qt::black,              false});
    p.setStyle(vl::XmlBracket,    {Qt::black,              true});
    p.setStyle(vl::XmlElement,    {Qt::black,              true});
    p.setStyle(vl::XmlAttrName,   {QColor(0, 128, 0),      false});
    p.setStyle(vl::XmlValue,      {QColor(160, 0, 0),      false});
    p.setStyle(vl::XmlComment,    {QColor(128, 128, 128),  false});
    p.setStyle(vl::XmlDoctype,    {QColor(128, 0, 128),    false});
    p.setStyle(vl::XmlProcessing, {QColor(128, 0, 128),    false});
    return p;
}

}

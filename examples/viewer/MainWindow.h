//
// Created by Andrzej Borucki on 2022-09-13
//

#ifndef VIEWER_DIALOG_H
#define VIEWER_DIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QMainWindow>
#include <QActionGroup>
#include <QWidget>
#include <QVBoxLayout>
#include <cstdint>
#include <memory>
#include "wid/TextViewer.h"
#include "logic/XmlHighlighter.h"
#include <QFile>

class MainWindow : public QMainWindow {
Q_OBJECT
    QPushButton *button;
    QLineEdit *lineEdit;
    wid::TextViewer *widget;
    QWidget *searchBar = nullptr;
    QLineEdit *searchInput = nullptr;
    class QCheckBox *caseBox = nullptr;
    class QCheckBox *wordBox = nullptr;
    int64_t m_lastMatchStart = -1;
    int64_t m_lastMatchEnd = -1;
    QByteArray m_lastPattern;
    bool m_lastCaseInsensitive = false;
    bool m_lastWholeWord = false;
    std::unique_ptr<vl::XmlHighlighter> m_xmlHighlighter;
    void createMenus();
    void createSearchBar(QVBoxLayout *mainLayout);
    void runSearch(bool forward);
    void focusSearch();
    void hideSearch();
    void zoomBy(qreal delta);
    void askFontSize();
    void askTabWidth();
    std::unique_ptr<QFile> file;
public:
    MainWindow(QWidget *parent = nullptr);
public slots:
    void onButtonCLick();
};

#endif // VIEWER_DIALOG_H
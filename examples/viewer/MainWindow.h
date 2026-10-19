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
#include "wid/FindBar.h"
#include <qce/RulesHighlighter.h>
#include <qce/RuleBasedFoldingProvider.h>
#include <QFile>

class MainWindow : public QMainWindow {
Q_OBJECT
    QPushButton *button;
    QLineEdit *lineEdit;
    wid::TextViewer *widget;
    wid::FindBar *findBar;
    QAction *m_foldToggleAct = nullptr;
    std::unique_ptr<qce::RulesHighlighter> m_kateHighlighter;
    std::unique_ptr<qce::RuleBasedFoldingProvider> m_kateFolding;
    void createMenus();
    void zoomBy(qreal delta);
    void askFontSize();
    void askTabWidth();
    void askLoadKate();
    void askFoldMaxBytes();
    void loadKateForFile(const QString &path);
    void setKate(std::unique_ptr<qce::RulesHighlighter> hl);
    std::unique_ptr<QFile> file;
public:
    MainWindow(QWidget *parent = nullptr);
    // The find bar stops its search before the mapped file goes away.
    ~MainWindow() override { delete findBar; }
    bool openFile(const QString &path);
    bool loadKate(const QString &path);
public slots:
    void onButtonCLick();
};

#endif // VIEWER_DIALOG_H
//
// Created by Andrzej Borucki on 2022-09-13
//

#ifndef VIEWER_DIALOG_H
#define VIEWER_DIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QMainWindow>
#include <QActionGroup>
#include "wid/TextViewer.h"
#include <QFile>

class MainWindow : public QMainWindow {
Q_OBJECT
    QPushButton *button;
    QLineEdit *lineEdit;
    wid::TextViewer *widget;
    void createMenus();
    std::unique_ptr<QFile> file;
public:
    MainWindow(QWidget *parent = nullptr);
public slots:
    void onButtonCLick();
};

#endif // VIEWER_DIALOG_H

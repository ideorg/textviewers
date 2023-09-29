//
// Created by Andrzej Borucki on 2022-09-13
//

#include "MainWindow.h"

#include <QVBoxLayout>
#include "wid/TextViewer.h"
#include <QLineEdit>
#include <QPushButton>
#include <QFileDialog>
#include <QMenuBar>
#include <QActionGroup>
#include <QCoreApplication>
#include <filesystem>

namespace fs = std::filesystem;

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    QVBoxLayout *mainLayout = new QVBoxLayout;
    QHBoxLayout *hLayout = new QHBoxLayout;
    button = new QPushButton;
    lineEdit = new QLineEdit;
    connect(button, &QPushButton::pressed, this, &MainWindow::onButtonCLick);
    hLayout->addWidget(lineEdit);
    hLayout->addWidget(button);
    mainLayout->addLayout(hLayout);
    lineEdit->setText("../../../test/data/textviewer.h0");
    fs::path fspath = QCoreApplication::applicationDirPath().toStdString();
    fspath /= lineEdit->text().toStdString();
    file = std::make_unique<QFile>(canonical(fspath));
    if (!file->open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "error open file";
        return;
    }
    uchar *addr = file->map(0, file->size());
    widget = new wid::TextViewer((char *) addr, file->size(), this);
    mainLayout->addWidget(widget);
    QWidget *central = new QWidget(this);
    central->setLayout(mainLayout);
    setCentralWidget(central);
    resize(400, 400);
    createMenus();
}

void MainWindow::onButtonCLick() {
    QFileDialog dialog(this, tr("Open File"));
    fs::path fspath = QCoreApplication::applicationDirPath().toStdString();
    fspath /= lineEdit->text().toStdString();
    QString dirPath = canonical(fspath.parent_path()).c_str();
    dialog.setDirectory(dirPath);
    dialog.setOption(QFileDialog::DontUseNativeDialog);
    if (dialog.exec() == QDialog::Accepted) {
        QString path = dialog.selectedFiles().first();
        lineEdit->setText(path);
        file = std::make_unique<QFile>(lineEdit->text());
        if (!file->open(QIODevice::ReadOnly | QIODevice::Text)) {
            qDebug() << "error open file";
            return;
        }
        uchar *addr = file->map(0, file->size());
        widget->setData((char*) addr, file->size());
        file->close();
    }
}

void MainWindow::createMenus() {
    QMenu *wrapMenu = menuBar()->addMenu(tr("&Wrap"));
    auto actionGroup = new QActionGroup(wrapMenu);

    QAction *nowrapAct = wrapMenu->addAction(tr("&No wrap"));
    nowrapAct->setData(0);
    nowrapAct->setCheckable(true);
    nowrapAct->setChecked(true);
    actionGroup->addAction(nowrapAct);

    QAction *wrapAct = wrapMenu->addAction(tr("&Wrap"));
    wrapAct->setData(1);
    wrapAct->setCheckable(true);
    actionGroup->addAction(wrapAct);

    QAction *smartWrapAct = wrapMenu->addAction(tr("&Smart wrap"));
    smartWrapAct->setData(2);
    smartWrapAct->setCheckable(true);
    actionGroup->addAction(smartWrapAct);

    connect(actionGroup, &QActionGroup::triggered, this, [this](QAction *action) {
        int mode = action->data().toInt();
        widget->setWrapMode(mode);
    });

    QMenu *kindMenu = menuBar()->addMenu(tr("&Kind"));
    auto actionGroup2 = new QActionGroup(kindMenu);

    QAction *byteAct = kindMenu->addAction(tr("&Byte position"));
    byteAct->setData(0);
    byteAct->setCheckable(true);
    byteAct->setChecked(true);
    actionGroup2->addAction(byteAct);

    QAction *lineAct = kindMenu->addAction(tr("&Line index"));
    lineAct->setData(1);
    lineAct->setCheckable(true);
    actionGroup2->addAction(lineAct);

    QAction *changeableAct = kindMenu->addAction(tr("&Changeable"));
    changeableAct->setData(2);
    changeableAct->setCheckable(true);
    actionGroup2->addAction(changeableAct);

    connect(actionGroup2, &QActionGroup::triggered, this, [this](QAction *action) {
        int kind = action->data().toInt();
        widget->setKind(kind);
    });
}

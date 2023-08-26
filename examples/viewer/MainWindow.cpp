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

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    QVBoxLayout *mainLayout = new QVBoxLayout;
    QHBoxLayout *hLayout = new QHBoxLayout;
    button = new QPushButton;
    lineEdit = new QLineEdit;
    connect(button, &QPushButton::pressed, this, &MainWindow::onButtonCLick);
    hLayout->addWidget(lineEdit);
    hLayout->addWidget(button);
    mainLayout->addLayout(hLayout);
    lineEdit->setText("../../../main.cpp");
    ///vmap.openFile(lineEdit->text().toStdString().c_str());
    file = std::make_unique<QFile>(lineEdit->text());
    if (!file->open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "error open file";
        return;
    }
    uchar *addr = file->map(0, file->size());
    widget = new wid::TextViewer((char *) addr, file->size(), this);
    //file.close();
    mainLayout->addWidget(widget);
    QWidget *central = new QWidget(this);
    central->setLayout(mainLayout);
    setCentralWidget(central);
    resize(400, 400);
    createMenus();
}

void MainWindow::onButtonCLick() {
    QFileDialog dialog(this, tr("Open File"));
    QFileInfo fileInfo(lineEdit->text());
    QString dirPath = fileInfo.absolutePath();
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
    QMenu *fileMenu = menuBar()->addMenu(tr("&Wrap"));
    auto actionGroup = new QActionGroup(fileMenu);

    QAction *nowrapAct = fileMenu->addAction(tr("&No wrap"));
    nowrapAct->setData(0);
    nowrapAct->setCheckable(true);
    nowrapAct->setChecked(true);
    actionGroup->addAction(nowrapAct);

    QAction *wrapAct = fileMenu->addAction(tr("&Wrap"));
    wrapAct->setData(1);
    wrapAct->setCheckable(true);
    actionGroup->addAction(wrapAct);

    QAction *smartWrapAct = fileMenu->addAction(tr("&Smart wrap"));
    smartWrapAct->setData(2);
    smartWrapAct->setCheckable(true);
    actionGroup->addAction(smartWrapAct);

    connect(actionGroup, &QActionGroup::triggered, this, [this](QAction *action) {
        int mode = action->data().toInt();
        widget->setWrapMode(mode);
    });
}

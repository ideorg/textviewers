//
// Created by Andrzej Borucki on 2022-09-13
//

#include "MainWindow.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include "wid/TextViewer.h"
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QFileDialog>
#include <QMenuBar>
#include <QShortcut>
#include <QKeySequence>
#include <QActionGroup>
#include <QCoreApplication>
#include <QMessageBox>
#include <QProgressDialog>
#include <QTimer>
#include <QFutureWatcher>
#include <QtConcurrent/QtConcurrentRun>
#include <atomic>
#include <memory>
#include <optional>
#include <filesystem>
#include "logic/Searcher.h"
#include "logic/IByteAccess.h"

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
    mainLayout->addWidget(widget, 1);
    createSearchBar(mainLayout);
    QWidget *central = new QWidget(this);
    central->setLayout(mainLayout);
    setCentralWidget(central);
    resize(400, 400);
    createMenus();

    auto *findShortcut = new QShortcut(QKeySequence::Find, this);
    connect(findShortcut, &QShortcut::activated, this, &MainWindow::focusSearch);
    auto *nextShortcut = new QShortcut(QKeySequence(Qt::Key_F3), this);
    connect(nextShortcut, &QShortcut::activated, this, [this] { runSearch(true); });
    auto *prevShortcut = new QShortcut(QKeySequence(Qt::SHIFT | Qt::Key_F3), this);
    connect(prevShortcut, &QShortcut::activated, this, [this] { runSearch(false); });
}

void MainWindow::createSearchBar(QVBoxLayout *mainLayout) {
    searchBar = new QWidget(this);
    searchBar->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    auto *row = new QHBoxLayout(searchBar);
    row->setContentsMargins(0, 0, 0, 0);
    row->addWidget(new QLabel(tr("Find:"), searchBar));
    searchInput = new QLineEdit(searchBar);
    row->addWidget(searchInput, 1);
    auto *prevBtn = new QPushButton(tr("Prev"), searchBar);
    auto *nextBtn = new QPushButton(tr("Next"), searchBar);
    auto *closeBtn = new QPushButton(tr("Close"), searchBar);
    row->addWidget(prevBtn);
    row->addWidget(nextBtn);
    row->addWidget(closeBtn);
    mainLayout->addWidget(searchBar);
    searchBar->hide();

    connect(nextBtn, &QPushButton::clicked, this, [this] { runSearch(true); });
    connect(prevBtn, &QPushButton::clicked, this, [this] { runSearch(false); });
    connect(closeBtn, &QPushButton::clicked, this, &MainWindow::hideSearch);
    connect(searchInput, &QLineEdit::returnPressed, this, [this] { runSearch(true); });

    auto *escShortcut = new QShortcut(QKeySequence(Qt::Key_Escape), searchInput);
    escShortcut->setContext(Qt::WidgetShortcut);
    connect(escShortcut, &QShortcut::activated, this, &MainWindow::hideSearch);
}

void MainWindow::focusSearch() {
    if (!searchBar) return;
    searchBar->show();
    searchInput->setFocus();
    searchInput->selectAll();
}

void MainWindow::hideSearch() {
    if (!searchBar) return;
    searchBar->hide();
    widget->setFocus();
}

void MainWindow::runSearch(bool forward) {
    if (!searchInput) return;
    QByteArray pattern = searchInput->text().toUtf8();
    if (pattern.isEmpty()) return;
    auto *bytes = widget->byteAccess();
    if (!bytes) {
        QMessageBox::information(this, tr("Search"),
                                 tr("Search requires Byte position mode."));
        return;
    }
    if (pattern != m_lastPattern) {
        m_searchStart = forward ? bytes->firstByte() : bytes->byteCount();
        m_lastPattern = pattern;
    }

    auto cancelFlag = std::make_shared<std::atomic<bool>>(false);
    auto progressDone = std::make_shared<std::atomic<int64_t>>(0);
    auto progressTotal = std::make_shared<std::atomic<int64_t>>(0);
    int64_t startOffset = m_searchStart;
    std::string patternStr = pattern.toStdString();

    auto future = QtConcurrent::run([bytes, patternStr, startOffset, forward,
                                     cancelFlag, progressDone, progressTotal]() {
        vl::Searcher s(bytes);
        auto cb = [cancelFlag, progressDone, progressTotal](int64_t done, int64_t total) {
            progressDone->store(done, std::memory_order_relaxed);
            progressTotal->store(total, std::memory_order_relaxed);
            return !cancelFlag->load(std::memory_order_relaxed);
        };
        return forward
            ? s.findNext(patternStr, startOffset, cb)
            : s.findPrev(patternStr, startOffset, cb);
    });

    auto *watcher = new QFutureWatcher<std::optional<int64_t>>(this);
    auto *dialog = new QProgressDialog(tr("Searching..."), tr("Cancel"), 0, 1000, this);
    dialog->setWindowModality(Qt::WindowModal);
    dialog->setMinimumDuration(250);
    dialog->setValue(0);

    auto *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, dialog, [dialog, progressDone, progressTotal]() {
        int64_t d = progressDone->load(std::memory_order_relaxed);
        int64_t t = progressTotal->load(std::memory_order_relaxed);
        if (t > 0) dialog->setValue((int)(1000LL * d / t));
    });
    timer->start(50);

    connect(dialog, &QProgressDialog::canceled, this, [cancelFlag] {
        cancelFlag->store(true, std::memory_order_relaxed);
    });

    int64_t patternLen = (int64_t) pattern.size();
    connect(watcher, &QFutureWatcher<std::optional<int64_t>>::finished, this,
            [this, watcher, dialog, timer, patternLen, forward, cancelFlag]() {
                timer->stop();
                timer->deleteLater();
                auto result = watcher->result();
                bool wasCancelled = cancelFlag->load(std::memory_order_relaxed);
                dialog->reset();
                dialog->deleteLater();
                watcher->deleteLater();
                if (result) {
                    widget->showMatch(*result, patternLen);
                    m_searchStart = forward ? (*result + patternLen) : *result;
                } else if (!wasCancelled) {
                    QMessageBox::information(this, tr("Search"), tr("Not found."));
                }
            });
    watcher->setFuture(future);
}

void MainWindow::onButtonCLick() {
    QFileDialog dialog(this, tr("Open File"));
    fs::path fspath = QCoreApplication::applicationDirPath().toStdString();
    fspath /= lineEdit->text().toStdString();
    QString dirPath = QString::fromStdString(canonical(fspath.parent_path()).string());
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
        m_lastPattern.clear();
        m_searchStart = 0;
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

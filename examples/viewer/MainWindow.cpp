//
// Created by Andrzej Borucki on 2022-09-13
//

#include "MainWindow.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include "wid/TextViewer.h"
#include "wid/FindBar.h"
#include <QLineEdit>
#include <QPushButton>
#include <QFileDialog>
#include <QMenuBar>
#include <QShortcut>
#include <QKeySequence>
#include <QActionGroup>
#include <QDir>
#include <QFileInfo>
#include <QInputDialog>
#include <QMessageBox>
#include <memory>
#include <qce/kate/KatePaths.h>
#include <qce/kate/KateSyntaxIndex.h>
#include <qce/kate/KateTheme.h>
#include <qce/kate/KateXmlReader.h>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    QVBoxLayout *mainLayout = new QVBoxLayout;
    QHBoxLayout *hLayout = new QHBoxLayout;
    button = new QPushButton;
    lineEdit = new QLineEdit;
    connect(button, &QPushButton::pressed, this, &MainWindow::onButtonCLick);
    hLayout->addWidget(lineEdit);
    hLayout->addWidget(button);
    mainLayout->addLayout(hLayout);
    widget = new wid::TextViewer("", 0, this);
    findBar = new wid::FindBar(widget, this);
    mainLayout->addWidget(findBar);
    mainLayout->addWidget(widget, 1);
    QWidget *central = new QWidget(this);
    central->setLayout(mainLayout);
    setCentralWidget(central);
    resize(400, 400);
    widget->setMaxTabW(4);
    createMenus();

    auto *zoomInSc = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_Plus), this);
    connect(zoomInSc, &QShortcut::activated, this, [this] { zoomBy(1.0); });
    auto *zoomInSc2 = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_Equal), this);
    connect(zoomInSc2, &QShortcut::activated, this, [this] { zoomBy(1.0); });
    auto *zoomOutSc = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_Minus), this);
    connect(zoomOutSc, &QShortcut::activated, this, [this] { zoomBy(-1.0); });
    auto *zoomResetSc = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_0), this);
    connect(zoomResetSc, &QShortcut::activated, this, [this] { widget->setFontSize(10.5); });
}

void MainWindow::zoomBy(qreal delta) {
    qreal newSize = widget->fontSize() + delta;
    if (newSize < 1.0) newSize = 1.0;
    if (newSize > 72.0) newSize = 72.0;
    widget->setFontSize(newSize);
}

void MainWindow::askFontSize() {
    bool ok = false;
    double pt = QInputDialog::getDouble(this, tr("Font size"),
                                        tr("Point size:"),
                                        widget->fontSize(), 1.0, 72.0, 1, &ok,
                                        Qt::WindowFlags(), 0.1);
    if (ok) widget->setFontSize(pt);
}

void MainWindow::askTabWidth() {
    bool ok = false;
    int w = QInputDialog::getInt(this, tr("Tab width"),
                                 tr("Columns per tab:"),
                                 widget->maxTabW(), 1, 32, 1, &ok);
    if (ok) widget->setMaxTabW(w);
}

bool MainWindow::openFile(const QString &path) {
    auto newFile = std::make_unique<QFile>(QFileInfo(path).absoluteFilePath());
    if (!newFile->open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(this, tr("Open File"),
                             tr("Cannot open %1: %2").arg(path, newFile->errorString()));
        return false;
    }
    const char *addr = "";
    if (newFile->size() > 0) {
        addr = (const char *) newFile->map(0, newFile->size());
        if (!addr) {
            QMessageBox::warning(this, tr("Open File"),
                                 tr("Cannot map %1: %2").arg(path, newFile->errorString()));
            return false;
        }
    }
    // A search still reading the old file is stopped before it is unmapped.
    findBar->cancelSearch();
    widget->setData(addr, newFile->size());
    newFile->close();
    file = std::move(newFile);
    lineEdit->setText(file->fileName());
    loadKateForFile(file->fileName());
    return true;
}

// The Kate definition matching the file name, as the editor picks it.
void MainWindow::loadKateForFile(const QString &path) {
    widget->setFoldingProvider(nullptr);
    widget->setHighlighter(nullptr);
    m_kateFolding.reset();
    m_kateHighlighter.reset();
    auto index = qce::kate::KateSyntaxIndex::load(qce::kate::dataDir());
    auto matches = index.forFileName(path);
    if (matches.isEmpty()) return;
    auto hl = KateXmlReader::load(index.filePath(*matches.first()), KateTheme(), index);
    if (hl) setKate(std::move(hl));
}

void MainWindow::onButtonCLick() {
    QFileDialog dialog(this, tr("Open File"));
    QFileInfo current(lineEdit->text());
    dialog.setDirectory(!lineEdit->text().isEmpty() && current.absoluteDir().exists()
                            ? current.absolutePath() : QDir::homePath());
    dialog.setOption(QFileDialog::DontUseNativeDialog);
    if (dialog.exec() == QDialog::Accepted)
        openFile(dialog.selectedFiles().first());
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

    QMenu *settingsMenu = menuBar()->addMenu(tr("&Settings"));
    QAction *fontSizeAct = settingsMenu->addAction(tr("&Font size..."));
    connect(fontSizeAct, &QAction::triggered, this, &MainWindow::askFontSize);
    QAction *tabWidthAct = settingsMenu->addAction(tr("&Tab width..."));
    connect(tabWidthAct, &QAction::triggered, this, &MainWindow::askTabWidth);
    settingsMenu->addSeparator();
    QAction *zoomInAct = settingsMenu->addAction(tr("Zoom &In"));
    zoomInAct->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Plus));
    connect(zoomInAct, &QAction::triggered, this, [this] { zoomBy(1.0); });
    QAction *zoomOutAct = settingsMenu->addAction(tr("Zoom &Out"));
    zoomOutAct->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Minus));
    connect(zoomOutAct, &QAction::triggered, this, [this] { zoomBy(-1.0); });
    QAction *zoomResetAct = settingsMenu->addAction(tr("&Reset zoom"));
    zoomResetAct->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_0));
    connect(zoomResetAct, &QAction::triggered, this, [this] { widget->setFontSize(10.5); });

    QMenu *hlMenu = menuBar()->addMenu(tr("&Highlighting"));
    QAction *hlNoneAct = hlMenu->addAction(tr("&None"));
    connect(hlNoneAct, &QAction::triggered, this, [this] {
        widget->setFoldingProvider(nullptr);
        widget->setHighlighter(nullptr);
        m_kateFolding.reset();
        m_kateHighlighter.reset();
    });
    QAction *hlKateAct = hlMenu->addAction(tr("Load &Kate XML..."));
    connect(hlKateAct, &QAction::triggered, this, &MainWindow::askLoadKate);
    hlMenu->addSeparator();
    m_foldToggleAct = hlMenu->addAction(tr("&Folding enabled"));
    m_foldToggleAct->setCheckable(true);
    m_foldToggleAct->setChecked(true);
    connect(m_foldToggleAct, &QAction::toggled, this, [this](bool on) {
        if (on && m_kateFolding) widget->setFoldingProvider(m_kateFolding.get());
        else widget->setFoldingProvider(nullptr);
    });
    QAction *foldMaxAct = hlMenu->addAction(tr("Fold max &span..."));
    connect(foldMaxAct, &QAction::triggered, this, &MainWindow::askFoldMaxBytes);
}

void MainWindow::askFoldMaxBytes() {
    bool ok = false;
    int cur = (int) std::min<int64_t>(widget->foldMaxBytes(),
                                       std::numeric_limits<int>::max());
    int n = QInputDialog::getInt(this, tr("Fold max span"),
                                 tr("Max bytes per fold region:"),
                                 cur, 1, 1 << 30, 1024, &ok);
    if (ok) widget->setFoldMaxBytes((int64_t) n);
}

void MainWindow::askLoadKate() {
    QString start = qce::kate::syntaxDir();
    if (!QFile::exists(start)) start.clear();
    QString path = QFileDialog::getOpenFileName(this, tr("Load Kate syntax XML"),
                                                start, tr("Kate syntax (*.xml)"));
    if (!path.isEmpty()) loadKate(path);
}

bool MainWindow::loadKate(const QString &path) {
    auto hl = KateXmlReader::load(path);
    if (!hl) {
        QMessageBox::warning(this, tr("Load Kate XML"),
                             tr("Failed to load %1").arg(path));
        return false;
    }
    setKate(std::move(hl));
    return true;
}

void MainWindow::setKate(std::unique_ptr<qce::RulesHighlighter> hl) {
    widget->setFoldingProvider(nullptr);
    m_kateHighlighter = std::move(hl);
    m_kateFolding = std::make_unique<qce::RuleBasedFoldingProvider>(m_kateHighlighter.get());
    widget->setHighlighter(m_kateHighlighter.get());
    if (m_foldToggleAct->isChecked())
        widget->setFoldingProvider(m_kateFolding.get());
}

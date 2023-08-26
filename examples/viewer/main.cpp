//
// Created by Andrzej Borucki on 2022-09-13
//

#include "wid/TextViewer.h"

#include <QApplication>
#include "MainWindow.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    MainWindow mainWindow;
    mainWindow.show();
    return app.exec();
}

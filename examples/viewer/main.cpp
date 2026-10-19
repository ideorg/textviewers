//
// Created by Andrzej Borucki on 2022-09-13
//

#include "wid/TextViewer.h"

#include <QApplication>
#include <QCommandLineParser>
#include "MainWindow.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    QCommandLineParser parser;
    parser.setApplicationDescription(QStringLiteral("Viewer for large text files"));
    parser.addHelpOption();
    parser.addPositionalArgument(QStringLiteral("file"), QStringLiteral("File to open."));
    QCommandLineOption syntaxOpt({QStringLiteral("s"), QStringLiteral("syntax")},
                                 QStringLiteral("Kate syntax XML to highlight with."),
                                 QStringLiteral("xml"));
    parser.addOption(syntaxOpt);
    parser.process(app);

    MainWindow mainWindow;
    const QStringList args = parser.positionalArguments();
    if (!args.isEmpty()) mainWindow.openFile(args.first());
    if (parser.isSet(syntaxOpt)) mainWindow.loadKate(parser.value(syntaxOpt));
    mainWindow.show();
    return app.exec();
}

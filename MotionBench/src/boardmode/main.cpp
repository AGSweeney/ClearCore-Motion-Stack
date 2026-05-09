#include <cstdlib>
#include <QtCore/QCoreApplication>
#include <QtGui/QGuiApplication>
#include <QtGui/QIcon>
#include <QtQml/QQmlApplicationEngine>
#include <QtQml/QQmlContext>
#include <QtQuickControls2/QQuickStyle>

#include "boardmode/BoardModeToolService.h"

int main(int argc, char *argv[]) {
    QQuickStyle::setStyle("Fusion");
    QGuiApplication app(argc, argv);
    QCoreApplication::setOrganizationName("ClearCore");
    QCoreApplication::setApplicationName("BoardModeTool");
    app.setWindowIcon(QIcon(":/icons/boardmode-icon-256.png"));

    motion_bench::boardmode::BoardModeToolService service;

    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty("boardModeTool", &service);
    engine.load(QUrl(QStringLiteral("qrc:/BoardModeTool/qml/BoardModeTool.qml")));
    if (engine.rootObjects().isEmpty()) {
        return EXIT_FAILURE;
    }
    return app.exec();
}

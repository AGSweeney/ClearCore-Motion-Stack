#include <QtGui/QGuiApplication>
#include <QtGui/QIcon>
#include <QtQuickControls2/QQuickStyle>
#include <QtQml/QQmlApplicationEngine>
#include <QtQml/QQmlContext>
#include <QtCore/QDebug>
#include <cstdlib>

#include "ui/viewmodels/AppViewModel.h"

int main(int argc, char *argv[]) {
    QQuickStyle::setStyle("Fusion");
    QGuiApplication app(argc, argv);
    QCoreApplication::setOrganizationName("ClearCore");
    QCoreApplication::setApplicationName("MotionBench");
    app.setWindowIcon(QIcon(":/icons/motionbench-icon-256.png"));

    motion_bench::ui::AppViewModel view_model;

    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty("appViewModel", &view_model);

    QObject::connect(
        &engine,
        &QQmlApplicationEngine::warnings,
        [](const QList<QQmlError> &warnings) {
            for (const QQmlError &e : warnings) {
                qWarning() << "QML warning:" << e.toString();
            }
        });

    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreationFailed,
        &app,
        []() { qWarning() << "QML root object creation failed."; QCoreApplication::exit(EXIT_FAILURE); },
        Qt::QueuedConnection);

    const QUrl root_url(QStringLiteral("qrc:/MotionBench/qml/Main.qml"));
    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreated,
        &app,
        [root_url](QObject *root, const QUrl &obj_url) {
            if (root_url == obj_url && !root) {
                qWarning() << "Failed to instantiate QML component:" << obj_url;
            }
        },
        Qt::QueuedConnection);

    engine.load(root_url);
    if (engine.rootObjects().isEmpty()) {
        qWarning() << "No QML root objects; engine load failed.";
        return EXIT_FAILURE;
    }

    return app.exec();
}

/******************************************************************************
 * Copyright (c) 2026 Adam G. Sweeney
 * SPDX-License-Identifier: MIT
 *
 * Contributors:
 *   2026 Adam G. Sweeney <agsweeney@gmail.com> - MotionBench original implementation
 *
 * File: main.cpp (boardmode)
 * Purpose: Application entry for BoardModeTool; loads BoardModeTool QML module.
 *
 * Attribution: Portions of this design/implementation are influenced by
 * OpENer (Open Source EtherNet/IP Adapter Stack), where applicable.
 ******************************************************************************/

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

/******************************************************************************
 * Copyright (c) 2026 Adam G. Sweeney
 * SPDX-License-Identifier: MIT
 *
 * Contributors:
 *   2026 Adam G. Sweeney <agsweeney@gmail.com> - MotionBench original implementation
 *
 * File: AppViewModel.h
 * Purpose: Root QML view model exposing DeviceService and discovery result lists.
 *
 * Attribution: Portions of this design/implementation are influenced by
 * OpENer (Open Source EtherNet/IP Adapter Stack), where applicable.
 ******************************************************************************/

#pragma once

#include <QtCore/QObject>
#include <QtCore/QStringList>
#include <QtCore/QVariantList>

#include "device/DeviceService.h"

namespace motion_bench::ui {

class AppViewModel : public QObject {
    Q_OBJECT
    Q_PROPERTY(motion_bench::device::DeviceService *deviceService READ deviceService CONSTANT)
    Q_PROPERTY(QVariantList discoveryResults READ discoveryResults NOTIFY discoveryResultsChanged)
    Q_PROPERTY(QStringList discoveryIps READ discoveryIps NOTIFY discoveryResultsChanged)

public:
    explicit AppViewModel(QObject *parent = nullptr);

    device::DeviceService *deviceService();
    QVariantList discoveryResults() const;
    QStringList discoveryIps() const;

    Q_INVOKABLE void discover();

signals:
    void discoveryResultsChanged();

private:
    device::DeviceService device_service_;
    QVariantList discovery_results_;
};

}  // namespace motion_bench::ui

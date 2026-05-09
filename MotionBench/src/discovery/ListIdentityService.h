/******************************************************************************
 * Copyright (c) 2026 Adam G. Sweeney
 * SPDX-License-Identifier: MIT
 *
 * Contributors:
 *   2026 Adam G. Sweeney <agsweeney@gmail.com> - MotionBench original implementation
 *
 * File: ListIdentityService.h
 * Purpose: QObject wrapper for EtherNet/IP List Identity discovery.
 *
 * Attribution: Portions of this design/implementation are influenced by
 * OpENer (Open Source EtherNet/IP Adapter Stack), where applicable.
 ******************************************************************************/

#pragma once

#include <QtCore/QObject>
#include <QtCore/QVector>

#include "protocol/EtherNetIpClient.h"

namespace motion_bench::discovery {

class ListIdentityService : public QObject {
    Q_OBJECT

public:
    explicit ListIdentityService(QObject *parent = nullptr);

    QVector<protocol::ListIdentityResult> Discover(int timeout_ms = 500) const;
};

}  // namespace motion_bench::discovery

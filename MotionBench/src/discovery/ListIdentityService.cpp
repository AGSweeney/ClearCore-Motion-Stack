/******************************************************************************
 * Copyright (c) 2026 Adam G. Sweeney
 * SPDX-License-Identifier: MIT
 *
 * Contributors:
 *   2026 Adam G. Sweeney <agsweeney@gmail.com> - MotionBench original implementation
 *
 * File: ListIdentityService.cpp
 * Purpose: Forwards UDP List Identity discovery to EtherNetIpClient.
 *
 * Attribution: Portions of this design/implementation are influenced by
 * OpENer (Open Source EtherNet/IP Adapter Stack), where applicable.
 ******************************************************************************/

#include "discovery/ListIdentityService.h"

namespace motion_bench::discovery {

ListIdentityService::ListIdentityService(QObject *parent) : QObject(parent) {}

QVector<protocol::ListIdentityResult> ListIdentityService::Discover(int timeout_ms) const {
    return protocol::EtherNetIpClient::ListIdentity(timeout_ms);
}

}  // namespace motion_bench::discovery

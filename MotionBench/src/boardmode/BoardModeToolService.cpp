/******************************************************************************
 * Copyright (c) 2026 Adam G. Sweeney
 * SPDX-License-Identifier: MIT
 *
 * Contributors:
 *   2026 Adam G. Sweeney <agsweeney@gmail.com> - MotionBench original implementation
 *
 * File: BoardModeToolService.cpp
 * Purpose: Connect/discover workflow and board mode get/set over explicit messaging.
 *
 * Attribution: Portions of this design/implementation are influenced by
 * OpENer (Open Source EtherNet/IP Adapter Stack), where applicable.
 ******************************************************************************/

#include "boardmode/BoardModeToolService.h"

#include <QtCore/QSet>
#include <QtCore/QSettings>
#include <cstdint>

namespace motion_bench {
namespace boardmode {

namespace {
constexpr std::uint16_t kBoardObjectClass = 0x69;
constexpr std::uint16_t kBoardObjectInstance = 1;
constexpr std::uint16_t kBoardModeAttribute = 2;
constexpr const char kSettingsGroupConnection[] = "connection";
constexpr const char kSettingsKeyTargetIp[] = "target_ip";
}  // namespace

BoardModeToolService::BoardModeToolService(QObject *parent)
    : QObject(parent), discovery_service_(this), eip_client_(this) {
    target_ip_ = LoadTargetIpSetting();
}

QStringList BoardModeToolService::discoveredIps() const { return discovered_ips_; }
QString BoardModeToolService::targetIp() const { return target_ip_; }
bool BoardModeToolService::connected() const { return eip_client_.IsConnected(); }
bool BoardModeToolService::boardModeMConnector() const { return board_mode_mconnector_; }
QString BoardModeToolService::lastError() const { return last_error_; }

void BoardModeToolService::setTargetIp(const QString &ip) {
    const QString trimmed = ip.trimmed();
    if (trimmed.isEmpty()) {
        // Avoid clobbering persisted IP from transient empty UI states.
        return;
    }
    if (target_ip_ == trimmed) {
        return;
    }
    target_ip_ = trimmed;
    SaveTargetIpSetting(target_ip_);
    emit targetIpChanged();
}

void BoardModeToolService::discover(int timeoutMs) {
    const auto rows = discovery_service_.Discover(timeoutMs);
    QStringList next_ips;
    QSet<QString> seen;
    for (const auto &row : rows) {
        const QString ip = row.ip_address.trimmed();
        if (ip.isEmpty() || seen.contains(ip)) {
            continue;
        }
        seen.insert(ip);
        next_ips.push_back(ip);
    }
    if (discovered_ips_ != next_ips) {
        discovered_ips_ = next_ips;
        emit discoveredIpsChanged();
    }
    SetError(QString());
}

bool BoardModeToolService::connectDevice() {
    if (target_ip_.isEmpty()) {
        SetError("Target IP is empty.");
        return false;
    }
    SaveTargetIpSetting(target_ip_);
    if (!eip_client_.Connect(target_ip_)) {
        SetError("Unable to connect/register EtherNet/IP session.");
        emit connectedChanged();
        return false;
    }
    emit connectedChanged();
    return refreshBoardMode();
}

void BoardModeToolService::disconnectDevice() {
    const bool was_connected = eip_client_.IsConnected();
    eip_client_.Disconnect();
    if (was_connected) {
        emit connectedChanged();
    }
}

bool BoardModeToolService::refreshBoardMode() {
    if (!eip_client_.IsConnected()) {
        SetError("Not connected.");
        return false;
    }
    protocol::CipPath path;
    path.class_id = kBoardObjectClass;
    path.instance_id = kBoardObjectInstance;
    path.attribute_id = kBoardModeAttribute;
    const auto result = eip_client_.GetAttributeSingle(path);
    if (!result.success) {
        SetError(result.error);
        return false;
    }
    const bool next_mode = !result.data.isEmpty() && static_cast<std::uint8_t>(result.data.at(0)) != 0;
    if (board_mode_mconnector_ != next_mode) {
        board_mode_mconnector_ = next_mode;
        emit boardModeMConnectorChanged();
    }
    SetError(QString());
    return true;
}

bool BoardModeToolService::setBoardModeMConnector(bool enabled) {
    if (!eip_client_.IsConnected()) {
        SetError("Not connected.");
        return false;
    }
    protocol::CipPath path;
    path.class_id = kBoardObjectClass;
    path.instance_id = kBoardObjectInstance;
    path.attribute_id = kBoardModeAttribute;
    const QByteArray payload(1, enabled ? '\x01' : '\x00');
    const auto result = eip_client_.SetAttributeSingle(path, payload);
    if (!result.success) {
        SetError(result.error);
        return false;
    }
    if (board_mode_mconnector_ != enabled) {
        board_mode_mconnector_ = enabled;
        emit boardModeMConnectorChanged();
    }
    SetError(QString());
    return true;
}

void BoardModeToolService::SetError(const QString &error) {
    if (last_error_ == error) {
        return;
    }
    last_error_ = error;
    emit lastErrorChanged();
}

QString BoardModeToolService::LoadTargetIpSetting() const {
    QSettings settings;
    settings.beginGroup(kSettingsGroupConnection);
    const QString saved_target_ip = settings.value(kSettingsKeyTargetIp).toString().trimmed();
    settings.endGroup();
    return saved_target_ip;
}

void BoardModeToolService::SaveTargetIpSetting(const QString &target_ip) const {
    QSettings settings;
    settings.beginGroup(kSettingsGroupConnection);
    settings.setValue(kSettingsKeyTargetIp, target_ip);
    settings.endGroup();
    settings.sync();
}

}  // namespace boardmode
}  // namespace motion_bench

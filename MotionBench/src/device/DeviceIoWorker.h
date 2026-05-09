/******************************************************************************
 * Copyright (c) 2026 Adam G. Sweeney
 * SPDX-License-Identifier: MIT
 *
 * Contributors:
 *   2026 Adam G. Sweeney <agsweeney@gmail.com> - MotionBench original implementation
 *
 * File: DeviceIoWorker.h
 * Purpose: Background I/O worker for explicit reads/writes and periodic polling.
 *
 * Attribution: Portions of this design/implementation are influenced by
 * OpENer (Open Source EtherNet/IP Adapter Stack), where applicable.
 ******************************************************************************/

#pragma once

#include <QtCore/QObject>
#include <QtCore/QTimer>
#include <QtCore/QVariantMap>

#include "discovery/ListIdentityService.h"
#include "protocol/EtherNetIpClient.h"

namespace motion_bench::device {

class DeviceIoWorker : public QObject {
    Q_OBJECT

public:
    explicit DeviceIoWorker(QObject *parent = nullptr);

    Q_INVOKABLE QVariantList discover(int timeout_ms = 500);
    Q_INVOKABLE bool connectDevice(const QString &target_ip);
    Q_INVOKABLE void disconnectDevice();
    Q_INVOKABLE void startPolling();
    Q_INVOKABLE void stopPolling();
    Q_INVOKABLE bool refreshOnce();

    Q_INVOKABLE bool writeMotorVelocity(int value);
    Q_INVOKABLE bool writeMoveDistance(int value);
    Q_INVOKABLE bool writeMoveVelocity(int value);
    Q_INVOKABLE bool writeAccel(int value);
    Q_INVOKABLE bool writeDecel(int value);
    Q_INVOKABLE bool writeControlRegister(quint32 value);
    Q_INVOKABLE bool writeControlBit(int bit_index, bool enabled);
    Q_INVOKABLE bool commandPositionalMove(int move_distance);
    Q_INVOKABLE bool commandStopPositionalMove();
    Q_INVOKABLE bool writeDigitalOutput(int instance_id, bool state);
    Q_INVOKABLE bool writeCcioEnabled(bool enabled);
    Q_INVOKABLE bool writeCcioOutputBit(int board_index, int output_bit, bool enabled);
    Q_INVOKABLE bool writeMConnectorOutputBit(int bit_index, bool enabled);
    Q_INVOKABLE bool writeMConnectorTriggerPulses(int value);
    Q_INVOKABLE bool writeMConnectorPwmA(int value);
    Q_INVOKABLE bool writeMConnectorPwmB(int value);
    Q_INVOKABLE bool writeMotorConfigField(const QString &field_key, const QVariant &value);

public slots:
    void setPollIntervalMs(int interval_ms);
    void setSelectedMotorInstance(int instance_id);
    void setOperatorControlsEnabled(bool enabled);

signals:
    void connectionChanged(bool connected);
    void pollingChanged(bool polling_enabled);
    void errorChanged(const QString &error);
    void supplyVoltageUpdated(double value);
    void monitorDataUpdated(const QVariantMap &monitor_data);

private:
    bool WriteAttribute(
        const QString &map_key,
        const QVariant &value,
        std::uint16_t instance_override = 0,
        std::uint16_t attribute_override = 0);
    bool ReadAttribute(
        const QString &map_key,
        QVariant *value,
        std::uint16_t instance_override = 0,
        std::uint16_t attribute_override = 0);
    void SetError(const QString &error);
    void PollTick();

    protocol::EtherNetIpClient eip_client_;
    discovery::ListIdentityService discovery_service_;
    QTimer poll_timer_;

    QString last_error_;
    int poll_interval_ms_ = 250;
    int selected_motor_instance_ = 1;
    bool operator_controls_enabled_ = true;
    QVariantMap monitor_data_;
    double supply_voltage_ = 0.0;
    bool pending_load_position_move_ = false;
    bool pending_load_velocity_move_ = false;
};

}  // namespace motion_bench::device

/******************************************************************************
 * Copyright (c) 2026 Adam G. Sweeney
 * SPDX-License-Identifier: MIT
 *
 * Contributors:
 *   2026 Adam G. Sweeney <agsweeney@gmail.com> - MotionBench original implementation
 *
 * File: DeviceService.h
 * Purpose: QML-facing device facade; connection, polling, monitor map, operator gating.
 *
 * Attribution: Portions of this design/implementation are influenced by
 * OpENer (Open Source EtherNet/IP Adapter Stack), where applicable.
 ******************************************************************************/

#pragma once

#include <QtCore/QObject>
#include <QtCore/QThread>
#include <QtCore/QVariantList>
#include <QtCore/QVariantMap>

namespace motion_bench::device {
class DeviceIoWorker;

class DeviceService : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool connected READ connected NOTIFY connectionChanged)
    Q_PROPERTY(QString targetIp READ targetIp WRITE setTargetIp NOTIFY targetIpChanged)
    Q_PROPERTY(QString lastError READ lastError NOTIFY errorChanged)
    Q_PROPERTY(int pollIntervalMs READ pollIntervalMs WRITE setPollIntervalMs NOTIFY pollIntervalChanged)
    Q_PROPERTY(int selectedMotorInstance READ selectedMotorInstance WRITE setSelectedMotorInstance NOTIFY selectedMotorInstanceChanged)
    Q_PROPERTY(bool pollingEnabled READ pollingEnabled NOTIFY pollingChanged)
    Q_PROPERTY(bool operatorControlsEnabled READ operatorControlsEnabled WRITE setOperatorControlsEnabled NOTIFY operatorControlsEnabledChanged)
    Q_PROPERTY(double supplyVoltage READ supplyVoltage NOTIFY supplyVoltageChanged)
    Q_PROPERTY(QVariantMap monitorData READ monitorData NOTIFY monitorDataChanged)

public:
    explicit DeviceService(QObject *parent = nullptr);
    ~DeviceService() override;

    bool connected() const;
    QString targetIp() const;
    QString lastError() const;
    int pollIntervalMs() const;
    int selectedMotorInstance() const;
    bool pollingEnabled() const;
    bool operatorControlsEnabled() const;
    double supplyVoltage() const;
    QVariantMap monitorData() const;

    void setTargetIp(const QString &ip_address);
    void setPollIntervalMs(int interval_ms);
    void setSelectedMotorInstance(int instance_id);
    void setOperatorControlsEnabled(bool enabled);

    Q_INVOKABLE QVariantList discover(int timeout_ms = 500);
    Q_INVOKABLE bool connectDevice();
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

signals:
    void connectionChanged();
    void targetIpChanged();
    void errorChanged();
    void pollIntervalChanged();
    void selectedMotorInstanceChanged();
    void pollingChanged();
    void operatorControlsEnabledChanged();
    void supplyVoltageChanged();
    void monitorDataChanged();

private:
    QString LoadTargetIpSetting() const;
    void SaveTargetIpSetting(const QString &target_ip) const;
    bool LoadOperatorControlsEnabledSetting() const;
    void SaveOperatorControlsEnabledSetting(bool enabled) const;
    void SetError(const QString &error);
    bool InvokeWorkerBool(const char *method_name);
    template <typename T>
    bool InvokeWorkerBool(const char *method_name, T value);

    QThread io_thread_;
    DeviceIoWorker *io_worker_ = nullptr;

    bool connected_ = false;
    bool polling_enabled_ = false;
    QString target_ip_;
    QString last_error_;
    int poll_interval_ms_ = 250;
    int selected_motor_instance_ = 1;
    bool operator_controls_enabled_ = true;
    double supply_voltage_ = 0.0;
    QVariantMap monitor_data_;
};

}  // namespace motion_bench::device

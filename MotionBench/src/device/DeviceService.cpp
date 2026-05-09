/******************************************************************************
 * Copyright (c) 2026 Adam G. Sweeney
 * SPDX-License-Identifier: MIT
 *
 * Contributors:
 *   2026 Adam G. Sweeney <agsweeney@gmail.com> - MotionBench original implementation
 *
 * File: DeviceService.cpp
 * Purpose: Orchestrates DeviceIoWorker, settings persistence, and UI property bindings.
 *
 * Attribution: Portions of this design/implementation are influenced by
 * OpENer (Open Source EtherNet/IP Adapter Stack), where applicable.
 ******************************************************************************/

#include "device/DeviceService.h"

#include <QtCore/QMetaObject>
#include <QtCore/QSettings>
#include <algorithm>
#include <utility>

#include "device/DeviceIoWorker.h"

namespace motion_bench::device {

namespace {
constexpr const char kSettingsGroupConnection[] = "connection";
constexpr const char kSettingsKeyTargetIp[] = "target_ip";
constexpr const char kSettingsGroupOperator[] = "operator";
constexpr const char kSettingsKeyControlsEnabled[] = "controls_enabled";
}  // namespace

DeviceService::DeviceService(QObject *parent) : QObject(parent) {
    target_ip_ = LoadTargetIpSetting();
    operator_controls_enabled_ = LoadOperatorControlsEnabledSetting();

    io_worker_ = new DeviceIoWorker();
    io_worker_->moveToThread(&io_thread_);

    connect(&io_thread_, &QThread::finished, io_worker_, &QObject::deleteLater);

    connect(io_worker_, &DeviceIoWorker::connectionChanged, this, [this](bool connected_now) {
        if (connected_ == connected_now) {
            return;
        }
        connected_ = connected_now;
        emit connectionChanged();
    });

    connect(io_worker_, &DeviceIoWorker::pollingChanged, this, [this](bool polling_now) {
        if (polling_enabled_ == polling_now) {
            return;
        }
        polling_enabled_ = polling_now;
        emit pollingChanged();
    });

    connect(io_worker_, &DeviceIoWorker::errorChanged, this, [this](const QString &error) { SetError(error); });

    connect(io_worker_, &DeviceIoWorker::supplyVoltageUpdated, this, [this](double value) {
        if (qFuzzyCompare(1.0 + supply_voltage_, 1.0 + value)) {
            return;
        }
        supply_voltage_ = value;
        emit supplyVoltageChanged();
    });

    connect(io_worker_, &DeviceIoWorker::monitorDataUpdated, this, [this](const QVariantMap &next) {
        if (monitor_data_ == next) {
            return;
        }
        monitor_data_ = next;
        emit monitorDataChanged();
    });

    io_thread_.start();

    QMetaObject::invokeMethod(
        io_worker_, "setPollIntervalMs", Qt::QueuedConnection, Q_ARG(int, poll_interval_ms_));
    QMetaObject::invokeMethod(
        io_worker_, "setSelectedMotorInstance", Qt::QueuedConnection, Q_ARG(int, selected_motor_instance_));
    QMetaObject::invokeMethod(
        io_worker_, "setOperatorControlsEnabled", Qt::QueuedConnection, Q_ARG(bool, operator_controls_enabled_));
}

DeviceService::~DeviceService() {
    if (io_worker_ != nullptr && io_thread_.isRunning()) {
        QMetaObject::invokeMethod(io_worker_, "disconnectDevice", Qt::BlockingQueuedConnection);
    }
    io_thread_.quit();
    io_thread_.wait();
}

bool DeviceService::connected() const { return connected_; }
QString DeviceService::targetIp() const { return target_ip_; }
QString DeviceService::lastError() const { return last_error_; }
int DeviceService::pollIntervalMs() const { return poll_interval_ms_; }
int DeviceService::selectedMotorInstance() const { return selected_motor_instance_; }
bool DeviceService::pollingEnabled() const { return polling_enabled_; }
bool DeviceService::operatorControlsEnabled() const { return operator_controls_enabled_; }
double DeviceService::supplyVoltage() const { return supply_voltage_; }
QVariantMap DeviceService::monitorData() const { return monitor_data_; }

void DeviceService::setTargetIp(const QString &ip_address) {
    const QString trimmed_ip = ip_address.trimmed();
    if (target_ip_ == trimmed_ip) {
        return;
    }
    target_ip_ = trimmed_ip;
    SaveTargetIpSetting(target_ip_);
    emit targetIpChanged();
}

void DeviceService::setPollIntervalMs(int interval_ms) {
    const int clamped = std::clamp(interval_ms, 50, 5000);
    if (poll_interval_ms_ == clamped) {
        return;
    }
    poll_interval_ms_ = clamped;
    QMetaObject::invokeMethod(io_worker_, "setPollIntervalMs", Qt::QueuedConnection, Q_ARG(int, poll_interval_ms_));
    emit pollIntervalChanged();
}

void DeviceService::setSelectedMotorInstance(int instance_id) {
    const int clamped = std::clamp(instance_id, 1, 4);
    if (selected_motor_instance_ == clamped) {
        return;
    }
    selected_motor_instance_ = clamped;
    QMetaObject::invokeMethod(
        io_worker_, "setSelectedMotorInstance", Qt::QueuedConnection, Q_ARG(int, selected_motor_instance_));
    emit selectedMotorInstanceChanged();
}

void DeviceService::setOperatorControlsEnabled(bool enabled) {
    if (operator_controls_enabled_ == enabled) {
        return;
    }
    operator_controls_enabled_ = enabled;
    SaveOperatorControlsEnabledSetting(operator_controls_enabled_);
    QMetaObject::invokeMethod(
        io_worker_, "setOperatorControlsEnabled", Qt::QueuedConnection, Q_ARG(bool, operator_controls_enabled_));
    emit operatorControlsEnabledChanged();
}

QVariantList DeviceService::discover(int timeout_ms) {
    QVariantList rows;
    QMetaObject::invokeMethod(
        io_worker_,
        "discover",
        Qt::BlockingQueuedConnection,
        Q_RETURN_ARG(QVariantList, rows),
        Q_ARG(int, timeout_ms));
    return rows;
}

bool DeviceService::connectDevice() {
    if (target_ip_.isEmpty()) {
        SetError("Target IP is empty.");
        return false;
    }
    bool connected_ok = false;
    QMetaObject::invokeMethod(
        io_worker_,
        "connectDevice",
        Qt::BlockingQueuedConnection,
        Q_RETURN_ARG(bool, connected_ok),
        Q_ARG(QString, target_ip_));
    return connected_ok;
}

void DeviceService::disconnectDevice() {
    QMetaObject::invokeMethod(io_worker_, "disconnectDevice", Qt::QueuedConnection);
}

void DeviceService::startPolling() {
    QMetaObject::invokeMethod(io_worker_, "startPolling", Qt::QueuedConnection);
}

void DeviceService::stopPolling() {
    QMetaObject::invokeMethod(io_worker_, "stopPolling", Qt::QueuedConnection);
}

bool DeviceService::refreshOnce() { return InvokeWorkerBool("refreshOnce"); }

bool DeviceService::writeMotorVelocity(int value) { return InvokeWorkerBool("writeMotorVelocity", value); }
bool DeviceService::writeMoveDistance(int value) { return InvokeWorkerBool("writeMoveDistance", value); }
bool DeviceService::writeMoveVelocity(int value) { return InvokeWorkerBool("writeMoveVelocity", value); }
bool DeviceService::writeAccel(int value) { return InvokeWorkerBool("writeAccel", value); }
bool DeviceService::writeDecel(int value) { return InvokeWorkerBool("writeDecel", value); }
bool DeviceService::writeControlRegister(quint32 value) { return InvokeWorkerBool("writeControlRegister", value); }

bool DeviceService::writeControlBit(int bit_index, bool enabled) {
    bool write_ok = false;
    QMetaObject::invokeMethod(
        io_worker_,
        "writeControlBit",
        Qt::BlockingQueuedConnection,
        Q_RETURN_ARG(bool, write_ok),
        Q_ARG(int, bit_index),
        Q_ARG(bool, enabled));
    return write_ok;
}

bool DeviceService::commandPositionalMove(int move_distance) {
    return InvokeWorkerBool("commandPositionalMove", move_distance);
}

bool DeviceService::commandStopPositionalMove() { return InvokeWorkerBool("commandStopPositionalMove"); }

bool DeviceService::writeDigitalOutput(int instance_id, bool state) {
    bool write_ok = false;
    QMetaObject::invokeMethod(
        io_worker_,
        "writeDigitalOutput",
        Qt::BlockingQueuedConnection,
        Q_RETURN_ARG(bool, write_ok),
        Q_ARG(int, instance_id),
        Q_ARG(bool, state));
    return write_ok;
}

bool DeviceService::writeCcioEnabled(bool enabled) { return InvokeWorkerBool("writeCcioEnabled", enabled); }

bool DeviceService::writeCcioOutputBit(int board_index, int output_bit, bool enabled) {
    bool write_ok = false;
    QMetaObject::invokeMethod(
        io_worker_,
        "writeCcioOutputBit",
        Qt::BlockingQueuedConnection,
        Q_RETURN_ARG(bool, write_ok),
        Q_ARG(int, board_index),
        Q_ARG(int, output_bit),
        Q_ARG(bool, enabled));
    return write_ok;
}

bool DeviceService::writeMConnectorOutputBit(int bit_index, bool enabled) {
    bool write_ok = false;
    QMetaObject::invokeMethod(
        io_worker_,
        "writeMConnectorOutputBit",
        Qt::BlockingQueuedConnection,
        Q_RETURN_ARG(bool, write_ok),
        Q_ARG(int, bit_index),
        Q_ARG(bool, enabled));
    return write_ok;
}

bool DeviceService::writeMConnectorTriggerPulses(int value) {
    return InvokeWorkerBool("writeMConnectorTriggerPulses", value);
}

bool DeviceService::writeMConnectorPwmA(int value) {
    return InvokeWorkerBool("writeMConnectorPwmA", value);
}

bool DeviceService::writeMConnectorPwmB(int value) {
    return InvokeWorkerBool("writeMConnectorPwmB", value);
}

bool DeviceService::writeMotorConfigField(const QString &field_key, const QVariant &value) {
    bool write_ok = false;
    QMetaObject::invokeMethod(
        io_worker_,
        "writeMotorConfigField",
        Qt::BlockingQueuedConnection,
        Q_RETURN_ARG(bool, write_ok),
        Q_ARG(QString, field_key),
        Q_ARG(QVariant, value));
    return write_ok;
}

QString DeviceService::LoadTargetIpSetting() const {
    QSettings settings;
    settings.beginGroup(kSettingsGroupConnection);
    const QString saved_target_ip = settings.value(kSettingsKeyTargetIp).toString().trimmed();
    settings.endGroup();
    return saved_target_ip;
}

void DeviceService::SaveTargetIpSetting(const QString &target_ip) const {
    QSettings settings;
    settings.beginGroup(kSettingsGroupConnection);
    settings.setValue(kSettingsKeyTargetIp, target_ip);
    settings.endGroup();
}

bool DeviceService::LoadOperatorControlsEnabledSetting() const {
    QSettings settings;
    settings.beginGroup(kSettingsGroupOperator);
    const bool enabled = settings.value(kSettingsKeyControlsEnabled, true).toBool();
    settings.endGroup();
    return enabled;
}

void DeviceService::SaveOperatorControlsEnabledSetting(bool enabled) const {
    QSettings settings;
    settings.beginGroup(kSettingsGroupOperator);
    settings.setValue(kSettingsKeyControlsEnabled, enabled);
    settings.endGroup();
}

void DeviceService::SetError(const QString &error) {
    if (last_error_ == error) {
        return;
    }
    last_error_ = error;
    emit errorChanged();
}

bool DeviceService::InvokeWorkerBool(const char *method_name) {
    bool result = false;
    QMetaObject::invokeMethod(
        io_worker_, method_name, Qt::BlockingQueuedConnection, Q_RETURN_ARG(bool, result));
    return result;
}

template <typename T>
bool DeviceService::InvokeWorkerBool(const char *method_name, T value) {
    bool result = false;
    QMetaObject::invokeMethod(
        io_worker_, method_name, Qt::BlockingQueuedConnection, Q_RETURN_ARG(bool, result), Q_ARG(T, value));
    return result;
}

}  // namespace motion_bench::device

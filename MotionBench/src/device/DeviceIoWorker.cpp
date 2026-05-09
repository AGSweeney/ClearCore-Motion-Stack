/******************************************************************************
 * Copyright (c) 2026 Adam G. Sweeney
 * SPDX-License-Identifier: MIT
 *
 * Contributors:
 *   2026 Adam G. Sweeney <agsweeney@gmail.com> - MotionBench original implementation
 *
 * File: DeviceIoWorker.cpp
 * Purpose: Motor, digital output, and CCIO access via ClearLinkObjectMap and EtherNet/IP.
 *
 * Attribution: Portions of this design/implementation are influenced by
 * OpENer (Open Source EtherNet/IP Adapter Stack), where applicable.
 ******************************************************************************/

#include "device/DeviceIoWorker.h"

#include <QtCore/QByteArray>
#include <QtCore/QSet>
#include <QtCore/QVariantList>
#include <algorithm>
#include <cmath>
#include <cstring>
#include <cstdint>

#include "device/ClearLinkObjectMap.h"

namespace motion_bench::device {

namespace {

QVariantList BytesToBits(const QByteArray &bytes, int max_bits) {
    QVariantList bits;
    bits.reserve(max_bits);
    for (int bit = 0; bit < max_bits; ++bit) {
        const int byte_index = bit / 8;
        const int bit_index = bit % 8;
        bool value = false;
        if (byte_index < bytes.size()) {
            value = (static_cast<std::uint8_t>(bytes.at(byte_index)) & (1u << bit_index)) != 0;
        }
        bits.push_back(value);
    }
    return bits;
}

bool ByteArrayBit(const QByteArray &bytes, int bit) {
    if (bit < 0) {
        return false;
    }
    const int byte_index = bit / 8;
    const int bit_index = bit % 8;
    if (byte_index >= bytes.size()) {
        return false;
    }
    return (static_cast<std::uint8_t>(bytes.at(byte_index)) & (1u << bit_index)) != 0;
}

quint16 ReadLe16At(const QByteArray &bytes, int offset) {
    if (offset < 0 || (offset + 1) >= bytes.size()) {
        return 0;
    }
    const auto b0 = static_cast<std::uint8_t>(bytes.at(offset));
    const auto b1 = static_cast<std::uint8_t>(bytes.at(offset + 1));
    return static_cast<quint16>((static_cast<quint16>(b1) << 8) | static_cast<quint16>(b0));
}

float ReadLeFloatAt(const QByteArray &bytes, int offset) {
    if (offset < 0 || (offset + 3) >= bytes.size()) {
        return 0.0f;
    }
    const std::uint32_t word = static_cast<std::uint32_t>(static_cast<std::uint8_t>(bytes.at(offset)))
        | (static_cast<std::uint32_t>(static_cast<std::uint8_t>(bytes.at(offset + 1))) << 8)
        | (static_cast<std::uint32_t>(static_cast<std::uint8_t>(bytes.at(offset + 2))) << 16)
        | (static_cast<std::uint32_t>(static_cast<std::uint8_t>(bytes.at(offset + 3))) << 24);
    float out = 0.0f;
    std::memcpy(&out, &word, sizeof(out));
    return out;
}

}  // namespace

DeviceIoWorker::DeviceIoWorker(QObject *parent)
    : QObject(parent), eip_client_(this), discovery_service_(this) {
    // Ensure timer follows worker thread affinity when this object is moved.
    poll_timer_.setParent(this);
    poll_timer_.setInterval(poll_interval_ms_);
    connect(&poll_timer_, &QTimer::timeout, this, &DeviceIoWorker::PollTick);
}

void DeviceIoWorker::setPollIntervalMs(int interval_ms) {
    const int clamped = std::clamp(interval_ms, 50, 5000);
    if (poll_interval_ms_ == clamped) {
        return;
    }
    poll_interval_ms_ = clamped;
    poll_timer_.setInterval(poll_interval_ms_);
}

void DeviceIoWorker::setSelectedMotorInstance(int instance_id) {
    selected_motor_instance_ = std::clamp(instance_id, 1, 4);
}

void DeviceIoWorker::setOperatorControlsEnabled(bool enabled) {
    operator_controls_enabled_ = enabled;
    if (!operator_controls_enabled_) {
        pending_load_position_move_ = false;
        pending_load_velocity_move_ = false;
    }
}

QVariantList DeviceIoWorker::discover(int timeout_ms) {
    const auto discovered = discovery_service_.Discover(timeout_ms);
    QVariantList rows;
    QSet<QString> seen_ips;
    rows.reserve(discovered.size());
    for (const auto &item : discovered) {
        if (item.ip_address.isEmpty() || seen_ips.contains(item.ip_address)) {
            continue;
        }
        seen_ips.insert(item.ip_address);
        QVariantMap row;
        row.insert("productName", item.product_name);
        row.insert("ipAddress", item.ip_address);
        row.insert("vendorId", item.vendor_id);
        row.insert("productCode", item.product_code);
        row.insert("deviceType", item.device_type);
        row.insert("serialNumber", static_cast<qulonglong>(item.serial_number));
        rows.push_back(row);
    }
    return rows;
}

bool DeviceIoWorker::connectDevice(const QString &target_ip) {
    if (target_ip.isEmpty()) {
        SetError("Target IP is empty.");
        return false;
    }
    if (!eip_client_.Connect(target_ip)) {
        SetError("Unable to connect/register EtherNet/IP session.");
        emit connectionChanged(eip_client_.IsConnected());
        return false;
    }
    SetError(QString());
    pending_load_position_move_ = false;
    pending_load_velocity_move_ = false;
    emit connectionChanged(true);
    const bool refreshed = refreshOnce();
    if (refreshed) {
        startPolling();
    }
    return refreshed;
}

void DeviceIoWorker::disconnectDevice() {
    const bool was_connected = eip_client_.IsConnected();
    stopPolling();
    eip_client_.Disconnect();
    pending_load_position_move_ = false;
    pending_load_velocity_move_ = false;
    if (was_connected) {
        emit connectionChanged(false);
    }
}

void DeviceIoWorker::startPolling() {
    if (!eip_client_.IsConnected()) {
        SetError("Cannot start polling while disconnected.");
        return;
    }
    if (poll_timer_.isActive()) {
        return;
    }
    poll_timer_.start();
    emit pollingChanged(true);
}

void DeviceIoWorker::stopPolling() {
    if (!poll_timer_.isActive()) {
        return;
    }
    poll_timer_.stop();
    emit pollingChanged(false);
}

bool DeviceIoWorker::refreshOnce() {
    if (!eip_client_.IsConnected()) {
        SetError("Cannot refresh while disconnected.");
        return false;
    }

    QVariantMap next = monitor_data_;
    QVariant value;
    QByteArray motor_status_raw;
    QByteArray mconnector_output_assembly;
    quint32 control_value = 0;
    bool has_control_value = false;

    const bool read_serial = ReadAttribute("identity.serial_number", &value);
    next.insert("serialNumber", value);
    if (ReadAttribute("identity.status_word", &value)) {
        const quint32 status_word = value.toUInt();
        next.insert("scannerConnected", (status_word & 0x0001u) != 0u);
    }

    if (ReadAttribute("identity.mac_address", &value)) {
        const QByteArray bytes = value.toByteArray();
        QStringList octets;
        for (const auto b : bytes) {
            octets.push_back(QString("%1").arg(static_cast<std::uint8_t>(b), 2, 16, QLatin1Char('0')).toUpper());
        }
        next.insert("macAddress", octets.join(":"));
    }

    if (ReadAttribute("board.mode", &value)) {
        next.insert("boardModeMConnector", value.toBool());
    }
    if (ReadAttribute("board.supply_voltage", &value)) {
        const double next_supply_voltage = value.toDouble();
        if (std::abs(next_supply_voltage - supply_voltage_) > 1e-6) {
            supply_voltage_ = next_supply_voltage;
            emit supplyVoltageUpdated(supply_voltage_);
        }
    }

    if (ReadAttribute("ccio.board_count", &value)) {
        next.insert("ccioBoardCount", value.toInt());
    }
    if (ReadAttribute("ccio.enabled", &value)) {
        next.insert("ccioEnabled", value.toBool());
    }
    if (ReadAttribute("ccio.io_status_bits", &value)) {
        const QByteArray bits_raw = value.toByteArray();
        next.insert("ccioBits", BytesToBits(bits_raw, 64));
    }
    const QString output_assembly_key = next.value("boardModeMConnector").toBool()
        ? QStringLiteral("assembly.mconnector_output")
        : QStringLiteral("assembly.stepdir_output");
    if (ReadAttribute(output_assembly_key, &value)) {
        const QByteArray output_assembly = value.toByteArray();
        if (next.value("boardModeMConnector").toBool()) {
            mconnector_output_assembly = output_assembly;
        }
        if (output_assembly.size() >= 20) {
            next.insert("ccioOutputBits", BytesToBits(output_assembly.mid(12, 8), 64));
        }
    }
    if (!next.contains("ccioOutputBits")) {
        QVariantList output_bits;
        output_bits.reserve(64);
        for (int i = 0; i < 64; ++i) {
            output_bits.push_back(false);
        }
        next.insert("ccioOutputBits", output_bits);
    }
    if (ReadAttribute("assembly.discrete_input_bits", &value)) {
        next.insert("discreteInputs", BytesToBits(value.toByteArray(), 13));
    }

    if (ReadAttribute("motor.status_reg", &value, selected_motor_instance_)) {
        motor_status_raw = value.toByteArray();
        next.insert("motorStatusBits", BytesToBits(motor_status_raw, 32));
    }
    if (ReadAttribute("motor.alert_reg", &value, selected_motor_instance_)) {
        next.insert("motorAlertBits", BytesToBits(value.toByteArray(), 32));
    }
    if (ReadAttribute("motor.commanded_position", &value, selected_motor_instance_)) {
        next.insert("commandedPosition", value.toInt());
    }
    if (ReadAttribute("motor.commanded_velocity", &value, selected_motor_instance_)) {
        next.insert("commandedVelocity", value.toInt());
    }
    if (ReadAttribute("motor.target_position", &value, selected_motor_instance_)) {
        next.insert("targetPosition", value.toInt());
    }
    if (ReadAttribute("motor.target_velocity", &value, selected_motor_instance_)) {
        next.insert("targetVelocity", value.toInt());
    }
    if (ReadAttribute("motor.captured_position", &value, selected_motor_instance_)) {
        next.insert("capturedPosition", value.toInt());
    }
    if (ReadAttribute("motor.measured_torque", &value, selected_motor_instance_)) {
        next.insert("measuredTorque", value.toDouble());
    }
    if (ReadAttribute("motor.move_distance", &value, selected_motor_instance_)) {
        next.insert("moveDistance", value.toInt());
    }
    if (ReadAttribute("motor.velocity", &value, selected_motor_instance_)) {
        next.insert("velocity", value.toInt());
    }
    if (ReadAttribute("motor.move_velocity", &value, selected_motor_instance_)) {
        next.insert("moveVelocity", value.toInt());
    }
    if (ReadAttribute("motor.accel", &value, selected_motor_instance_)) {
        next.insert("accel", value.toInt());
    }
    if (ReadAttribute("motor.decel", &value, selected_motor_instance_)) {
        next.insert("decel", value.toInt());
    }
    if (ReadAttribute("motor.control_reg", &value, selected_motor_instance_)) {
        control_value = value.toUInt();
        has_control_value = true;
        QVariantList control_bits;
        control_bits.reserve(8);
        for (int i = 0; i < 8; ++i) {
            control_bits.push_back((control_value & (1u << i)) != 0);
        }
        next.insert("controlRegValue", control_value);
        next.insert("controlRegBits", control_bits);
    }

    if (has_control_value && !motor_status_raw.isEmpty()) {
        const bool load_position_ack = ByteArrayBit(motor_status_raw, 19);
        const bool load_velocity_ack = ByteArrayBit(motor_status_raw, 20);

        quint32 next_control_value = control_value;
        if (pending_load_position_move_ && load_position_ack) {
            next_control_value &= ~(1u << 3);
            pending_load_position_move_ = false;
        }
        if (pending_load_velocity_move_ && load_velocity_ack) {
            next_control_value &= ~(1u << 4);
            pending_load_velocity_move_ = false;
        }
        if (next_control_value != control_value) {
            writeControlRegister(next_control_value);
            next.insert("controlRegValue", next_control_value);
            QVariantList control_bits;
            control_bits.reserve(8);
            for (int i = 0; i < 8; ++i) {
                control_bits.push_back((next_control_value & (1u << i)) != 0);
            }
            next.insert("controlRegBits", control_bits);
        }
    }

    // In M-connector mode, decode per-motor fields from implicit assemblies (101 input / 113 output).
    const bool mconnector_mode = next.value("boardModeMConnector").toBool();
    if (mconnector_mode) {
        const int axis_index = std::clamp(selected_motor_instance_, 1, 4) - 1;
        const int input_hlfb_offset = 52 + axis_index * 4;     // float32 HLFB duty
        const int input_status_offset = 68 + axis_index * 2;   // uint16 status register
        const int output_motor_offset = 24 + axis_index * 8;   // 8-byte motor output block

        if (ReadAttribute("assembly.mconnector_input", &value)) {
            const QByteArray input_assembly = value.toByteArray();
            if (input_assembly.size() >= (input_status_offset + 2)) {
                next.insert("mconnectorHlfbDuty", static_cast<double>(ReadLeFloatAt(input_assembly, input_hlfb_offset)));
                const quint16 status_word = ReadLe16At(input_assembly, input_status_offset);
                next.insert("mconnectorStatusWord", static_cast<int>(status_word));
                next.insert("mconnectorStatusBits", BytesToBits(input_assembly.mid(input_status_offset, 2), 16));
            }
        }

        if (mconnector_output_assembly.size() >= (output_motor_offset + 8)) {
            next.insert("mconnectorTriggerPulses", static_cast<int>(ReadLe16At(mconnector_output_assembly, output_motor_offset)));
            next.insert("mconnectorPwmA", static_cast<int>(ReadLe16At(mconnector_output_assembly, output_motor_offset + 2)));
            next.insert("mconnectorPwmB", static_cast<int>(ReadLe16At(mconnector_output_assembly, output_motor_offset + 4)));

            const QByteArray output_reg(1, mconnector_output_assembly.at(output_motor_offset + 6));
            next.insert("mconnectorOutputReg", static_cast<int>(static_cast<std::uint8_t>(output_reg.at(0))));
            next.insert("mconnectorOutputBits", BytesToBits(output_reg, 8));

            next.insert(
                "mconnectorTriggerPulseTime",
                static_cast<int>(static_cast<std::uint8_t>(mconnector_output_assembly.at(output_motor_offset + 7))));
        }

        // Prefer explicit class 0x67 attr 1 readback for operator-control state.
        if (ReadAttribute("mconnector.output_reg", &value, selected_motor_instance_)) {
            const QByteArray raw = value.toByteArray();
            const std::uint8_t reg = raw.isEmpty() ? 0u : static_cast<std::uint8_t>(raw.at(0));
            const QByteArray reg_bytes(1, static_cast<char>(reg));
            next.insert("mconnectorOutputReg", static_cast<int>(reg));
            next.insert("mconnectorOutputBits", BytesToBits(reg_bytes, 8));
        }
        if (ReadAttribute("mconnector.trigger_pulses", &value, selected_motor_instance_)) {
            next.insert("mconnectorTriggerPulses", value.toInt());
        }
        if (ReadAttribute("mconnector.pwm_a", &value, selected_motor_instance_)) {
            next.insert("mconnectorPwmA", value.toInt());
        }
        if (ReadAttribute("mconnector.pwm_b", &value, selected_motor_instance_)) {
            next.insert("mconnectorPwmB", value.toInt());
        }
        if (ReadAttribute("mconnector_cfg.enable_connector", &value, selected_motor_instance_)) {
            next.insert("mconnectorCfgEnableConnector", value.toInt());
        }
        if (ReadAttribute("mconnector_cfg.input_a_connector", &value, selected_motor_instance_)) {
            next.insert("mconnectorCfgInputAConnector", value.toInt());
        }
        if (ReadAttribute("mconnector_cfg.input_b_connector", &value, selected_motor_instance_)) {
            next.insert("mconnectorCfgInputBConnector", value.toInt());
        }
        if (ReadAttribute("mconnector.trigger", &value, selected_motor_instance_)) {
            next.insert("mconnectorTriggerPulseTime", value.toInt());
        }
    }

    auto sin_to_str = [](int v) -> QString { return v == -1 || v == 255 ? "NA" : QString::number(v); };
    if (ReadAttribute("motor_cfg.config_reg", &value, selected_motor_instance_)) {
        const quint32 cfg = value.toUInt();
        QVariantList cfg_bits;
        cfg_bits.reserve(6);
        for (int i = 0; i < 6; ++i) {
            cfg_bits.push_back((cfg & (1u << i)) != 0);
        }
        next.insert("cfgRegBits", cfg_bits);
    }
    if (ReadAttribute("motor_cfg.soft_limit_pos1", &value, selected_motor_instance_)) next.insert("cfgSoftLimitPos1", value.toInt());
    if (ReadAttribute("motor_cfg.soft_limit_pos2", &value, selected_motor_instance_)) next.insert("cfgSoftLimitPos2", value.toInt());
    if (ReadAttribute("motor_cfg.pos_limit_input", &value, selected_motor_instance_)) next.insert("cfgPosLimitInput", sin_to_str(value.toInt()));
    if (ReadAttribute("motor_cfg.neg_limit_input", &value, selected_motor_instance_)) next.insert("cfgNegLimitInput", sin_to_str(value.toInt()));
    if (ReadAttribute("motor_cfg.home_sensor", &value, selected_motor_instance_)) next.insert("cfgHomeSensor", sin_to_str(value.toInt()));
    if (ReadAttribute("motor_cfg.brake_output", &value, selected_motor_instance_)) next.insert("cfgBrakeOutput", sin_to_str(value.toInt()));
    if (ReadAttribute("motor_cfg.pos_capture", &value, selected_motor_instance_)) next.insert("cfgPosCapture", sin_to_str(value.toInt()));
    if (ReadAttribute("motor_cfg.max_decel_rate", &value, selected_motor_instance_)) next.insert("cfgMaxDecelRate", value.toInt());
    if (ReadAttribute("motor_cfg.stop_sensor", &value, selected_motor_instance_)) next.insert("cfgStopSensor", sin_to_str(value.toInt()));
    if (ReadAttribute("motor_cfg.follow_encoder", &value, selected_motor_instance_)) next.insert("cfgFollowEncoder", sin_to_str(value.toInt()));
    if (ReadAttribute("motor_cfg.follow_divisor", &value, selected_motor_instance_)) next.insert("cfgFollowDivisor", value.toInt());
    if (ReadAttribute("motor_cfg.follow_multiplier", &value, selected_motor_instance_)) next.insert("cfgFollowMultiplier", value.toInt());

    if (read_serial) {
        if (monitor_data_ != next) {
            monitor_data_ = next;
            emit monitorDataUpdated(monitor_data_);
        }
        return true;
    }
    return false;
}

bool DeviceIoWorker::writeMotorVelocity(int value) { return WriteAttribute("motor.velocity", value, selected_motor_instance_); }
bool DeviceIoWorker::writeMoveDistance(int value) { return WriteAttribute("motor.move_distance", value, selected_motor_instance_); }
bool DeviceIoWorker::writeMoveVelocity(int value) { return WriteAttribute("motor.move_velocity", value, selected_motor_instance_); }
bool DeviceIoWorker::writeAccel(int value) { return WriteAttribute("motor.accel", value, selected_motor_instance_); }
bool DeviceIoWorker::writeDecel(int value) { return WriteAttribute("motor.decel", value, selected_motor_instance_); }
bool DeviceIoWorker::writeControlRegister(quint32 value) { return WriteAttribute("motor.control_reg", value, selected_motor_instance_); }

bool DeviceIoWorker::writeControlBit(int bit_index, bool enabled) {
    if (bit_index < 0 || bit_index > 31) {
        SetError("Control bit index must be in range 0..31.");
        return false;
    }

    QVariant current_value;
    if (!ReadAttribute("motor.control_reg", &current_value, selected_motor_instance_)) {
        return false;
    }

    quint32 mask = current_value.toUInt();
    const quint32 bit_mask = (1u << bit_index);
    if (enabled) {
        mask |= bit_mask;
    } else {
        mask &= ~bit_mask;
    }
    if (bit_index == 3) {
        pending_load_position_move_ = enabled;
    } else if (bit_index == 4) {
        pending_load_velocity_move_ = enabled;
    }
    return writeControlRegister(mask);
}

bool DeviceIoWorker::commandPositionalMove(int move_distance) {
    if (!WriteAttribute("motor.move_distance", move_distance, selected_motor_instance_)) {
        return false;
    }
    pending_load_position_move_ = true;
    return writeControlBit(3, true);
}

bool DeviceIoWorker::commandStopPositionalMove() {
    if (!WriteAttribute("motor.move_distance", 0, selected_motor_instance_)) {
        return false;
    }
    return writeControlBit(5, true);
}

bool DeviceIoWorker::writeDigitalOutput(int instance_id, bool state) {
    return WriteAttribute("digital_output.state", state, instance_id, 3);
}

bool DeviceIoWorker::writeCcioEnabled(bool enabled) {
    if (!WriteAttribute("ccio.enabled", enabled)) {
        return false;
    }

    monitor_data_.insert("ccioEnabled", enabled);
    emit monitorDataUpdated(monitor_data_);
    return true;
}

bool DeviceIoWorker::writeCcioOutputBit(int board_index, int output_bit, bool enabled) {
    if (board_index < 1 || board_index > 8 || output_bit < 0 || output_bit > 7) {
        SetError("CCIO output index out of range.");
        return false;
    }

    const bool board_mode_mconnector = monitor_data_.value("boardModeMConnector").toBool();
    const QString output_assembly_key = board_mode_mconnector
        ? QStringLiteral("assembly.mconnector_output")
        : QStringLiteral("assembly.stepdir_output");
    const int output_assembly_size = board_mode_mconnector ? 200 : 280;

    QByteArray output_raw(8, '\0');
    QVariantList current_bits = monitor_data_.value("ccioOutputBits").toList();
    for (int bit = 0; bit < 64 && bit < current_bits.size(); ++bit) {
        if (!current_bits.at(bit).toBool()) {
            continue;
        }
        const int byte_index = bit / 8;
        const int bit_index = bit % 8;
        output_raw[byte_index] = static_cast<char>(
            static_cast<std::uint8_t>(output_raw.at(byte_index)) | (1u << bit_index));
    }

    const int bit = (board_index - 1) * 8 + output_bit;
    const int byte_index = bit / 8;
    const int bit_index = bit % 8;
    std::uint8_t byte = static_cast<std::uint8_t>(output_raw.at(byte_index));
    if (enabled) {
        byte |= (1u << bit_index);
    } else {
        byte &= static_cast<std::uint8_t>(~(1u << bit_index));
    }
    output_raw[byte_index] = static_cast<char>(byte);

    QByteArray output_assembly(output_assembly_size, '\0');
    for (int i = 0; i < 8; ++i) {
        output_assembly[12 + i] = output_raw.at(i);
    }

    if (!WriteAttribute(output_assembly_key, output_assembly)) {
        return false;
    }

    monitor_data_.insert("ccioOutputBits", BytesToBits(output_raw, 64));
    emit monitorDataUpdated(monitor_data_);
    return true;
}

bool DeviceIoWorker::writeMConnectorOutputBit(int bit_index, bool enabled) {
    if (bit_index < 0 || bit_index > 7) {
        SetError("M-connector output bit index must be in range 0..7.");
        return false;
    }

    QVariant current_reg_value;
    if (!ReadAttribute("mconnector.output_reg", &current_reg_value, selected_motor_instance_)) {
        return false;
    }
    const QByteArray current_raw = current_reg_value.toByteArray();
    std::uint8_t output_reg = current_raw.isEmpty() ? 0u : static_cast<std::uint8_t>(current_raw.at(0));

    if (enabled) {
        output_reg = static_cast<std::uint8_t>(output_reg | (1u << bit_index));
    } else {
        output_reg = static_cast<std::uint8_t>(output_reg & ~(1u << bit_index));
    }
    const QByteArray payload(1, static_cast<char>(output_reg));
    if (!WriteAttribute("mconnector.output_reg", payload, selected_motor_instance_)) {
        return false;
    }
    monitor_data_.insert("mconnectorOutputReg", static_cast<int>(output_reg));
    monitor_data_.insert("mconnectorOutputBits", BytesToBits(payload, 8));
    emit monitorDataUpdated(monitor_data_);
    return true;
}

bool DeviceIoWorker::writeMConnectorTriggerPulses(int value) {
    const int clamped = std::clamp(value, 0, 65535);
    if (!WriteAttribute("mconnector.trigger_pulses", static_cast<quint32>(clamped), selected_motor_instance_)) {
        return false;
    }
    monitor_data_.insert("mconnectorTriggerPulses", clamped);
    emit monitorDataUpdated(monitor_data_);
    return true;
}

bool DeviceIoWorker::writeMConnectorPwmA(int value) {
    const int clamped = std::clamp(value, 0, 4095);
    QVariant current_reg_value;
    if (!ReadAttribute("mconnector.output_reg", &current_reg_value, selected_motor_instance_)) {
        return false;
    }
    const QByteArray current_raw = current_reg_value.toByteArray();
    std::uint8_t output_reg = current_raw.isEmpty() ? 0u : static_cast<std::uint8_t>(current_raw.at(0));
    if (clamped > 0) {
        output_reg = static_cast<std::uint8_t>(output_reg | (1u << 1));
    } else {
        output_reg = static_cast<std::uint8_t>(output_reg & ~(1u << 1));
    }
    const QByteArray output_payload(1, static_cast<char>(output_reg));
    if (!WriteAttribute("mconnector.output_reg", output_payload, selected_motor_instance_)) {
        return false;
    }
    if (!WriteAttribute("mconnector.pwm_a", static_cast<quint32>(clamped), selected_motor_instance_)) {
        return false;
    }
    monitor_data_.insert("mconnectorOutputReg", static_cast<int>(output_reg));
    monitor_data_.insert("mconnectorOutputBits", BytesToBits(output_payload, 8));
    monitor_data_.insert("mconnectorPwmA", clamped);
    emit monitorDataUpdated(monitor_data_);
    return true;
}

bool DeviceIoWorker::writeMConnectorPwmB(int value) {
    const int clamped = std::clamp(value, 0, 4095);
    QVariant current_reg_value;
    if (!ReadAttribute("mconnector.output_reg", &current_reg_value, selected_motor_instance_)) {
        return false;
    }
    const QByteArray current_raw = current_reg_value.toByteArray();
    std::uint8_t output_reg = current_raw.isEmpty() ? 0u : static_cast<std::uint8_t>(current_raw.at(0));
    if (clamped > 0) {
        output_reg = static_cast<std::uint8_t>(output_reg | (1u << 2));
    } else {
        output_reg = static_cast<std::uint8_t>(output_reg & ~(1u << 2));
    }
    const QByteArray output_payload(1, static_cast<char>(output_reg));
    if (!WriteAttribute("mconnector.output_reg", output_payload, selected_motor_instance_)) {
        return false;
    }
    if (!WriteAttribute("mconnector.pwm_b", static_cast<quint32>(clamped), selected_motor_instance_)) {
        return false;
    }
    monitor_data_.insert("mconnectorOutputReg", static_cast<int>(output_reg));
    monitor_data_.insert("mconnectorOutputBits", BytesToBits(output_payload, 8));
    monitor_data_.insert("mconnectorPwmB", clamped);
    emit monitorDataUpdated(monitor_data_);
    return true;
}

bool DeviceIoWorker::writeMotorConfigField(const QString &field_key, const QVariant &value) {
    static const QSet<QString> kWritableMotorConfigFields = {
        "motor_cfg.config_reg",
        "motor_cfg.soft_limit_pos1",
        "motor_cfg.soft_limit_pos2",
        "motor_cfg.pos_limit_input",
        "motor_cfg.neg_limit_input",
        "motor_cfg.home_sensor",
        "motor_cfg.brake_output",
        "motor_cfg.pos_capture",
        "motor_cfg.max_decel_rate",
        "motor_cfg.stop_sensor",
        "motor_cfg.follow_encoder",
        "motor_cfg.follow_divisor",
        "motor_cfg.follow_multiplier",
        "mconnector_cfg.enable_connector",
        "mconnector_cfg.input_a_connector",
        "mconnector_cfg.input_b_connector",
        "mconnector_cfg.trigger_pulse_time_ms",
    };
    if (!kWritableMotorConfigFields.contains(field_key)) {
        SetError(QString("Unsupported motor config key: %1").arg(field_key));
        return false;
    }
    return WriteAttribute(field_key, value, selected_motor_instance_);
}

bool DeviceIoWorker::WriteAttribute(
    const QString &map_key,
    const QVariant &value,
    std::uint16_t instance_override,
    std::uint16_t attribute_override) {
    if (monitor_data_.value("scannerConnected").toBool()) {
        SetError("Write failed: external EtherNet/IP scanner owns I/O.");
        return false;
    }
    if (!operator_controls_enabled_) {
        SetError("Operator controls are disabled; write blocked.");
        return false;
    }
    if (!eip_client_.IsConnected()) {
        SetError("Write failed: device is disconnected.");
        return false;
    }
    const auto map = ClearLinkObjectMap::Attributes();
    const auto it = map.find(map_key);
    if (it == map.end()) {
        SetError(QString("Unknown map key: %1").arg(map_key));
        return false;
    }
    if (!it->writable) {
        SetError(QString("Attribute is read-only: %1").arg(map_key));
        return false;
    }

    protocol::CipPath path = it->path;
    if (instance_override != 0) {
        path.instance_id = instance_override;
    }
    if (attribute_override != 0) {
        path.attribute_id = attribute_override;
    }
    const QByteArray payload = ClearLinkObjectMap::EncodeValue(value, it->data_type);
    const auto result = eip_client_.SetAttributeSingle(path, payload);
    if (!result.success) {
        SetError(result.error);
        return false;
    }
    SetError(QString());
    return true;
}

bool DeviceIoWorker::ReadAttribute(
    const QString &map_key,
    QVariant *value,
    std::uint16_t instance_override,
    std::uint16_t attribute_override) {
    const auto map = ClearLinkObjectMap::Attributes();
    const auto it = map.find(map_key);
    if (it == map.end()) {
        SetError(QString("Unknown map key: %1").arg(map_key));
        return false;
    }

    protocol::CipPath path = it->path;
    if (instance_override != 0) {
        path.instance_id = instance_override;
    }
    if (attribute_override != 0) {
        path.attribute_id = attribute_override;
    }

    const auto result = eip_client_.GetAttributeSingle(path);
    if (!result.success) {
        SetError(result.error);
        return false;
    }
    *value = ClearLinkObjectMap::DecodeValue(result.data, it->data_type);
    SetError(QString());
    return true;
}

void DeviceIoWorker::SetError(const QString &error) {
    if (last_error_ == error) {
        return;
    }
    last_error_ = error;
    emit errorChanged(last_error_);
}

void DeviceIoWorker::PollTick() {
    refreshOnce();
}

}  // namespace motion_bench::device

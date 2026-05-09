#include "device/ClearLinkObjectMap.h"

#include <QtEndian>
#include <cstring>

namespace motion_bench::device {

namespace {

ObjectAttribute Attr(
    const char *key,
    const char *description,
    std::uint16_t class_id,
    std::uint16_t instance_id,
    std::uint16_t attribute_id,
    DataType data_type,
    bool writable) {
    ObjectAttribute attribute;
    attribute.key = QString::fromLatin1(key);
    attribute.description = QString::fromLatin1(description);
    attribute.path.class_id = class_id;
    attribute.path.instance_id = instance_id;
    attribute.path.attribute_id = attribute_id;
    attribute.data_type = data_type;
    attribute.writable = writable;
    return attribute;
}

template <typename T>
QByteArray ToLittleEndianBytes(T value) {
    QByteArray bytes(sizeof(T), Qt::Uninitialized);
    qToLittleEndian<T>(value, reinterpret_cast<uchar *>(bytes.data()));
    return bytes;
}

template <typename T>
T FromLittleEndianBytes(const QByteArray &raw, T fallback) {
    if (raw.size() < static_cast<int>(sizeof(T))) {
        return fallback;
    }
    return qFromLittleEndian<T>(reinterpret_cast<const uchar *>(raw.constData()));
}

}  // namespace

const QHash<QString, ObjectAttribute> &ClearLinkObjectMap::Attributes() {
    static const QHash<QString, ObjectAttribute> kAttributes = {
        // Identity and board metadata.
        {"identity.serial_number", Attr("identity.serial_number", "Identity serial number", 0x01, 1, 6, DataType::kInt32, false)},
        {"identity.status_word", Attr("identity.status_word", "Identity status word (Owned bit indicates active scanner I/O owner)", 0x01, 1, 5, DataType::kUint16, false)},
        {"identity.mac_address", Attr("identity.mac_address", "Ethernet MAC address bytes", 0xF6, 1, 3, DataType::kByteArray, false)},
        {"board.mode", Attr("board.mode", "Board mode (Step/Dir false, M-connector true)", 0x69, 1, 2, DataType::kBool, false)},
        {"board.supply_voltage", Attr("board.supply_voltage", "Supply voltage float", 0x69, 1, 5, DataType::kFloat32, false)},

        // Discrete input assembly pattern from legacy monitor.
        {"assembly.discrete_input_bits", Attr("assembly.discrete_input_bits", "Discrete input bits", 4, 100, 3, DataType::kByteArray, false)},
        {"assembly.mconnector_input", Attr("assembly.mconnector_input", "M-connector T2O input assembly image", 4, 101, 3, DataType::kByteArray, false)},
        {"assembly.stepdir_output", Attr("assembly.stepdir_output", "Step/Dir O2T output assembly image", 4, 112, 3, DataType::kByteArray, true)},
        {"assembly.mconnector_output", Attr("assembly.mconnector_output", "M-connector O2T output assembly image", 4, 113, 3, DataType::kByteArray, true)},

        // CCIO group.
        {"ccio.output_value", Attr("ccio.output_value", "CCIO output bitfield for up to 8 boards", 0x68, 1, 1, DataType::kByteArray, true)},
        {"ccio.io_status_bits", Attr("ccio.io_status_bits", "CCIO bitfield status for up to 8 boards", 0x68, 1, 2, DataType::kByteArray, false)},
        {"ccio.board_count", Attr("ccio.board_count", "Attached CCIO board count", 0x68, 1, 4, DataType::kSint, false)},
        {"ccio.enabled", Attr("ccio.enabled", "CCIO enable state bitfield", 0x68, 1, 6, DataType::kBool, true)},

        // Motor status and data (instance selected per motor 1..4).
        {"motor.commanded_position", Attr("motor.commanded_position", "Commanded position", 0x65, 1, 1, DataType::kInt32, false)},
        {"motor.commanded_velocity", Attr("motor.commanded_velocity", "Commanded velocity", 0x65, 1, 2, DataType::kInt32, false)},
        {"motor.target_position", Attr("motor.target_position", "Target position", 0x65, 1, 3, DataType::kInt32, false)},
        {"motor.target_velocity", Attr("motor.target_velocity", "Target velocity", 0x65, 1, 4, DataType::kInt32, false)},
        {"motor.captured_position", Attr("motor.captured_position", "Captured position", 0x65, 1, 5, DataType::kInt32, false)},
        {"motor.measured_torque", Attr("motor.measured_torque", "Measured torque", 0x65, 1, 6, DataType::kFloat32, false)},
        {"motor.status_reg", Attr("motor.status_reg", "Status register bits", 0x65, 1, 7, DataType::kByteArray, false)},
        {"motor.alert_reg", Attr("motor.alert_reg", "Alert register bits", 0x65, 1, 8, DataType::kByteArray, false)},

        // Motor command object.
        {"motor.move_distance", Attr("motor.move_distance", "Move distance", 0x66, 1, 1, DataType::kInt32, true)},
        {"motor.velocity", Attr("motor.velocity", "Velocity", 0x66, 1, 2, DataType::kInt32, true)},
        {"motor.move_velocity", Attr("motor.move_velocity", "Move velocity", 0x66, 1, 3, DataType::kInt32, true)},
        {"motor.accel", Attr("motor.accel", "Acceleration", 0x66, 1, 4, DataType::kInt32, true)},
        {"motor.decel", Attr("motor.decel", "Deceleration", 0x66, 1, 5, DataType::kInt32, true)},
        {"motor.control_reg", Attr("motor.control_reg", "Control register bits", 0x66, 1, 6, DataType::kUint32, true)},
        {"motor.add_to_position", Attr("motor.add_to_position", "Add to position", 0x66, 1, 7, DataType::kInt32, true)},

        // M-connector object.
        {"mconnector.output_reg", Attr("mconnector.output_reg", "M-connector output register", 0x67, 1, 1, DataType::kByteArray, true)},
        {"mconnector.status_reg", Attr("mconnector.status_reg", "M-connector status register", 0x67, 1, 2, DataType::kByteArray, false)},
        {"mconnector.measured_torque", Attr("mconnector.measured_torque", "M-connector measured torque", 0x67, 1, 6, DataType::kFloat32, false)},
        {"mconnector.trigger_pulses", Attr("mconnector.trigger_pulses", "M-connector trigger pulses command", 0x67, 1, 7, DataType::kUint16, true)},
        {"mconnector.pwm_a", Attr("mconnector.pwm_a", "M-connector PWM A", 0x67, 1, 8, DataType::kUint16, true)},
        {"mconnector.pwm_b", Attr("mconnector.pwm_b", "M-connector PWM B", 0x67, 1, 9, DataType::kUint16, true)},
        {"mconnector.trigger", Attr("mconnector.trigger", "M-connector trigger pulse time (ms)", 0x67, 1, 10, DataType::kUint16, false)},
        {"mconnector_cfg.enable_connector", Attr("mconnector_cfg.enable_connector", "M-connector enable connector number", 0x67, 1, 3, DataType::kSint, true)},
        {"mconnector_cfg.input_a_connector", Attr("mconnector_cfg.input_a_connector", "M-connector input A connector number", 0x67, 1, 4, DataType::kSint, true)},
        {"mconnector_cfg.input_b_connector", Attr("mconnector_cfg.input_b_connector", "M-connector input B connector number", 0x67, 1, 5, DataType::kSint, true)},
        {"mconnector_cfg.trigger_pulse_time_ms", Attr("mconnector_cfg.trigger_pulse_time_ms", "M-connector enable pulse time ms", 0x67, 1, 10, DataType::kUint16, true)},

        // Motor configuration object (class 0x64, instance per motor 1..4).
        {"motor_cfg.config_reg",       Attr("motor_cfg.config_reg",       "Config register bits 0-5", 0x64, 1, 1,  DataType::kUint32, true)},
        {"motor_cfg.soft_limit_pos1",  Attr("motor_cfg.soft_limit_pos1",  "Soft limit position 1",   0x64, 1, 2,  DataType::kInt32,  true)},
        {"motor_cfg.soft_limit_pos2",  Attr("motor_cfg.soft_limit_pos2",  "Soft limit position 2",   0x64, 1, 3,  DataType::kInt32,  true)},
        {"motor_cfg.pos_limit_input",  Attr("motor_cfg.pos_limit_input",  "Positive limit connector",0x64, 1, 4,  DataType::kSint,   true)},
        {"motor_cfg.neg_limit_input",  Attr("motor_cfg.neg_limit_input",  "Negative limit connector",0x64, 1, 5,  DataType::kSint,   true)},
        {"motor_cfg.home_sensor",      Attr("motor_cfg.home_sensor",      "Home sensor connector",   0x64, 1, 6,  DataType::kSint,   true)},
        {"motor_cfg.brake_output",     Attr("motor_cfg.brake_output",     "Brake output connector",  0x64, 1, 7,  DataType::kSint,   true)},
        {"motor_cfg.pos_capture",      Attr("motor_cfg.pos_capture",      "Position capture sensor", 0x64, 1, 8,  DataType::kSint,   true)},
        {"motor_cfg.max_decel_rate",   Attr("motor_cfg.max_decel_rate",   "Max deceleration rate",   0x64, 1, 9,  DataType::kInt32,  true)},
        {"motor_cfg.stop_sensor",      Attr("motor_cfg.stop_sensor",      "Stop sensor connector",   0x64, 1, 10, DataType::kSint,   true)},
        {"motor_cfg.follow_encoder",   Attr("motor_cfg.follow_encoder",   "Follow encoder/axis",     0x64, 1, 11, DataType::kSint,   true)},
        {"motor_cfg.follow_divisor",   Attr("motor_cfg.follow_divisor",   "Follow divisor",          0x64, 1, 12, DataType::kInt32,  true)},
        {"motor_cfg.follow_multiplier",Attr("motor_cfg.follow_multiplier","Follow multiplier",       0x64, 1, 13, DataType::kInt32,  true)},

        // Digital outputs object (instance is line number 1..6 in legacy app).
        {"digital_output.state", Attr("digital_output.state", "Digital output state bit", 0x09, 1, 3, DataType::kBool, true)},
    };
    return kAttributes;
}

QVariant ClearLinkObjectMap::DecodeValue(const QByteArray &raw, DataType data_type) {
    switch (data_type) {
        case DataType::kBool:
            return !raw.isEmpty() && static_cast<std::uint8_t>(raw.at(0)) != 0;
        case DataType::kSint:
            return raw.isEmpty() ? 0 : static_cast<qint8>(raw.at(0));
        case DataType::kUint16:
            return static_cast<quint16>(FromLittleEndianBytes<std::uint16_t>(raw, 0));
        case DataType::kInt16:
            return static_cast<qint16>(FromLittleEndianBytes<std::int16_t>(raw, 0));
        case DataType::kUint32:
            return static_cast<quint32>(FromLittleEndianBytes<std::uint32_t>(raw, 0));
        case DataType::kInt32:
            return static_cast<qint32>(FromLittleEndianBytes<std::int32_t>(raw, 0));
        case DataType::kFloat32: {
            const std::uint32_t bits = FromLittleEndianBytes<std::uint32_t>(raw, 0);
            float value = 0.0f;
            std::memcpy(&value, &bits, sizeof(value));
            return value;
        }
        case DataType::kByteArray:
        default:
            return raw;
    }
}

QByteArray ClearLinkObjectMap::EncodeValue(const QVariant &value, DataType data_type) {
    switch (data_type) {
        case DataType::kBool:
            return QByteArray(1, value.toBool() ? '\x01' : '\x00');
        case DataType::kSint:
            return QByteArray(1, static_cast<char>(value.toInt()));
        case DataType::kUint16:
            return ToLittleEndianBytes<std::uint16_t>(static_cast<std::uint16_t>(value.toUInt()));
        case DataType::kInt16:
            return ToLittleEndianBytes<std::int16_t>(static_cast<std::int16_t>(value.toInt()));
        case DataType::kUint32:
            return ToLittleEndianBytes<std::uint32_t>(static_cast<std::uint32_t>(value.toUInt()));
        case DataType::kInt32:
            return ToLittleEndianBytes<std::int32_t>(static_cast<std::int32_t>(value.toInt()));
        case DataType::kFloat32: {
            const float f_value = value.toFloat();
            std::uint32_t bits = 0;
            std::memcpy(&bits, &f_value, sizeof(bits));
            return ToLittleEndianBytes<std::uint32_t>(bits);
        }
        case DataType::kByteArray:
        default:
            return value.toByteArray();
    }
}

}  // namespace motion_bench::device

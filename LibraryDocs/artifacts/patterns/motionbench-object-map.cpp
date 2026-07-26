// EXCERPT — source: MotionBench/src/device/ClearLinkObjectMap.cpp
// EVIDENCE: E1 | symbol: board.mode | lines: 60-85
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

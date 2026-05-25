#ifndef ETHERCAT_SLAVE_PDO_LAYOUT_H_
#define ETHERCAT_SLAVE_PDO_LAYOUT_H_

#include <stdint.h>

namespace ethercat_slave {

#pragma pack(push, 1)

/**
 * @brief Fixed command image consumed from EtherCAT transport.
 */
struct EthercatPdoCommand {
    uint16_t control_word;  // bits 0..5 output levels for IO0..IO5
    int8_t mode_of_operation;
    uint8_t reserved0;  // bits 0..3: A9..A12 analog mode select (1=analog, 0=digital)
    int32_t target_position_counts;
    int32_t target_velocity_counts_per_sec;
    int16_t target_torque_tenths_percent;
    uint16_t reserved1;  // bits 0..5 IO0..IO5 direction request, bit 15 direction-valid flag
    uint32_t sequence;
    uint16_t ccio_control;  // bit 0: enable CCIO on COM-1
    uint16_t ccio_reserved;
    uint64_t ccio_output_bits;  // CCIOA0..CCIOH7 requested output levels
    uint64_t ccio_direction_bits;  // CCIOA0..CCIOH7 direction request, 1=output
};

/**
 * @brief Fixed status image produced to EtherCAT transport.
 */
struct EthercatPdoStatus {
    uint16_t status_word;  // bits 0..5 IO0..IO5 live state
    int8_t mode_of_operation_display;  // bits 0..3: applied A9..A12 analog mode mask
    uint8_t fault_code;  // reserved for faults in this personality
    int32_t actual_position_counts;  // lo16 A9 raw, hi16 A10 raw
    int32_t actual_velocity_counts_per_sec;  // lo16 A11 raw, hi16 A12 raw
    int16_t actual_torque_tenths_percent;  // supply voltage in centivolts for this IO personality
    uint16_t reserved0;  // bits 0..2 DI6..DI8 state, bits 3..6 A9..A12 digital state, bits 8..13 IO0..IO5 applied direction
    uint32_t sequence_ack;
    uint16_t ccio_status;  // bit 0: enabled, bit 1: link broken, bit 2: any overload
    uint8_t ccio_board_count;
    uint8_t ccio_reserved;
    uint64_t ccio_input_bits;  // CCIOA0..CCIOH7 live input image
    uint64_t ccio_status_bits;  // overload bits 0..63, bit 63 also indicates link broken
    uint64_t ccio_direction_bits;  // CCIOA0..CCIOH7 applied direction, 1=output
    uint32_t firmware_loop_period_us;
    uint32_t firmware_loop_runtime_us;
    uint32_t firmware_loop_jitter_us;
    uint32_t firmware_transport_us;
};

#pragma pack(pop)

static_assert(sizeof(EthercatPdoCommand) == 40U,
              "EthercatPdoCommand layout must remain fixed.");
static_assert(sizeof(EthercatPdoStatus) == 64U,
              "EthercatPdoStatus layout must remain fixed.");

}  // namespace ethercat_slave

#endif  // ETHERCAT_SLAVE_PDO_LAYOUT_H_

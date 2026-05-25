/*
 * EtherCAT Slave Personality Firmware -- Main Entry Point
 *
 * Copyright (c) 2026 Adam G. Sweeney <agsweeney@gmail.com>
 * SPDX-License-Identifier: MIT
 */

#include "ClearCore.h"
#include "lwip/ip4_addr.h"
#include "lwip/netif.h"
#include "protocol/ethercat_slave/ethercat_slave.h"

#include <stdint.h>
#include <stdio.h>

namespace {

static const uint8_t kIoCount = 6U;
static const uint16_t kIoBitMask = 0x003FU;
static const uint8_t kAnalogModeMask = 0x0FU;
static const uint16_t kDirectionValidFlag = 0x8000U;
static const uint16_t kCcioControlEnable = 0x0001U;
static const uint16_t kCcioStatusEnabled = 0x0001U;
static const uint16_t kCcioStatusLinkBroken = 0x0002U;
static const uint16_t kCcioStatusAnyOverload = 0x0004U;
static const uint32_t kExpectedLoopPeriodUs = 1000UL;

static const uint8_t kDi6StatusBit = 0U;
static const uint8_t kDi7StatusBit = 1U;
static const uint8_t kDi8StatusBit = 2U;
static const uint8_t kA9DigitalStatusBit = 3U;
static const uint8_t kA10DigitalStatusBit = 4U;
static const uint8_t kA11DigitalStatusBit = 5U;
static const uint8_t kA12DigitalStatusBit = 6U;

ClearCorePins CcioPinFromBitIndex(uint8_t bit_index) {
    const uint16_t pin_index =
        static_cast<uint16_t>(CLEARCORE_PIN_CCIOA0) + static_cast<uint16_t>(bit_index);
    if (pin_index >= static_cast<uint16_t>(CLEARCORE_PIN_CCIO_MAX)) {
        return CLEARCORE_PIN_CCIOA0;
    }
    return static_cast<ClearCorePins>(pin_index);
}

void ConfigureIoDefaults() {
    ConnectorIO0.Mode(Connector::INPUT_DIGITAL);
    ConnectorIO1.Mode(Connector::INPUT_DIGITAL);
    ConnectorIO2.Mode(Connector::INPUT_DIGITAL);
    ConnectorIO3.Mode(Connector::INPUT_DIGITAL);
    ConnectorIO4.Mode(Connector::INPUT_DIGITAL);
    ConnectorIO5.Mode(Connector::INPUT_DIGITAL);
    ConnectorDI6.Mode(Connector::INPUT_DIGITAL);
    ConnectorDI7.Mode(Connector::INPUT_DIGITAL);
    ConnectorDI8.Mode(Connector::INPUT_DIGITAL);
    ConnectorA9.Mode(Connector::INPUT_DIGITAL);
    ConnectorA10.Mode(Connector::INPUT_DIGITAL);
    ConnectorA11.Mode(Connector::INPUT_DIGITAL);
    ConnectorA12.Mode(Connector::INPUT_DIGITAL);
}

void SetCcioEnabled(bool enable) {
    if (enable) {
        if (!ConnectorCOM1.PortIsOpen()) {
            (void)ConnectorCOM1.Mode(Connector::CCIO);
            ConnectorCOM1.PortOpen();
        }
    } else if (ConnectorCOM1.PortIsOpen()) {
        ConnectorCOM1.PortClose();
    }
}

void SetCcioDirectionModes(uint64_t direction_mask) {
    for (uint8_t bit = 0U; bit < 64U; ++bit) {
        CcioPin *const ccio_pin = CcioMgr.PinByIndex(CcioPinFromBitIndex(bit));
        if (ccio_pin == nullptr) {
            continue;
        }
        const bool output_mode = ((direction_mask >> bit) & 0x01ULL) != 0ULL;
        (void)ccio_pin->Mode(output_mode ? Connector::OUTPUT_DIGITAL : Connector::INPUT_DIGITAL);
    }
}

void ApplyCcioOutputs(uint64_t output_bits, uint64_t direction_mask) {
    for (uint8_t bit = 0U; bit < 64U; ++bit) {
        if (((direction_mask >> bit) & 0x01ULL) == 0ULL) {
            continue;
        }
        CcioMgr.PinState(CcioPinFromBitIndex(bit), ((output_bits >> bit) & 0x01ULL) != 0ULL);
    }
}

void FillCcioStatus(bool ccio_enabled, uint64_t direction_mask,
                    ethercat_slave::EthercatPdoStatus *status_image) {
    if (status_image == nullptr) {
        return;
    }

    status_image->ccio_direction_bits = ccio_enabled ? direction_mask : 0ULL;
    status_image->ccio_board_count = ccio_enabled ? CcioMgr.CcioCount() : 0U;
    status_image->ccio_input_bits = ccio_enabled ? static_cast<uint64_t>(CcioMgr.InputState()) : 0ULL;
    status_image->ccio_status_bits = ccio_enabled ? CcioMgr.IoOverloadRT() : 0ULL;
    status_image->ccio_reserved = 0U;

    uint16_t status = ccio_enabled ? kCcioStatusEnabled : 0U;
    if (ccio_enabled && CcioMgr.LinkBroken()) {
        status = static_cast<uint16_t>(status | kCcioStatusLinkBroken);
        status_image->ccio_status_bits |= (1ULL << 63U);
    }
    if (ccio_enabled && status_image->ccio_status_bits != 0ULL) {
        status = static_cast<uint16_t>(status | kCcioStatusAnyOverload);
    }
    status_image->ccio_status = status;
}

uint32_t AbsoluteDifference(uint32_t lhs, uint32_t rhs) {
    return (lhs > rhs) ? (lhs - rhs) : (rhs - lhs);
}

void SetIoDirectionModes(uint8_t direction_mask) {
    for (uint8_t bit = 0U; bit < kIoCount; ++bit) {
        const bool output_mode = ((direction_mask >> bit) & 0x01U) != 0U;
        const Connector::ConnectorModes mode =
            output_mode ? Connector::OUTPUT_DIGITAL : Connector::INPUT_DIGITAL;
        switch (bit) {
            case 0U:
                ConnectorIO0.Mode(mode);
                break;
            case 1U:
                ConnectorIO1.Mode(mode);
                break;
            case 2U:
                ConnectorIO2.Mode(mode);
                break;
            case 3U:
                ConnectorIO3.Mode(mode);
                break;
            case 4U:
                ConnectorIO4.Mode(mode);
                break;
            case 5U:
                ConnectorIO5.Mode(mode);
                break;
            default:
                break;
        }
    }
}

void ApplyIoOutputsFromCommandWord(uint16_t control_word, uint8_t direction_mask) {
    for (uint8_t bit = 0U; bit < kIoCount; ++bit) {
        const bool output_mode = ((direction_mask >> bit) & 0x01U) != 0U;
        if (!output_mode) {
            continue;
        }
        const bool level = ((control_word >> bit) & 0x01U) != 0U;
        switch (bit) {
            case 0U:
                ConnectorIO0.State(level);
                break;
            case 1U:
                ConnectorIO1.State(level);
                break;
            case 2U:
                ConnectorIO2.State(level);
                break;
            case 3U:
                ConnectorIO3.State(level);
                break;
            case 4U:
                ConnectorIO4.State(level);
                break;
            case 5U:
                ConnectorIO5.State(level);
                break;
            default:
                break;
        }
    }
}

uint8_t ReadIoStateBits(uint8_t direction_mask) {
    uint8_t io_state = 0U;
    io_state |= ((((direction_mask & (1U << 0U)) != 0U) ? ConnectorIO0.State() : ConnectorIO0.StateRT()) != 0) ? (1U << 0U) : 0U;
    io_state |= ((((direction_mask & (1U << 1U)) != 0U) ? ConnectorIO1.State() : ConnectorIO1.StateRT()) != 0) ? (1U << 1U) : 0U;
    io_state |= ((((direction_mask & (1U << 2U)) != 0U) ? ConnectorIO2.State() : ConnectorIO2.StateRT()) != 0) ? (1U << 2U) : 0U;
    io_state |= ((((direction_mask & (1U << 3U)) != 0U) ? ConnectorIO3.State() : ConnectorIO3.StateRT()) != 0) ? (1U << 3U) : 0U;
    io_state |= ((((direction_mask & (1U << 4U)) != 0U) ? ConnectorIO4.State() : ConnectorIO4.StateRT()) != 0) ? (1U << 4U) : 0U;
    io_state |= ((((direction_mask & (1U << 5U)) != 0U) ? ConnectorIO5.State() : ConnectorIO5.StateRT()) != 0) ? (1U << 5U) : 0U;
    return io_state;
}

uint16_t BuildStatusWord(uint8_t io_state_bits) {
    return static_cast<uint16_t>(io_state_bits & kIoBitMask);
}

uint8_t ResolveDirectionMask(const ethercat_slave::EthercatPdoCommand &command_image,
                             uint8_t current_direction_mask,
                             bool *direction_updated) {
    if ((command_image.reserved1 & kDirectionValidFlag) != 0U) {
        const uint8_t requested_mask =
            static_cast<uint8_t>(command_image.reserved1 & kIoBitMask);
        if (direction_updated != nullptr) {
            *direction_updated = true;
        }
        return requested_mask;
    }

    // Backward compatibility: legacy masters encoded direction in control_word[8..13].
    const uint8_t legacy_mask =
        static_cast<uint8_t>((command_image.control_word >> 8U) & kIoBitMask);
    if (legacy_mask != 0U) {
        if (direction_updated != nullptr) {
            *direction_updated = true;
        }
        return legacy_mask;
    }

    if (direction_updated != nullptr) {
        *direction_updated = false;
    }
    return current_direction_mask;
}

void SetAnalogInputModes(uint8_t analog_mode_mask) {
    ConnectorA9.Mode(((analog_mode_mask & (1U << 0U)) != 0U) ? Connector::INPUT_ANALOG
                                                              : Connector::INPUT_DIGITAL);
    ConnectorA10.Mode(((analog_mode_mask & (1U << 1U)) != 0U) ? Connector::INPUT_ANALOG
                                                               : Connector::INPUT_DIGITAL);
    ConnectorA11.Mode(((analog_mode_mask & (1U << 2U)) != 0U) ? Connector::INPUT_ANALOG
                                                               : Connector::INPUT_DIGITAL);
    ConnectorA12.Mode(((analog_mode_mask & (1U << 3U)) != 0U) ? Connector::INPUT_ANALOG
                                                               : Connector::INPUT_DIGITAL);
}

uint16_t ClampSignedStateToU16(int16_t signed_state) {
    if (signed_state < 0) {
        return 0U;
    }
    return static_cast<uint16_t>(signed_state);
}

int16_t ReadSupplyCentivolts() {
    const float supply_volts = AdcMgr.AnalogVoltage(AdcManager::ADC_VSUPPLY_MON);
    if (supply_volts <= 0.0F) {
        return 0;
    }

    const float centivolts = (supply_volts * 100.0F) + 0.5F;
    if (centivolts > 32767.0F) {
        return 32767;
    }
    return static_cast<int16_t>(centivolts);
}

void FillExtendedInputStatus(uint8_t analog_mode_mask, uint8_t direction_mask,
                             ethercat_slave::EthercatPdoStatus *status_image) {
    if (status_image == nullptr) {
        return;
    }

    uint8_t digital_bank = 0U;
    if (ConnectorDI6.StateRT() != 0) {
        digital_bank |= static_cast<uint8_t>(1U << kDi6StatusBit);
    }
    if (ConnectorDI7.StateRT() != 0) {
        digital_bank |= static_cast<uint8_t>(1U << kDi7StatusBit);
    }
    if (ConnectorDI8.StateRT() != 0) {
        digital_bank |= static_cast<uint8_t>(1U << kDi8StatusBit);
    }
    if (((analog_mode_mask & (1U << 0U)) == 0U) && (ConnectorA9.StateRT() != 0)) {
        digital_bank |= static_cast<uint8_t>(1U << kA9DigitalStatusBit);
    }
    if (((analog_mode_mask & (1U << 1U)) == 0U) && (ConnectorA10.StateRT() != 0)) {
        digital_bank |= static_cast<uint8_t>(1U << kA10DigitalStatusBit);
    }
    if (((analog_mode_mask & (1U << 2U)) == 0U) && (ConnectorA11.StateRT() != 0)) {
        digital_bank |= static_cast<uint8_t>(1U << kA11DigitalStatusBit);
    }
    if (((analog_mode_mask & (1U << 3U)) == 0U) && (ConnectorA12.StateRT() != 0)) {
        digital_bank |= static_cast<uint8_t>(1U << kA12DigitalStatusBit);
    }

    const uint16_t a9_raw = ClampSignedStateToU16(ConnectorA9.State());
    const uint16_t a10_raw = ClampSignedStateToU16(ConnectorA10.State());
    const uint16_t a11_raw = ClampSignedStateToU16(ConnectorA11.State());
    const uint16_t a12_raw = ClampSignedStateToU16(ConnectorA12.State());

    status_image->mode_of_operation_display = static_cast<int8_t>(analog_mode_mask & kAnalogModeMask);
    status_image->fault_code = 0U;
    status_image->actual_position_counts =
        static_cast<int32_t>(static_cast<uint32_t>(a9_raw) |
                             (static_cast<uint32_t>(a10_raw) << 16U));
    status_image->actual_velocity_counts_per_sec =
        static_cast<int32_t>(static_cast<uint32_t>(a11_raw) |
                             (static_cast<uint32_t>(a12_raw) << 16U));
    status_image->actual_torque_tenths_percent = ReadSupplyCentivolts();
    status_image->reserved0 = static_cast<uint16_t>(
        static_cast<uint16_t>(digital_bank) |
        static_cast<uint16_t>(static_cast<uint16_t>(direction_mask & kIoBitMask) << 8U));
}

}  // namespace

int main(void) {
    ConnectorUsb.PortOpen();
    Delay_ms(100);

    ConnectorUsb.SendLine("\r\n========================================");
    ConnectorUsb.SendLine("  EtherCAT Slave Personality Firmware");
    ConnectorUsb.SendLine("========================================\r\n");
    ConnectorUsb.SendLine("Waiting for Ethernet link...");
    ConnectorUsb.Flush();

    const uint32_t link_timeout_ms = 5000U;
    const uint32_t link_wait_start = Milliseconds();
    while (!EthernetMgr.PhyLinkActive()) {
        if (Milliseconds() - link_wait_start > link_timeout_ms) {
            ConnectorUsb.SendLine("ERROR: Ethernet link timeout.");
            while (true) {
                Delay_ms(1000);
            }
        }
        Delay_ms(100);
    }

    EthernetMgr.Setup();
    Delay_ms(100);

    struct netif *netif = EthernetMgr.MacInterface();
    if (netif == nullptr) {
        ConnectorUsb.SendLine("ERROR: Failed to get netif pointer.");
        while (true) {
            Delay_ms(1000);
        }
    }

    ethercat_slave::EthercatSlaveInit(netif);
    ConfigureIoDefaults();

    char ip_message[48];
    if (netif->ip_addr.addr != 0U) {
        snprintf(ip_message, sizeof(ip_message), "IP Address: %d.%d.%d.%d",
                 ip4_addr1(&netif->ip_addr),
                 ip4_addr2(&netif->ip_addr),
                 ip4_addr3(&netif->ip_addr),
                 ip4_addr4(&netif->ip_addr));
    } else {
        snprintf(ip_message, sizeof(ip_message), "IP Address: not assigned");
    }
    ConnectorUsb.SendLine(ip_message);
    ConnectorUsb.SendLine("Entering EtherCAT raw-frame loop.");
    ConnectorUsb.Flush();

    uint32_t last_stats_log_ms = Milliseconds();
    uint8_t applied_direction_mask = static_cast<uint8_t>(kIoBitMask);
    SetIoDirectionModes(applied_direction_mask);
    uint8_t applied_analog_mode_mask = 0xFFU;
    bool ccio_enabled = false;
    uint64_t applied_ccio_direction_mask = 0ULL;
    uint32_t previous_loop_start_us = Microseconds();
    while (true) {
        const uint32_t loop_start_us = Microseconds();
        const ethercat_slave::EthercatPdoCommand &command_image =
            ethercat_slave::CommandImage();
        ethercat_slave::EthercatPdoStatus &status_image =
            ethercat_slave::StatusImage();
        bool direction_updated = false;
        const uint8_t resolved_direction_mask = ResolveDirectionMask(
            command_image, applied_direction_mask, &direction_updated);
        if (direction_updated && resolved_direction_mask != applied_direction_mask) {
            SetIoDirectionModes(resolved_direction_mask);
            applied_direction_mask = resolved_direction_mask;
        }

        const uint8_t requested_analog_mode_mask =
            static_cast<uint8_t>(command_image.reserved0 & kAnalogModeMask);
        if (requested_analog_mode_mask != applied_analog_mode_mask) {
            SetAnalogInputModes(requested_analog_mode_mask);
            applied_analog_mode_mask = requested_analog_mode_mask;
        }

        ApplyIoOutputsFromCommandWord(command_image.control_word, applied_direction_mask);
        const bool requested_ccio_enabled =
            (command_image.ccio_control & kCcioControlEnable) != 0U;
        if (requested_ccio_enabled != ccio_enabled) {
            SetCcioEnabled(requested_ccio_enabled);
            ccio_enabled = requested_ccio_enabled;
            applied_ccio_direction_mask = 0ULL;
        }
        if (ccio_enabled && command_image.ccio_direction_bits != applied_ccio_direction_mask) {
            SetCcioDirectionModes(command_image.ccio_direction_bits);
            applied_ccio_direction_mask = command_image.ccio_direction_bits;
        }
        if (ccio_enabled) {
            ApplyCcioOutputs(command_image.ccio_output_bits, applied_ccio_direction_mask);
        }
        status_image.status_word = BuildStatusWord(ReadIoStateBits(applied_direction_mask));
        FillExtendedInputStatus(applied_analog_mode_mask, applied_direction_mask, &status_image);
        FillCcioStatus(ccio_enabled, applied_ccio_direction_mask, &status_image);

        const uint32_t transport_start_us = Microseconds();
        EthernetMgr.Refresh();
        ethercat_slave::EthercatSlaveCyclic();
        const uint32_t transport_us = Microseconds() - transport_start_us;
        const uint32_t loop_runtime_us = Microseconds() - loop_start_us;
        const uint32_t loop_period_us = loop_start_us - previous_loop_start_us;
        previous_loop_start_us = loop_start_us;
        status_image.firmware_loop_period_us = loop_period_us;
        status_image.firmware_loop_runtime_us = loop_runtime_us;
        status_image.firmware_loop_jitter_us =
            AbsoluteDifference(loop_period_us, kExpectedLoopPeriodUs);
        status_image.firmware_transport_us = transport_us;

        const uint32_t now_ms = Milliseconds();
        if (now_ms - last_stats_log_ms >= 1000U) {
            const ethercat_slave::EthercatSlaveStats &stats = ethercat_slave::Stats();
            char stats_message[96];
            snprintf(stats_message, sizeof(stats_message),
                     "ECAT stats: rx_ok=%lu rx_drop=%lu tx_ok=%lu tx_drop=%lu",
                     static_cast<unsigned long>(stats.rx_ok),
                     static_cast<unsigned long>(stats.rx_drop),
                     static_cast<unsigned long>(stats.tx_ok),
                     static_cast<unsigned long>(stats.tx_drop));
            ConnectorUsb.SendLine(stats_message);
            last_stats_log_ms = now_ms;
        }

        Delay_ms(1);
    }
}

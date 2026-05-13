/*******************************************************************************
 * EtherNet/IP -- ClearCore hardware bridge for ClearLink-style I/O assembly
 *
 * This header is PROJECT-OWNED: do not overwrite from upstream OpENer or DX200.
 * Implemented in clearcore_clearlink_bridge.cpp (C++ -> libClearCore).
 *
 * Copyright (c) 2025 Adam G. Sweeney <agsweeney@gmail.com>
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#ifndef CLEARCORE_CLEARLINK_BRIDGE_H_
#define CLEARCORE_CLEARLINK_BRIDGE_H_

#ifdef CLEARCORE

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void ConnectorIO0_SetAnalogOutputMode(void);
void ConnectorIO0_SetOutputCurrentMicroamps(uint16_t microamps);
void ConnectorIO0_SetPwmOutputMode(void);
void ConnectorIO0_SetPwmDuty(uint8_t duty);
void ConnectorIO0_SetDigitalOutputMode(void);
void ConnectorIO0_SetDigitalInputMode(void);
int ConnectorIO0_IsInFault(void);
int ConnectorIO0_GetInputState(void);

void ConnectorIO1_SetPwmOutputMode(void);
void ConnectorIO1_SetPwmDuty(uint8_t duty);
void ConnectorIO1_SetDigitalOutputMode(void);
void ConnectorIO1_SetDigitalInputMode(void);
int ConnectorIO1_IsInFault(void);
int ConnectorIO1_GetInputState(void);

void ConnectorIO2_SetPwmOutputMode(void);
void ConnectorIO2_SetPwmDuty(uint8_t duty);
void ConnectorIO2_SetDigitalOutputMode(void);
void ConnectorIO2_SetDigitalInputMode(void);
int ConnectorIO2_IsInFault(void);
int ConnectorIO2_GetInputState(void);

void ConnectorIO3_SetPwmOutputMode(void);
void ConnectorIO3_SetPwmDuty(uint8_t duty);
void ConnectorIO3_SetDigitalOutputMode(void);
void ConnectorIO3_SetDigitalInputMode(void);
int ConnectorIO3_IsInFault(void);
int ConnectorIO3_GetInputState(void);

void ConnectorIO4_SetPwmOutputMode(void);
void ConnectorIO4_SetPwmDuty(uint8_t duty);
void ConnectorIO4_SetDigitalOutputMode(void);
void ConnectorIO4_SetDigitalInputMode(void);
int ConnectorIO4_IsInFault(void);
int ConnectorIO4_GetInputState(void);

void ConnectorIO5_SetPwmOutputMode(void);
void ConnectorIO5_SetPwmDuty(uint8_t duty);
void ConnectorIO5_SetDigitalOutputMode(void);
void ConnectorIO5_SetDigitalInputMode(void);
int ConnectorIO5_IsInFault(void);
int ConnectorIO5_GetInputState(void);

void ConnectorA9_SetAnalogInputMode(void);
void ConnectorA9_SetAnalogFilterMs(uint8_t ms);
void ConnectorA9_SetDigitalInputMode(void);
uint16_t ConnectorA9_GetAnalogRaw(void);

void ConnectorA10_SetAnalogInputMode(void);
void ConnectorA10_SetAnalogFilterMs(uint8_t ms);
void ConnectorA10_SetDigitalInputMode(void);
uint16_t ConnectorA10_GetAnalogRaw(void);

void ConnectorA11_SetAnalogInputMode(void);
void ConnectorA11_SetAnalogFilterMs(uint8_t ms);
void ConnectorA11_SetDigitalInputMode(void);
uint16_t ConnectorA11_GetAnalogRaw(void);

void ConnectorA12_SetAnalogInputMode(void);
void ConnectorA12_SetAnalogFilterMs(uint8_t ms);
void ConnectorA12_SetDigitalInputMode(void);
uint16_t ConnectorA12_GetAnalogRaw(void);

void ConnectorDipApplyFilterUs(uint8_t connector_index,
                               uint16_t off_on_microseconds,
                               uint16_t on_off_microseconds);

void Ccio_Initialize(void);
void Encoder_Initialize(void);
uint64_t Ccio_GetInputBits(void);
uint64_t Ccio_GetStatusBits(void);
uint8_t Ccio_GetBoardCount(void);
void Ccio_SetOutputBits(uint64_t output_bits);
void Ccio_SetBoardFilterMs(uint8_t board_index, uint8_t filter_ms);
void Ccio_SetEnabled(int enable);

void Encoder_SetSwapDirection(int swap_direction);
void Encoder_SetIndexInverted(int invert_index);
void Encoder_SetEnabled(int enable);
void Encoder_AddToPosition(int32_t delta_counts);
int32_t Encoder_GetPosition(void);
int32_t Encoder_GetVelocity(void);
int32_t Encoder_GetIndexPosition(void);
int Encoder_GetAlarmFlag(void);

/**
 * @brief Step/Direction motor connectors M0–M3 (Teknic ClearLink motor objects / assemblies).
 *
 * Applies EtherNet/IP motor output fields using libClearCore MotorDriver semantics
 * (enable, limits, load-position / load-velocity handshake per object reference Table 28).
 * Homing (output register bit 2) uses the same Move path as a normal load-position; Teknic
 * sensor / hard-stop / position-zero sequencing is not replicated.
 */
void ClearLinkMotor_Init(void);
void ClearLinkMotor_ApplyStepDirOutputs(unsigned axis,
                                        int32_t move_distance,
                                        int32_t jog_velocity,
                                        uint32_t vel_limit,
                                        uint32_t accel_limit,
                                        uint32_t decel_limit,
                                        uint32_t output_register);
void ClearLinkMotor_AddToPosition(unsigned axis, int32_t delta_steps);
/** @brief MC implicit output: `mc_output_byte` bit 0 drives `MotorDriver::EnableRequest`. */
void ClearLinkMotor_ApplyMConnectorOutputs(unsigned axis,
                                             uint16_t trigger_pulses,
                                             uint16_t a_pwm,
                                             uint16_t b_pwm,
                                             uint8_t mc_output_byte,
                                             uint16_t enable_pulse_time_ms);
/** @brief Returns non-zero if IO-[io_index] is reserved as any motor brake output. */
int ClearLinkMotor_IsIoReservedForBrakeOutput(uint8_t io_index);
/**
 * @brief Maps Teknic connector indices to libClearCore pins for MC mode routing.
 *
 * On-board: 0–12 → IO-0 … A-12. CCIO-8: 13–76 → CCIOA0 … CCIOH7 (64 inputs). Use -1 to disable.
 */
void ClearLinkMotor_ApplyMConnectorRouting(unsigned axis,
                                           int16_t enable_connector_index,
                                           int16_t input_a_connector_index,
                                           int16_t input_b_connector_index);
/**
 * @brief Applies motor configuration (class 0x64) to MotorDriver and bridge state.
 *
 * @param config_register Class 0x64 DWORD per ClearLink object reference Table 21 (bit 2 enable
 *        inversion, bit 3 HLFB inversion, bit 5 soft limit enable; bits 0–1 homing; bit 4 capture).
 * @param soft_limit_1 @param soft_limit_2 Soft limits apply only when the two values differ and
 *        config bit 5 is set; bounds are min/max. Absolute load-position targets outside the range
 *        are rejected; relative targets use `PositionRefCommanded() + distance` as an approximate
 *        endpoint check.
 *
 * Home sensor connector is consumed by bridge-side homing logic when output register bit 2 is set.
 * Position-capture connector is not assigned on `MotorDriver` (no capture API in libClearCore).
 */
void ClearLinkMotor_ApplyMotorConfiguration(unsigned axis,
                                          uint32_t config_register,
                                          int32_t soft_limit_1,
                                          int32_t soft_limit_2,
                                          int16_t positive_limit_connector_index,
                                          int16_t negative_limit_connector_index,
                                          int16_t home_sensor_connector_index,
                                          int16_t brake_output_connector_index,
                                          int16_t stop_sensor_connector_index,
                                          int32_t max_deceleration_rate_steps_per_sec2);
void ClearLinkMotor_PollStepDirFeedback(unsigned axis,
                                        uint32_t extra_status_bits,
                                        int32_t *commanded_position,
                                        int32_t *commanded_velocity,
                                        int32_t *target_position,
                                        int32_t *target_velocity,
                                        float *measured_torque,
                                        uint32_t *motor_status,
                                        uint32_t *motor_shutdowns);
void ClearLinkMotor_PollMConnectorFeedback(unsigned axis,
                                           float *hlfb_duty_percent,
                                           uint16_t *status_word);

/** @brief ClearLink Board Object (vendor class 0x69), motor/board personality. */
int BoardMotorMode_Request(int step_and_dir_nonzero);
void BoardVoltageSamples(float *auxiliary_volts, float *supply_volts);

/**
 * Writes application blink bitmask from EtherNet/IP; bytes 1-4 unused for hardware apply today.
 */
void BoardApplyBlinkFiveBytes(const uint8_t data[5]);

/** Copies live blink group masks (five bytes = UI groups 1..5) for Get_Attribute readback. */
void BoardBlinkCodesSnapshotFiveBytes(uint8_t data[5]);

/** ClearLink Board Object attr 3: clear all latched/status blink codes (Teknic UI groups 1..5). */
void BoardBlinkCodes_ResetLatchingBlinkCodes(void);

/**
 * ClearLink ASCII Object (class 70H): COM-0 baud/status mirrors and Apply on Set_Attribute.
 * Mirrors Teknic ASCII config/status register low bits where libClearCore supports them.
 */
void AsciiCom0_RefreshMirrors(uint32_t *baud_udint, uint32_t *status_udint);
void AsciiCom0_ApplyCommitted(uint32_t config_register_udint, uint32_t baud_udint);
void AsciiCom0_ReadRxIntoBuffer(uint8_t *dst,
                                size_t dst_capacity,
                                uint32_t *bytes_read_out);
void AsciiCom0_SendTxBytes(const uint8_t *src, size_t byte_count);

#ifdef __cplusplus
}
#endif

#endif  /* CLEARCORE */

#endif  /* CLEARCORE_CLEARLINK_BRIDGE_H_ */

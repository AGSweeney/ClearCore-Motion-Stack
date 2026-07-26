// EXCERPT — source: ProjectTemplate/ClearLinkCompatibilityFirmware/OpENer/source/src/ports/ClearCore/clearcore_clearlink_bridge.h
// EVIDENCE: E1 | symbol: clearcore_clearlink_bridge.h | lines: 1-110
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

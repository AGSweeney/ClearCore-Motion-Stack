// EXCERPT — source: ProjectTemplate/EtherCATSlaveFirmware/main.cpp
// EVIDENCE: E1 | symbol: main includes | lines: 1-40
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

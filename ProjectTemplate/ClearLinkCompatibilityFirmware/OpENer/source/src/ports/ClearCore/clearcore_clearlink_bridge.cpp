/*******************************************************************************
 * EtherNet/IP -- ClearCore hardware bridge for ClearLink-style I/O assembly
 *
 * PROJECT-OWNED: keep out of DX200 / vanilla OpENer copy operations.
 * See AGENTS.md ("OpENer port files that must not be overwritten").
 *
 * Copyright (c) 2025 Adam G. Sweeney <agsweeney@gmail.com>
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#ifdef CLEARCORE

#include <algorithm>
#include <cstring>

#include "ClearCore.h"
#include "MotorDriver.h"
#include "SysConnectors.h"
#include "ports/ClearCore/clearcore_clearlink_bridge.h"

extern "C" {

namespace {

using ClearCore::CcioMgr;
using ClearCore::DigitalIn;
using ClearCore::EncoderIn;

uint16_t MaxU16(uint16_t a, uint16_t b) {
  return (a > b) ? a : b;
}

void ApplyDigitalDebounceUs(DigitalIn &connector,
                            uint16_t off_on_us,
                            uint16_t on_off_us) {
  const uint32_t longest = static_cast<uint32_t>(MaxU16(off_on_us, on_off_us));
  uint16_t ms = static_cast<uint16_t>((longest + 999UL) / 1000UL);
  if (ms < 1U) {
    ms = 1U;
  }
  connector.FilterLength(ms, DigitalIn::FILTER_UNIT_MS);
}

ClearCorePins CcioPinFromBitIndex(unsigned bit_index) {
  const unsigned pin_index =
      static_cast<unsigned>(CLEARCORE_PIN_CCIOA0) + bit_index;
  if (pin_index >= static_cast<unsigned>(CLEARCORE_PIN_CCIO_MAX)) {
    return CLEARCORE_PIN_CCIOA0;
  }
  return static_cast<ClearCorePins>(pin_index);
}

} /* namespace */

void ConnectorIO0_SetAnalogOutputMode(void) {
  (void)ConnectorIO0.Mode(Connector::OUTPUT_ANALOG);
}

void ConnectorIO0_SetOutputCurrentMicroamps(uint16_t microamps) {
  ConnectorIO0.OutputCurrent(microamps);
}

void ConnectorIO0_SetPwmOutputMode(void) {
  (void)ConnectorIO0.Mode(Connector::OUTPUT_PWM);
}

void ConnectorIO0_SetPwmDuty(uint8_t duty) {
  (void)ConnectorIO0.PwmDuty(duty);
}

void ConnectorIO0_SetDigitalOutputMode(void) {
  (void)ConnectorIO0.Mode(Connector::OUTPUT_DIGITAL);
}

void ConnectorIO0_SetDigitalInputMode(void) {
  (void)ConnectorIO0.Mode(Connector::INPUT_DIGITAL);
}

int ConnectorIO0_IsInFault(void) {
  return ConnectorIO0.IsInHwFault() ? 1 : 0;
}

int ConnectorIO0_GetInputState(void) {
  return (ConnectorIO0.State() != 0) ? 1 : 0;
}

void ConnectorIO1_SetPwmOutputMode(void) {
  (void)ConnectorIO1.Mode(Connector::OUTPUT_PWM);
}

void ConnectorIO1_SetPwmDuty(uint8_t duty) {
  (void)ConnectorIO1.PwmDuty(duty);
}

void ConnectorIO1_SetDigitalOutputMode(void) {
  (void)ConnectorIO1.Mode(Connector::OUTPUT_DIGITAL);
}

void ConnectorIO1_SetDigitalInputMode(void) {
  (void)ConnectorIO1.Mode(Connector::INPUT_DIGITAL);
}

int ConnectorIO1_IsInFault(void) {
  return ConnectorIO1.IsInHwFault() ? 1 : 0;
}

int ConnectorIO1_GetInputState(void) {
  return (ConnectorIO1.State() != 0) ? 1 : 0;
}

void ConnectorIO2_SetPwmOutputMode(void) {
  (void)ConnectorIO2.Mode(Connector::OUTPUT_PWM);
}

void ConnectorIO2_SetPwmDuty(uint8_t duty) {
  (void)ConnectorIO2.PwmDuty(duty);
}

void ConnectorIO2_SetDigitalOutputMode(void) {
  (void)ConnectorIO2.Mode(Connector::OUTPUT_DIGITAL);
}

void ConnectorIO2_SetDigitalInputMode(void) {
  (void)ConnectorIO2.Mode(Connector::INPUT_DIGITAL);
}

int ConnectorIO2_IsInFault(void) {
  return ConnectorIO2.IsInHwFault() ? 1 : 0;
}

int ConnectorIO2_GetInputState(void) {
  return (ConnectorIO2.State() != 0) ? 1 : 0;
}

void ConnectorIO3_SetPwmOutputMode(void) {
  (void)ConnectorIO3.Mode(Connector::OUTPUT_PWM);
}

void ConnectorIO3_SetPwmDuty(uint8_t duty) {
  (void)ConnectorIO3.PwmDuty(duty);
}

void ConnectorIO3_SetDigitalOutputMode(void) {
  (void)ConnectorIO3.Mode(Connector::OUTPUT_DIGITAL);
}

void ConnectorIO3_SetDigitalInputMode(void) {
  (void)ConnectorIO3.Mode(Connector::INPUT_DIGITAL);
}

int ConnectorIO3_IsInFault(void) {
  return ConnectorIO3.IsInHwFault() ? 1 : 0;
}

int ConnectorIO3_GetInputState(void) {
  return (ConnectorIO3.State() != 0) ? 1 : 0;
}

void ConnectorIO4_SetPwmOutputMode(void) {
  (void)ConnectorIO4.Mode(Connector::OUTPUT_PWM);
}

void ConnectorIO4_SetPwmDuty(uint8_t duty) {
  (void)ConnectorIO4.PwmDuty(duty);
}

void ConnectorIO4_SetDigitalOutputMode(void) {
  (void)ConnectorIO4.Mode(Connector::OUTPUT_DIGITAL);
}

void ConnectorIO4_SetDigitalInputMode(void) {
  (void)ConnectorIO4.Mode(Connector::INPUT_DIGITAL);
}

int ConnectorIO4_IsInFault(void) {
  return ConnectorIO4.IsInHwFault() ? 1 : 0;
}

int ConnectorIO4_GetInputState(void) {
  return (ConnectorIO4.State() != 0) ? 1 : 0;
}

void ConnectorIO5_SetPwmOutputMode(void) {
  (void)ConnectorIO5.Mode(Connector::OUTPUT_PWM);
}

void ConnectorIO5_SetPwmDuty(uint8_t duty) {
  (void)ConnectorIO5.PwmDuty(duty);
}

void ConnectorIO5_SetDigitalOutputMode(void) {
  (void)ConnectorIO5.Mode(Connector::OUTPUT_DIGITAL);
}

void ConnectorIO5_SetDigitalInputMode(void) {
  (void)ConnectorIO5.Mode(Connector::INPUT_DIGITAL);
}

int ConnectorIO5_IsInFault(void) {
  return ConnectorIO5.IsInHwFault() ? 1 : 0;
}

int ConnectorIO5_GetInputState(void) {
  return (ConnectorIO5.State() != 0) ? 1 : 0;
}

void ConnectorA9_SetAnalogInputMode(void) {
  (void)ConnectorA9.Mode(Connector::INPUT_ANALOG);
}

void ConnectorA9_SetAnalogFilterMs(uint8_t ms) {
  (void)ConnectorA9.FilterTc(ms, ClearCore::AdcManager::FILTER_UNIT_MS);
}

void ConnectorA9_SetDigitalInputMode(void) {
  (void)ConnectorA9.Mode(Connector::INPUT_DIGITAL);
}

uint16_t ConnectorA9_GetAnalogRaw(void) {
  const int16_t s = ConnectorA9.State();
  if (s < 0) {
    return 0U;
  }
  return static_cast<uint16_t>(s);
}

void ConnectorA10_SetAnalogInputMode(void) {
  (void)ConnectorA10.Mode(Connector::INPUT_ANALOG);
}

void ConnectorA10_SetAnalogFilterMs(uint8_t ms) {
  (void)ConnectorA10.FilterTc(ms, ClearCore::AdcManager::FILTER_UNIT_MS);
}

void ConnectorA10_SetDigitalInputMode(void) {
  (void)ConnectorA10.Mode(Connector::INPUT_DIGITAL);
}

uint16_t ConnectorA10_GetAnalogRaw(void) {
  const int16_t s = ConnectorA10.State();
  if (s < 0) {
    return 0U;
  }
  return static_cast<uint16_t>(s);
}

void ConnectorA11_SetAnalogInputMode(void) {
  (void)ConnectorA11.Mode(Connector::INPUT_ANALOG);
}

void ConnectorA11_SetAnalogFilterMs(uint8_t ms) {
  (void)ConnectorA11.FilterTc(ms, ClearCore::AdcManager::FILTER_UNIT_MS);
}

void ConnectorA11_SetDigitalInputMode(void) {
  (void)ConnectorA11.Mode(Connector::INPUT_DIGITAL);
}

uint16_t ConnectorA11_GetAnalogRaw(void) {
  const int16_t s = ConnectorA11.State();
  if (s < 0) {
    return 0U;
  }
  return static_cast<uint16_t>(s);
}

void ConnectorA12_SetAnalogInputMode(void) {
  (void)ConnectorA12.Mode(Connector::INPUT_ANALOG);
}

void ConnectorA12_SetAnalogFilterMs(uint8_t ms) {
  (void)ConnectorA12.FilterTc(ms, ClearCore::AdcManager::FILTER_UNIT_MS);
}

void ConnectorA12_SetDigitalInputMode(void) {
  (void)ConnectorA12.Mode(Connector::INPUT_DIGITAL);
}

uint16_t ConnectorA12_GetAnalogRaw(void) {
  const int16_t s = ConnectorA12.State();
  if (s < 0) {
    return 0U;
  }
  return static_cast<uint16_t>(s);
}

void ConnectorDipApplyFilterUs(uint8_t connector_index,
                               uint16_t off_on_microseconds,
                               uint16_t on_off_microseconds) {
  switch (connector_index) {
    case 0:
      ApplyDigitalDebounceUs(ConnectorIO0, off_on_microseconds,
                             on_off_microseconds);
      break;
    case 1:
      ApplyDigitalDebounceUs(ConnectorIO1, off_on_microseconds,
                             on_off_microseconds);
      break;
    case 2:
      ApplyDigitalDebounceUs(ConnectorIO2, off_on_microseconds,
                             on_off_microseconds);
      break;
    case 3:
      ApplyDigitalDebounceUs(ConnectorIO3, off_on_microseconds,
                             on_off_microseconds);
      break;
    case 4:
      ApplyDigitalDebounceUs(ConnectorIO4, off_on_microseconds,
                             on_off_microseconds);
      break;
    case 5:
      ApplyDigitalDebounceUs(ConnectorIO5, off_on_microseconds,
                             on_off_microseconds);
      break;
    case 6:
      ApplyDigitalDebounceUs(ConnectorDI6, off_on_microseconds,
                             on_off_microseconds);
      break;
    case 7:
      ApplyDigitalDebounceUs(ConnectorDI7, off_on_microseconds,
                             on_off_microseconds);
      break;
    case 8:
      ApplyDigitalDebounceUs(ConnectorDI8, off_on_microseconds,
                             on_off_microseconds);
      break;
    case 9:
      ApplyDigitalDebounceUs(ConnectorA9, off_on_microseconds,
                             on_off_microseconds);
      break;
    case 10:
      ApplyDigitalDebounceUs(ConnectorA10, off_on_microseconds,
                             on_off_microseconds);
      break;
    case 11:
      ApplyDigitalDebounceUs(ConnectorA11, off_on_microseconds,
                             on_off_microseconds);
      break;
    case 12:
      ApplyDigitalDebounceUs(ConnectorA12, off_on_microseconds,
                             on_off_microseconds);
      break;
    default:
      break;
  }
}

void Ccio_Initialize(void) {
  /* Ccio_SetEnabled owns COM-1 open/close; configuration applies later. */
}

void Encoder_Initialize(void) {
  EncoderIn.ClearQuadratureError();
  EncoderIn.Enable(true);
}

uint64_t Ccio_GetInputBits(void) {
  return static_cast<uint64_t>(CcioMgr.InputState());
}

uint64_t Ccio_GetStatusBits(void) {
  uint64_t bits = CcioMgr.IoOverloadRT();
  if (CcioMgr.LinkBroken()) {
    bits |= (1ULL << 63);
  }
  return bits;
}

uint8_t Ccio_GetBoardCount(void) {
  return CcioMgr.CcioCount();
}

void Ccio_SetOutputBits(uint64_t output_bits) {
  for (unsigned i = 0; i < 64U; ++i) {
    const bool level = ((output_bits >> i) & 1ULL) != 0ULL;
    const ClearCorePins pin = CcioPinFromBitIndex(i);
    ClearCore::CcioPin *const ccio_pin = CcioMgr.PinByIndex(pin);
    if (ccio_pin == nullptr) {
      continue;
    }
    // CCIO pins default to INPUT_DIGITAL; writes are masked until output mode is enabled.
    if (level && !ccio_pin->IsWritable()) {
      (void)ccio_pin->Mode(Connector::OUTPUT_DIGITAL);
    }
    CcioMgr.PinState(pin, level);
  }
}

void Ccio_SetBoardFilterMs(uint8_t board_index, uint8_t filter_ms) {
  if (board_index >= 8U) {
    return;
  }
  const uint16_t len = filter_ms;
  const unsigned base = static_cast<unsigned>(board_index) * 8U;
  for (unsigned pin = 0; pin < 8U; ++pin) {
    ClearCore::CcioPin *const p_pin =
        CcioMgr.PinByIndex(CcioPinFromBitIndex(base + pin));
    if (p_pin != nullptr) {
      p_pin->Filter_ms(len);
    }
  }
}

void Ccio_SetEnabled(int enable) {
  if (enable) {
    if (!ConnectorCOM1.PortIsOpen()) {
      (void)ConnectorCOM1.Mode(Connector::CCIO);
      ConnectorCOM1.PortOpen();
    }
  } else {
    if (ConnectorCOM1.PortIsOpen()) {
      ConnectorCOM1.PortClose();
    }
  }
}

void Encoder_SetSwapDirection(int swap_direction) {
  EncoderIn.SwapDirection(swap_direction != 0);
}

void Encoder_SetIndexInverted(int invert_index) {
  EncoderIn.IndexInverted(invert_index != 0);
}

void Encoder_SetEnabled(int enable) {
  EncoderIn.Enable(enable != 0);
}

void Encoder_AddToPosition(int32_t delta_counts) {
  EncoderIn.AddToPosition(delta_counts);
}

int32_t Encoder_GetPosition(void) {
  return EncoderIn.Position();
}

int32_t Encoder_GetVelocity(void) {
  const int32_t v = EncoderIn.Velocity();
  return v;
}

int32_t Encoder_GetIndexPosition(void) {
  return EncoderIn.IndexPosition();
}

int Encoder_GetAlarmFlag(void) {
  return EncoderIn.QuadratureError() ? 1 : 0;
}

namespace {

MotorDriver *MotorAxisPtr(unsigned axis) {
  switch (axis) {
    case 0:
      return &ConnectorM0;
    case 1:
      return &ConnectorM1;
    case 2:
      return &ConnectorM2;
    case 3:
      return &ConnectorM3;
    default:
      return nullptr;
  }
}

struct StepDirMotorBridgeState {
  uint32_t prev_output_register;
  bool pos_move_accepted_while_load_high;
  bool vel_move_accepted_while_load_high;
  int32_t last_target_position_steps;
  int32_t last_target_velocity_steps_per_s;
  bool soft_limits_enabled;
  int32_t soft_limit_lo_steps;
  int32_t soft_limit_hi_steps;
  bool homing_enabled;
  bool home_sensor_active_high;
  int16_t home_sensor_connector_index;
  bool homing_move_active;
  ClearCorePins brake_output_pin;
  /** Table 25 bit 4: latched when O2T output register SW E-Stop (bit 5) asserts. */
  bool shutdown_sw_estop_latched;
  /** Table 25 bit 0: latched when a motion command is issued while in shutdown/disabled state. */
  bool shutdown_command_while_shutdown_latched;
  /** Table 25 bit 5: latched when a motion command is rejected because motor is not enabled. */
  bool shutdown_motor_disabled_latched;
  /** Table 25 bit 6: latched when a position load is rejected due to soft limits. */
  bool shutdown_soft_limit_exceeded_latched;
};

StepDirMotorBridgeState g_step_dir_bridge[4];

/** Non-zero: clamp E-stop / output decel to this cap (class 0x64 attr 9, steps/s/s). */
uint32_t g_motor_estop_decel_cap_steps_s2[4];

uint32_t ClampVelocityLimit(uint32_t vel) {
  if (vel == 0U) {
    return 1U;
  }
  if (vel > 500000U) {
    return 500000U;
  }
  return vel;
}

uint32_t ClampAccel(uint32_t accel) {
  if (accel < 1527U) {
    return 1527U;
  }
  return accel;
}

bool ReadBoardInputByConnectorIndex(const int16_t idx) {
  if (idx < 0) {
    return false;
  }
  switch (idx) {
    case 0:
      return ConnectorIO0.State() != 0;
    case 1:
      return ConnectorIO1.State() != 0;
    case 2:
      return ConnectorIO2.State() != 0;
    case 3:
      return ConnectorIO3.State() != 0;
    case 4:
      return ConnectorIO4.State() != 0;
    case 5:
      return ConnectorIO5.State() != 0;
    case 6:
      return ConnectorDI6.State() != 0;
    case 7:
      return ConnectorDI7.State() != 0;
    case 8:
      return ConnectorDI8.State() != 0;
    case 9:
      return ConnectorA9.State() != 0;
    case 10:
      return ConnectorA10.State() != 0;
    case 11:
      return ConnectorA11.State() != 0;
    case 12:
      return ConnectorA12.State() != 0;
    default:
      break;
  }
  if (idx <= 76) {
    const unsigned ccio_bit = static_cast<unsigned>(idx - 13);
    const uint64_t ccio_inputs = Ccio_GetInputBits();
    return ((ccio_inputs >> ccio_bit) & 1ULL) != 0ULL;
  }
  return false;
}

bool HomeSensorActive(const StepDirMotorBridgeState &state) {
  if (state.home_sensor_connector_index < 0) {
    return false;
  }
  const bool raw_state = ReadBoardInputByConnectorIndex(state.home_sensor_connector_index);
  return state.home_sensor_active_high ? raw_state : !raw_state;
}

void SetBridgeBrakePinState(const ClearCorePins pin, const bool asserted) {
  if (pin == CLEARCORE_PIN_INVALID) {
    return;
  }
  Connector *const out = SysMgr.ConnectorByIndex(pin);
  if (out == nullptr) {
    return;
  }
  if (out->Type() == Connector::CCIO_DIGITAL_IN_OUT_TYPE) {
    CcioMgr.PinState(pin, asserted);
    return;
  }
  if (out->Mode() != Connector::OUTPUT_DIGITAL) {
    (void)out->Mode(Connector::OUTPUT_DIGITAL);
  }
  out->State(asserted ? 1 : 0);
}

void UpdateBridgeBrakeOutput(MotorDriver &motor,
                             const StepDirMotorBridgeState &state) {
  if (state.brake_output_pin == CLEARCORE_PIN_INVALID) {
    return;
  }
  const bool brake_enable = (motor.StatusReg().bit.Enabled != 0U);
  SetBridgeBrakePinState(state.brake_output_pin, brake_enable);
}

ClearCorePins ClearlinkBoardIoPinFromConnectorIndex(int16_t idx) {
  if (idx < 0) {
    return CLEARCORE_PIN_INVALID;
  }
  if (idx <= 12) {
    return static_cast<ClearCorePins>(static_cast<int>(CLEARCORE_PIN_IO0) +
                                      static_cast<int>(idx));
  }
  /* CCIO-8 inputs: map 13..76 to CCIOA0..CCIOH7 (64 connectors). */
  if (idx <= 76) {
    const int ord = static_cast<int>(idx) - 13;
    const int base = static_cast<int>(CLEARCORE_PIN_CCIO_BASE);
    const int last_ccio = static_cast<int>(CLEARCORE_PIN_CCIO_MAX) - 1;
    const int pin_index = base + ord;
    if (pin_index > last_ccio) {
      return CLEARCORE_PIN_INVALID;
    }
    return static_cast<ClearCorePins>(pin_index);
  }
  return CLEARCORE_PIN_INVALID;
}

uint32_t MapMotorAlertRegToTeknicShutdownLayout(const uint32_t alert_reg) {
  MotorDriver::AlertRegMotor ar;
  ar.reg = alert_reg;
  uint32_t t = 0U;
  /* ClearLink object reference Table 25 (Motor Shutdowns). */
  if (ar.bit.MotionCanceledInAlert != 0U) {
    t |= (1U << 0);
  }
  if (ar.bit.MotionCanceledPositiveLimit != 0U) {
    t |= (1U << 1);
  }
  if (ar.bit.MotionCanceledNegativeLimit != 0U) {
    t |= (1U << 2);
  }
  if (ar.bit.MotionCanceledSensorEStop != 0U) {
    t |= (1U << 3);
  }
  /* Bit 4 SW E-Stop: host-latched in ApplyStepDirOutputs when O2T bit 5 rises (see Poll). */
  if (ar.bit.MotionCanceledMotorDisabled != 0U) {
    t |= (1U << 5);
  }
  if (ar.bit.MotorFaulted != 0U) {
    t |= (1U << 10);
  }
  return t;
}

uint32_t MapMotorStatusToTeknicLayout(MotorDriver &motor) {
  MotorDriver::StatusRegMotor sr;
  sr.reg = motor.StatusReg().reg;
  uint32_t t = 0U;
  /* ClearLink object reference Table 24 (Motor Status Register). */
  if (sr.bit.AtTargetPosition != 0U) {
    t |= (1U << 0);
  }
  if (sr.bit.StepsActive != 0U) {
    t |= (1U << 1);
  }
  if (sr.bit.AtTargetVelocity != 0U) {
    t |= (1U << 2);
  }
  if (sr.bit.MoveDirection != 0U) {
    t |= (1U << 3);
  }
  if (sr.bit.InPositiveLimit != 0U) {
    t |= (1U << 4);
  }
  if (sr.bit.InNegativeLimit != 0U) {
    t |= (1U << 5);
  }
  if (sr.bit.InEStopSensor != 0U) {
    t |= (1U << 6);
  }
  if (sr.bit.MotorInFault != 0U) {
    t |= (1U << 9);
  }
  if (sr.bit.Enabled != 0U) {
    t |= (1U << 10);
  }
  if (sr.bit.PositionalMove != 0U) {
    t |= (1U << 12);
  }
  if (sr.bit.AlertsPresent != 0U) {
    t |= (1U << 17);
  }
  const MotorDriver::HlfbStates hs = motor.HlfbState();
  if (hs == MotorDriver::HLFB_ASSERTED) {
    t |= (1U << 14);
  }
  if (hs == MotorDriver::HLFB_HAS_MEASUREMENT) {
    t |= (1U << 15);
  }
  return t;
}

} /* namespace */

void ClearLinkMotor_Init(void) {
  std::memset(g_step_dir_bridge, 0, sizeof(g_step_dir_bridge));
  std::memset(g_motor_estop_decel_cap_steps_s2, 0,
              sizeof(g_motor_estop_decel_cap_steps_s2));
  for (unsigned i = 0; i < 4U; ++i) {
    g_step_dir_bridge[i].brake_output_pin = CLEARCORE_PIN_INVALID;
  }
}

void ClearLinkMotor_ApplyStepDirOutputs(const unsigned axis,
                                        const int32_t move_distance,
                                        const int32_t jog_velocity,
                                        const uint32_t vel_limit,
                                        const uint32_t accel_limit,
                                        const uint32_t decel_limit,
                                        const uint32_t output_register) {
  if (axis >= 4U) {
    return;
  }
  MotorDriver *const motor = MotorAxisPtr(axis);
  if (motor == nullptr) {
    return;
  }
  if (!BoardMotorMode_Request(1)) {
    return;
  }
  StepDirMotorBridgeState *const st = &g_step_dir_bridge[axis];
  const uint32_t prev = st->prev_output_register;
  const bool homing_move_request = (output_register & (1U << 2)) != 0U;
  const bool homing_supported =
      st->homing_enabled && (st->home_sensor_connector_index >= 0);
  const bool homing_commanded = homing_move_request && homing_supported;

  motor->EnableRequest((output_register & 1U) != 0U);

  const uint32_t vlim = ClampVelocityLimit(vel_limit);
  uint32_t accel = ClampAccel(accel_limit);
  uint32_t decel = decel_limit;
  if (decel == 0U) {
    decel = accel;
  } else {
    decel = ClampAccel(decel);
  }
  const uint32_t decel_cap = g_motor_estop_decel_cap_steps_s2[axis];
  if (decel_cap != 0U && decel > decel_cap) {
    decel = decel_cap;
  }
  motor->VelMax(vlim);
  motor->AccelMax(accel);
  motor->EStopDecelMax(decel);
  UpdateBridgeBrakeOutput(*motor, *st);

  /*
   * Table 28: process Clear Alerts (6) / Clear Motor Fault (7) before SW E-Stop (5) early
   * exit so a host can clear shutdowns while the E-Stop bit is still asserted.
   */
  if (((output_register & (1U << 6)) != 0U) && ((prev & (1U << 6)) == 0U)) {
    motor->ClearAlerts(UINT32_MAX);
    st->shutdown_sw_estop_latched = false;
    st->shutdown_command_while_shutdown_latched = false;
    st->shutdown_motor_disabled_latched = false;
    st->shutdown_soft_limit_exceeded_latched = false;
    st->pos_move_accepted_while_load_high = false;
    st->vel_move_accepted_while_load_high = false;
  }

  if (((output_register & (1U << 7)) != 0U) && ((prev & (1U << 7)) == 0U)) {
    motor->ClearFaults(10U, 25U);
    st->shutdown_command_while_shutdown_latched = false;
    st->shutdown_motor_disabled_latched = false;
    st->pos_move_accepted_while_load_high = false;
    st->vel_move_accepted_while_load_high = false;
  }

  if ((output_register & (1U << 5)) != 0U && (prev & (1U << 5)) == 0U) {
    st->shutdown_sw_estop_latched = true;
  }

  if ((output_register & (1U << 5)) != 0U) {
    motor->MoveStopDecel(decel);
    st->homing_move_active = false;
    UpdateBridgeBrakeOutput(*motor, *st);
    st->prev_output_register = output_register;
    return;
  }

  if ((homing_commanded || st->homing_move_active) && HomeSensorActive(*st)) {
    motor->MoveStopDecel(decel);
    motor->PositionRefSet(0);
    st->last_target_position_steps = 0;
    st->homing_move_active = false;
    UpdateBridgeBrakeOutput(*motor, *st);
    st->prev_output_register = output_register;
    return;
  }

  const bool clear_command_active =
      ((output_register & (1U << 6)) != 0U) ||
      ((output_register & (1U << 7)) != 0U);
  const bool pos_load =
      !clear_command_active && ((output_register & (1U << 3)) != 0U);
  const bool vel_load =
      !clear_command_active && ((output_register & (1U << 4)) != 0U);
  const bool pos_was_high = (prev & (1U << 3)) != 0U;
  const bool vel_was_high = (prev & (1U << 4)) != 0U;

  if (!pos_load) {
    st->pos_move_accepted_while_load_high = false;
  }
  if (!vel_load) {
    st->vel_move_accepted_while_load_high = false;
  }

  const bool pos_rising = pos_load && !pos_was_high;
  const bool vel_rising = vel_load && !vel_was_high;

  const bool pos_ack_blocking =
      st->pos_move_accepted_while_load_high && pos_load;
  const bool vel_ack_blocking =
      st->vel_move_accepted_while_load_high && vel_load;
  const bool move_command_requested =
      (pos_rising || vel_rising) && !pos_ack_blocking && !vel_ack_blocking;
  const bool motor_enabled =
      (motor->StatusReg().bit.Enabled != 0U) && ((output_register & 1U) != 0U);

  if (move_command_requested && !motor_enabled) {
    st->shutdown_command_while_shutdown_latched = true;
    st->shutdown_motor_disabled_latched = true;
  }

  if (pos_rising && !pos_ack_blocking && !vel_ack_blocking && motor_enabled) {
    /* Table 28 bit 2 (Homing Move): same Move path; Teknic homing completion /
     * position zeroing is not replicated — host supplies approach target. */
    const bool absolute = (output_register & (1U << 1)) != 0U;
    bool allow_move = true;
    if (st->soft_limits_enabled) {
      if (absolute) {
        if (move_distance < st->soft_limit_lo_steps ||
            move_distance > st->soft_limit_hi_steps) {
          allow_move = false;
        }
      } else {
        /* Approximate endpoint vs current commanded position (see Move() REL semantics). */
        const int32_t ref = motor->PositionRefCommanded();
        const int32_t tentative_end = ref + move_distance;
        if (tentative_end < st->soft_limit_lo_steps ||
            tentative_end > st->soft_limit_hi_steps) {
          allow_move = false;
        }
      }
    }
    if (allow_move) {
      const bool moved =
          motor->Move(move_distance,
                      absolute ? ClearCore::StepGenerator::MOVE_TARGET_ABSOLUTE
                               : ClearCore::StepGenerator::MOVE_TARGET_REL_END_POSN);
      if (moved) {
        st->pos_move_accepted_while_load_high = true;
        st->last_target_position_steps = move_distance;
        st->last_target_velocity_steps_per_s = static_cast<int32_t>(vlim);
        st->homing_move_active = homing_commanded;
      }
    } else if (st->soft_limits_enabled) {
      st->shutdown_soft_limit_exceeded_latched = true;
    }
  } else if (vel_rising && !pos_ack_blocking && !vel_ack_blocking && motor_enabled) {
    const bool moved_vel = motor->MoveVelocity(jog_velocity);
    if (moved_vel) {
      st->vel_move_accepted_while_load_high = true;
      st->last_target_velocity_steps_per_s = jog_velocity;
      st->homing_move_active = homing_commanded;
    }
  }

  if (!homing_move_request && motor->StatusReg().bit.StepsActive == 0U) {
    st->homing_move_active = false;
  }

  UpdateBridgeBrakeOutput(*motor, *st);

  st->prev_output_register = output_register;
}

void ClearLinkMotor_AddToPosition(const unsigned axis, const int32_t delta_steps) {
  if (axis >= 4U || delta_steps == 0) {
    return;
  }
  MotorDriver *const motor = MotorAxisPtr(axis);
  if (motor == nullptr) {
    return;
  }
  motor->PositionRefSet(motor->PositionRefCommanded() + delta_steps);
  UpdateBridgeBrakeOutput(*motor, g_step_dir_bridge[axis]);
}

void ClearLinkMotor_ApplyMConnectorOutputs(const unsigned axis,
                                           const uint16_t trigger_pulses,
                                           const uint16_t a_pwm,
                                           const uint16_t b_pwm,
                                           const uint8_t mc_output_byte,
                                           const uint16_t enable_pulse_time_ms) {
  if (axis >= 4U) {
    return;
  }
  MotorDriver *const motor = MotorAxisPtr(axis);
  if (motor == nullptr) {
    return;
  }
  motor->EnableRequest((mc_output_byte & 1U) != 0U);
  uint32_t pulse_ms = enable_pulse_time_ms;
  if (pulse_ms == 0U) {
    pulse_ms = 2U;
  }
  if (pulse_ms > 5000U) {
    pulse_ms = 5000U;
  }

  const bool output_a_forced_high = (mc_output_byte & (1U << 1)) != 0U;
  const bool output_b_forced_high = (mc_output_byte & (1U << 2)) != 0U;
  const bool disable_pulse = (mc_output_byte & (1U << 3)) != 0U;

  if (trigger_pulses > 0U && motor->EnableRequest() && !disable_pulse) {
    motor->EnableTriggerPulse(trigger_pulses,
                              static_cast<uint32_t>(pulse_ms),
                              false);
  }

  // In A_PWM_B_PWM mode:
  //  - Both channels accept PWM duty.
  //  - Raw CIP values are 12-bit (0..4095), mapped to 0..255.
  //  - Output A/B bits act as channel enables / direct-force-high when PWM is zero.
  auto raw_pwm_to_duty_8bit = [](const uint16_t raw) -> uint8_t {
    uint32_t clamped = raw;
    if (clamped > 4095U) {
      clamped = 4095U;
    }
    const uint32_t duty = (clamped * 255U + 2047U) / 4095U;
    return static_cast<uint8_t>(duty > 255U ? 255U : duty);
  };

  const bool a_enabled = output_a_forced_high || (a_pwm > 0U);
  const bool b_enabled = output_b_forced_high || (b_pwm > 0U);
  const uint8_t a_duty = a_enabled
      ? raw_pwm_to_duty_8bit((a_pwm == 0U) ? 4095U : a_pwm)
      : static_cast<uint8_t>(0U);
  const uint8_t b_duty = b_enabled
      ? raw_pwm_to_duty_8bit((b_pwm == 0U) ? 4095U : b_pwm)
      : static_cast<uint8_t>(0U);
  (void)motor->MotorInAState(a_enabled);
  (void)motor->MotorInBState(b_enabled);
  (void)motor->MotorInADuty(a_duty);
  (void)motor->MotorInBDuty(b_duty);
}

int ClearLinkMotor_IsIoReservedForBrakeOutput(const uint8_t io_index) {
  if (io_index >= 6U) {
    return 0;
  }
  const ClearCorePins io_pin =
      static_cast<ClearCorePins>(static_cast<int>(CLEARCORE_PIN_IO0) +
                                 static_cast<int>(io_index));
  for (unsigned axis = 0U; axis < 4U; ++axis) {
    if (g_step_dir_bridge[axis].brake_output_pin == io_pin) {
      return 1;
    }
  }
  return 0;
}

void ClearLinkMotor_ApplyMConnectorRouting(const unsigned axis,
                                           const int16_t enable_connector_index,
                                           const int16_t input_a_connector_index,
                                           const int16_t input_b_connector_index) {
  if (axis >= 4U) {
    return;
  }
  MotorDriver *const motor = MotorAxisPtr(axis);
  if (motor == nullptr) {
    return;
  }
  const ClearCorePins en = ClearlinkBoardIoPinFromConnectorIndex(enable_connector_index);
  const ClearCorePins in_a = ClearlinkBoardIoPinFromConnectorIndex(input_a_connector_index);
  const ClearCorePins in_b = ClearlinkBoardIoPinFromConnectorIndex(input_b_connector_index);
  if (enable_connector_index < 0) {
    (void)motor->EnableConnector(CLEARCORE_PIN_INVALID);
  } else {
    (void)motor->EnableConnector(en);
  }
  if (input_a_connector_index < 0) {
    (void)motor->InputAConnector(CLEARCORE_PIN_INVALID);
  } else {
    (void)motor->InputAConnector(in_a);
  }
  if (input_b_connector_index < 0) {
    (void)motor->InputBConnector(CLEARCORE_PIN_INVALID);
  } else {
    (void)motor->InputBConnector(in_b);
  }
}

void ClearLinkMotor_ApplyMotorConfiguration(
    const unsigned axis,
    const uint32_t config_register,
    const int32_t soft_limit_1,
    const int32_t soft_limit_2,
    const int16_t positive_limit_connector_index,
    const int16_t negative_limit_connector_index,
    const int16_t home_sensor_connector_index,
    const int16_t brake_output_connector_index,
    const int16_t stop_sensor_connector_index,
    const int32_t max_deceleration_rate_steps_per_sec2) {
  if (axis >= 4U) {
    return;
  }
  MotorDriver *const motor = MotorAxisPtr(axis);
  if (motor == nullptr) {
    return;
  }
  StepDirMotorBridgeState *const st = &g_step_dir_bridge[axis];
  /*
   * Class 0x64 attr 1 / assembly 150: Table 21 Config Register (ClearLink object reference).
   * Bits 0–1: homing enable / home sensor active level (not applied to MotorDriver yet).
   * Bit 2: enable inversion; bit 3: HLFB inversion; bit 4: position capture active level;
   * bit 5: soft limit enable. Bits 6–31 reserved. Step/direction polarity has no documented
   * direction-invert bit here — direction invert is forced off.
   */
  const bool soft_limits_bit = (config_register & (1U << 5)) != 0U;
  const bool limits_distinct = (soft_limit_1 != soft_limit_2);
  if (!limits_distinct || !soft_limits_bit) {
    st->soft_limits_enabled = false;
    st->soft_limit_lo_steps = 0;
    st->soft_limit_hi_steps = 0;
  } else {
    st->soft_limits_enabled = true;
    st->soft_limit_lo_steps =
        (std::min)(soft_limit_1, soft_limit_2);
    st->soft_limit_hi_steps =
        (std::max)(soft_limit_1, soft_limit_2);
  }
  st->homing_enabled = (config_register & 1U) != 0U;
  st->home_sensor_active_high = ((config_register >> 1) & 1U) != 0U;
  st->home_sensor_connector_index = home_sensor_connector_index;
  if (!st->homing_enabled || st->home_sensor_connector_index < 0) {
    st->homing_move_active = false;
  }

  (void)motor->PolarityInvertSDEnable(((config_register >> 2) & 1U) != 0U);
  (void)motor->PolarityInvertSDDirection(false);
  /*
   * ClearLink host config bit 3 is treated as HLFB active-high selection.
   * MotorDriver::PolarityInvertSDHlfb(true) means active-low, so invert the bit
   * when applying to the underlying driver polarity flag.
   */
  (void)motor->PolarityInvertSDHlfb(((config_register >> 3) & 1U) == 0U);

  const ClearCorePins pos_lim =
      ClearlinkBoardIoPinFromConnectorIndex(positive_limit_connector_index);
  const ClearCorePins neg_lim =
      ClearlinkBoardIoPinFromConnectorIndex(negative_limit_connector_index);
  const ClearCorePins brake_pin =
      ClearlinkBoardIoPinFromConnectorIndex(brake_output_connector_index);
  const ClearCorePins estop_pin =
      ClearlinkBoardIoPinFromConnectorIndex(stop_sensor_connector_index);

  if (positive_limit_connector_index < 0) {
    (void)motor->LimitSwitchPos(CLEARCORE_PIN_INVALID);
  } else {
    (void)motor->LimitSwitchPos(pos_lim);
  }
  if (negative_limit_connector_index < 0) {
    (void)motor->LimitSwitchNeg(CLEARCORE_PIN_INVALID);
  } else {
    (void)motor->LimitSwitchNeg(neg_lim);
  }
  (void)motor->BrakeOutput(CLEARCORE_PIN_INVALID);
  if (st->brake_output_pin != CLEARCORE_PIN_INVALID &&
      st->brake_output_pin != brake_pin) {
    SetBridgeBrakePinState(st->brake_output_pin, false);
  }
  if (brake_output_connector_index < 0) {
    st->brake_output_pin = CLEARCORE_PIN_INVALID;
  } else {
    st->brake_output_pin = brake_pin;
    SetBridgeBrakePinState(st->brake_output_pin, false);
  }
  if (stop_sensor_connector_index < 0) {
    (void)motor->EStopConnector(CLEARCORE_PIN_INVALID);
  } else {
    (void)motor->EStopConnector(estop_pin);
  }
  if (max_deceleration_rate_steps_per_sec2 > 0) {
    g_motor_estop_decel_cap_steps_s2[axis] = ClampAccel(static_cast<uint32_t>(
        max_deceleration_rate_steps_per_sec2));
  } else {
    g_motor_estop_decel_cap_steps_s2[axis] = 0U;
  }
}

void ClearLinkMotor_PollStepDirFeedback(const unsigned axis,
                                        const uint32_t extra_status_bits,
                                        int32_t *commanded_position,
                                        int32_t *commanded_velocity,
                                        int32_t *target_position,
                                        int32_t *target_velocity,
                                        float *measured_torque,
                                        uint32_t *motor_status,
                                        uint32_t *motor_shutdowns) {
  if (axis >= 4U || commanded_position == nullptr ||
      commanded_velocity == nullptr || target_position == nullptr ||
      target_velocity == nullptr || measured_torque == nullptr ||
      motor_status == nullptr || motor_shutdowns == nullptr) {
    return;
  }
  MotorDriver *const motor = MotorAxisPtr(axis);
  if (motor == nullptr) {
    return;
  }
  StepDirMotorBridgeState *const st = &g_step_dir_bridge[axis];

  const bool homing_supported =
      st->homing_enabled && (st->home_sensor_connector_index >= 0);
  const bool homing_commanded = (st->prev_output_register & (1U << 2)) != 0U;
  if (homing_supported &&
      (st->homing_move_active || homing_commanded) &&
      HomeSensorActive(*st)) {
    /*
     * Homing sensor is asynchronous to O2T updates. Enforce stop+zero from the
     * feedback poll path so a held-constant output image still terminates homing.
     */
    motor->MoveStopAbrupt();
    motor->PositionRefSet(0);
    st->last_target_position_steps = 0;
    st->homing_move_active = false;
  }

  /* Keep brake output synchronized continuously, not only on output-register edges. */
  UpdateBridgeBrakeOutput(*motor, *st);

  *commanded_position = motor->PositionRefCommanded();
  *commanded_velocity = motor->VelocityRefCommanded();
  *target_position = st->last_target_position_steps;
  *target_velocity = st->last_target_velocity_steps_per_s;

  const float duty = motor->HlfbPercent();
  if (duty <= static_cast<float>(MotorDriver::HLFB_DUTY_UNKNOWN + 1)) {
    *measured_torque = -9999.0F;
  } else {
    *measured_torque = duty;
  }

  uint32_t sts = MapMotorStatusToTeknicLayout(*motor);
  if (st->homing_move_active) {
    sts |= (1U << 8);
  }
  /* Table 24: expose "In Home Sensor" from configured home connector + active-level setting. */
  if (st->home_sensor_connector_index >= 0 && HomeSensorActive(*st)) {
    sts |= (1U << 7);
  }
  sts |= extra_status_bits;
  if (st->pos_move_accepted_while_load_high) {
    sts |= (1U << 19);
  }
  if (st->vel_move_accepted_while_load_high) {
    sts |= (1U << 20);
  }
  *motor_status = sts;
  {
    uint32_t sh =
        MapMotorAlertRegToTeknicShutdownLayout(motor->AlertReg().reg);
    if (st->shutdown_sw_estop_latched) {
      sh |= (1U << 4);
    }
    if (st->shutdown_command_while_shutdown_latched) {
      sh |= (1U << 0);
    }
    if (st->shutdown_motor_disabled_latched) {
      sh |= (1U << 5);
    }
    if (st->shutdown_soft_limit_exceeded_latched) {
      sh |= (1U << 6);
    }
    *motor_shutdowns = sh;
  }
}

void ClearLinkMotor_PollMConnectorFeedback(const unsigned axis,
                                           float *hlfb_duty_percent,
                                           uint16_t *status_word) {
  if (axis >= 4U || hlfb_duty_percent == nullptr || status_word == nullptr) {
    return;
  }
  MotorDriver *const motor = MotorAxisPtr(axis);
  if (motor == nullptr) {
    return;
  }
  const float duty = motor->HlfbPercent();
  const MotorDriver::HlfbStates hs = motor->HlfbState();
  const bool has_pwm_measurement =
      duty > static_cast<float>(MotorDriver::HLFB_DUTY_UNKNOWN + 1) &&
      duty >= -100.0f && duty <= 100.0f;
  if (duty <= static_cast<float>(MotorDriver::HLFB_DUTY_UNKNOWN + 1)) {
    *hlfb_duty_percent = -9999.0F;
  } else {
    *hlfb_duty_percent = duty;
  }
  uint16_t status = 0U;
  if (hs == MotorDriver::HLFB_ASSERTED) {
    status |= (1U << 0);  // HLFB_On
  }
  if (hs == MotorDriver::HLFB_HAS_MEASUREMENT || has_pwm_measurement) {
    status |= (1U << 1);  // Has PWM (482 Hz carrier mode)
  }
  *status_word = status;
}

int BoardMotorMode_Request(int step_and_dir_nonzero) {
  ClearCore::Connector::ConnectorModes const mode =
      step_and_dir_nonzero != 0
          ? ClearCore::Connector::CPM_MODE_STEP_AND_DIR
          : ClearCore::Connector::CPM_MODE_A_PWM_B_PWM;
  const bool applied = MotorMgr.MotorModeSet(MotorManager::MOTOR_ALL, mode);
  if (!applied) {
    return 0;
  }
  // Both personalities can use PWM HLFB measurement for consistent telemetry.
  for (unsigned axis = 0U; axis < 4U; ++axis) {
    MotorDriver *const motor = MotorAxisPtr(axis);
    if (motor == nullptr) {
      continue;
    }
    (void)motor->HlfbMode(MotorDriver::HLFB_MODE_HAS_BIPOLAR_PWM);
    (void)motor->HlfbCarrier(MotorDriver::HLFB_CARRIER_482_HZ);
  }
  return 1;
}

void BoardVoltageSamples(float *auxiliary_volts, float *supply_volts) {
  if (auxiliary_volts == nullptr || supply_volts == nullptr) {
    return;
  }
  /* ADC_VSUPPLY_MON: main rail; ADC_5VOB_MON: 5 V off-board (auxiliary) per Teknic naming. */
  *supply_volts = AdcMgr.AnalogVoltage(AdcManager::ADC_VSUPPLY_MON);
  *auxiliary_volts = AdcMgr.AnalogVoltage(AdcManager::ADC_5VOB_MON);
}

void AsciiCom0_RefreshMirrors(uint32_t *baud_udint,
                             uint32_t *status_udint) {
  if (baud_udint == nullptr || status_udint == nullptr) {
    return;
  }
  ClearCore::SerialDriver &ser = ConnectorCOM0;
  /* SerialDriver::Speed(uint32_t) hides SerialBase::Speed() — use qualified getter. */
  *baud_udint = ser.SerialBase::Speed();
  uint32_t sts = ser.PortIsOpen() ? 1U : 0U;
  if (ser.PortIsOpen() && ser.CtsState()) {
    sts |= (1U << 1);
  }
  /*
   * Omit framing / parity / overflow reflection here: ErrorStatusAccum() clears
   * latched faults on read. Those bits are cleared explicitly via Teknic config
   * register bit 17 (Clear Errors) in AsciiCom0_ApplyCommitted().
   */
  *status_udint = sts;
}

void AsciiCom0_ApplyCommitted(uint32_t config_register_udint,
                              uint32_t baud_udint) {
  ClearCore::SerialDriver &ser = ConnectorCOM0;
  bool const want_open = (config_register_udint & 1U) != 0U;
  bool const use_rs232 = (config_register_udint & 2U) != 0U;
  bool const flush_in = ((config_register_udint >> 18) & 1U) != 0U;

  (void)ser.Speed(baud_udint);

  if (want_open) {
    (void)ser.Mode(use_rs232 ? ClearCore::Connector::RS232
                             : ClearCore::Connector::TTL);
    ser.PortOpen();
  } else {
    ser.PortClose();
  }

  if (((config_register_udint >> 17) & 1U) != 0U) {
    ClearCore::SerialBase::SerialErrorStatusRegister const all_ones(UINT32_MAX);
    (void)ser.ErrorStatusAccum(all_ones);
  }
  if (flush_in) {
    ser.FlushInput();
  }
}

void AsciiCom0_ReadRxIntoBuffer(uint8_t *dst,
                                size_t dst_capacity,
                                uint32_t *bytes_read_out) {
  if (dst == nullptr || bytes_read_out == nullptr) {
    return;
  }
  *bytes_read_out = 0U;
  if (dst_capacity > 0U) {
    std::memset(dst, 0, dst_capacity);
  }

  ClearCore::SerialDriver &ser = ConnectorCOM0;
  if (!ser.PortIsOpen()) {
    return;
  }

  int32_t avail = ser.AvailableForRead();
  if (avail < 0) {
    avail = 0;
  }

  size_t n = static_cast<size_t>(avail);
  if (n > dst_capacity) {
    n = dst_capacity;
  }

  size_t got = 0U;
  while (got < n) {
    const int16_t c = ser.CharGet();
    if (c < 0) {
      break;
    }
    dst[got++] = static_cast<uint8_t>(static_cast<uint16_t>(c) & 0xFFU);
  }
  *bytes_read_out = static_cast<uint32_t>(got);
}

void AsciiCom0_SendTxBytes(const uint8_t *src, size_t byte_count) {
  if (src == nullptr || byte_count == 0U) {
    return;
  }
  ClearCore::SerialDriver &ser = ConnectorCOM0;
  if (!ser.PortIsOpen()) {
    return;
  }
  (void)ser.Send(reinterpret_cast<const char *>(src), byte_count);
}

void BoardApplyBlinkFiveBytes(const uint8_t data[5]) {
  /* UI group index matching StatusMgr.BlinkCodeClear (1-based): Application = fifth group. */
  constexpr uint8_t kApplicationBlinkGroupUi = 5U;
  for (uint8_t c = 1U; c <= 8U; ++c) {
    StatusMgr.BlinkCodeClear(kApplicationBlinkGroupUi, c);
  }
  if (data[0] != 0U) {
    StatusMgr.UserBlinkCode(data[0]);
  }
}

void BoardBlinkCodesSnapshotFiveBytes(uint8_t data[5]) {
  if (data == nullptr) {
    return;
  }
  StatusMgr.BlinkCodeGroupMaskSnapshot(data, 5U);
}

void BoardBlinkCodes_ResetLatchingBlinkCodes(void) {
  constexpr uint8_t kMaxUiGroup =
      static_cast<uint8_t>(ClearCore::BlinkCodeDriver::BLINK_GROUP_MAX);
  for (uint8_t group = 1U; group <= kMaxUiGroup; ++group) {
    for (uint8_t code = 1U; code <= 8U; ++code) {
      StatusMgr.BlinkCodeClear(group, code);
    }
  }
}

} /* extern "C" */

#endif /* CLEARCORE */

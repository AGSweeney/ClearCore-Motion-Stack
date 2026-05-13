/*******************************************************************************
 * Originally based on OpENer sample application
 * Copyright (c) 2012, Rockwell Automation, Inc.
 * All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause (adapted, see OpENer/license.txt)
 *
 * ClearLink Compatibility Firmware modifications and additions:
 * Copyright (c) 2025 Adam G. Sweeney <agsweeney@gmail.com>
 * SPDX-License-Identifier: MIT
 *
 ******************************************************************************/

/*******************************************************************************
 * ClearLink Compatibility Firmware -- EtherNet/IP Assembly Definitions
 *
 * Assembly byte layout: ExternalReferances/clearlink_ethernet-ip_object_reference.pdf
 * (summarized in docs/ASSEMBLY_LAYOUT.md). Parity instances and sizes:
 *   100 (T2O): 332, 101 (T2O): 228
 *   112 (O2T): 280, 113 (O2T): 200
 *   150 (CFG): 232, 151 (CFG): 120
 ******************************************************************************/

#include <string.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

#include "opener_api.h"
#include "appcontype.h"
#include "trace.h"
#include "cipcommon.h"
#include "cipidentity.h"
#include "ciptcpipinterface.h"
#include "cipqos.h"
#include "ports/ClearCore/clearcore_wrapper.h"
#include "ports/ClearCore/clearcore_clearlink_bridge.h"
#include "cipconnectionmanager.h"
#include "cipethernetlink.h"
#include "ports/ClearCore/clearlink_compatibility_firmware/ethlinkcbs.h"
#include "ports/nvdata/nvdata.h"
#include "endianconv.h"

/* ---- Assembly instance numbers ---- */
#define kAssemblyInputStepDirInstance               100
#define kAssemblyInputMConnectorInstance            101
#define kAssemblyOutputStepDirInstance              112
#define kAssemblyOutputMConnectorInstance           113
#define kAssemblyConfigStepDirInstance              150
#define kAssemblyConfigMConnectorInstance           151

/* ---- Parity assembly sizes (docs/ASSEMBLY_LAYOUT.md) ---- */
#define kAssemblyInputStepDirSize                   332
#define kAssemblyInputMConnectorSize                228
#define kAssemblyOutputStepDirSize                  280
#define kAssemblyOutputMConnectorSize               200
#define kAssemblyConfigStepDirSize                  232
#define kAssemblyConfigMConnectorSize               120

/* ---- Assembly data buffers ---- */
EipUint8 g_assembly_data_input_stepdir[kAssemblyInputStepDirSize];
EipUint8 g_assembly_data_input_mconnector[kAssemblyInputMConnectorSize];
EipUint8 g_assembly_data_output_stepdir[kAssemblyOutputStepDirSize];
EipUint8 g_assembly_data_output_mconnector[kAssemblyOutputMConnectorSize];
EipUint8 g_assembly_data_config_stepdir[kAssemblyConfigStepDirSize];
EipUint8 g_assembly_data_config_mconnector[kAssemblyConfigMConnectorSize];

/* ---- EIP scanner connection state (accessed from clearcore_wrapper) ---- */
volatile int g_eip_scanner_connected = 0;

/* ---- Deferred reset handling for CIP Identity Reset service ---- */
typedef enum {
  kPendingResetNone = 0,
  kPendingResetWarm,
  kPendingResetFactoryDefaults
} PendingResetType;

static PendingResetType g_pending_reset = kPendingResetNone;
static unsigned long g_pending_reset_deadline_ms = 0;

static const CipUint kCipCcioObjectClassCode = 0x68U;
static const CipUint kCipClearLinkBoardObjectClassCode = 0x69U;
static const CipUint kCipAsciiSerialObjectClassCode = 0x70U;
static const CipUint kCipDiscreteInputPointClassCode = 0x08U;
static const CipUint kCipDiscreteOutputPointClassCode = 0x09U;
static const CipUint kCipAnalogInputPointClassCode = 0x0AU;
static const CipUint kCipAnalogOutputPointClassCode = 0x0BU;
static const CipUint kCipPositionSensorClassCode = 0x23U;
static const CipUint kCipMotorConfigurationClassCode = 0x64U;
static const CipUint kCipMotorInputClassCode = 0x65U;
static const CipUint kCipMotorOutputClassCode = 0x66U;
static const CipUint kCipMConnectorClassCode = 0x67U;

typedef enum {
  kClearLinkImplicitAssemblyStepDir = 0,
  kClearLinkImplicitAssemblyMConnector = 1
} ClearLinkImplicitAssemblyFamily;

/**
 * Tracks libClearCore motor connector personality (BoardMotorMode_Request).
 * MotorDriver::UpdateFast runs StepGenerator::StepsCalculated only in
 * CPM_MODE_STEP_AND_DIR; without this, assembly 112 can command Move() while
 * hardware stays in default A_DIRECT_B_DIRECT (enable works, no steps).
 */
static int s_applied_clearcore_motor_family = -1;

static void DiscreteInputRefreshInstanceAttributes(CipInstanceNum instance_number);
static void AnalogInputRefreshInstanceAttributes(CipInstanceNum instance_number);
static void DiscreteOutputRefreshInstanceAttributes(CipInstanceNum instance_number);
static void RefreshPositionSensorMirrors(void);
static void RefreshMotorInputInstance(CipInstanceNum instance_number);
static void RefreshMConnectorInstance(CipInstanceNum instance_number);
static int32_t EncoderVelocityScaledForEip(void);
static void MotorAxisApplyMotorOutputAdd(unsigned axis_idx_zero_based,
                                         CipDint cmd);
static void MotorAxisExplicitDefaultsInit(void);
static void MotorAxisApplyStoredMotorConfiguration(unsigned axis_idx_zero_based,
                                                   bool sync_stepdir_cfg150_assembly);
static void SyncStepDirMotorConfigBlockToAssembly(unsigned axis_idx_zero_based);
static void SyncMcRouting151BlockToAssembly(unsigned axis_idx_zero_based);
static void EncoderApplyAddPositionTransition(int32_t add_command);
static void MirrorSingleDipFilterToConfigAssembly(unsigned dip_idx_zero_based);
static void MirrorAiConfigurationToAssemblyBuffers(void);
static void MirrorAoConfigurationToAssemblyBuffers(void);
static void MirrorDopPwmFreqBitsToAssemblyBuffers(void);
static void MirrorEncoderVelocityResolutionToAssemblyBuffers(void);
static void MirrorEncoderConfigurationByteToAssemblyBuffers(void);
static void SyncIoOutputBuffersFromIoState(void);
static void LoadIoConfigurationFromBuffer(const EipUint8 *cfg, const size_t cfg_byte_count);
static void LoadIoOutputFromBuffer(const EipUint8 *output,
                                   ClearLinkImplicitAssemblyFamily family);
static void LoadStepDirMotorOutputsFromImplicitAssembly(const EipUint8 *output);

/* ---- Shared I/O segment offsets (docs/ASSEMBLY_LAYOUT.md) ---- */
#define kIoInDipValueOffset                       0U
#define kIoInDipStatusOffset                      2U
#define kIoInAipValueOffset                       4U
#define kIoInAiopStatusOffset                     12U
#define kIoInDopStatusOffset                      14U
#define kIoInCcioInputValueOffset                 16U
#define kIoInCcioStatusOffset                     24U
#define kIoInCcioBoardCountOffset                 32U
#define kIoInReservedOffset                       33U
#define kIoInSharedSize                           36U
#define kIoInEncoderPositionOffset                36U
#define kIoInEncoderVelocityOffset                40U
#define kIoInEncoderIndexPositionOffset           44U
#define kIoInEncoderStatusOffset                  48U

#define kIoOutAopValueOffset                      0U
#define kIoOutDopValueOffset                      2U
#define kIoOutDopPwmOffset                        4U
#define kIoOutCcioValueOffset                     12U
#define kIoOutEncoderAddPositionOffset            20U

#define kIoCfgAiRangeOffset                       0U
#define kIoCfgAoRangeOffset                       4U
#define kIoCfgDopPwmFreqOffset                    5U
#define kIoCfgAipFilterOffset                     8U
#define kIoCfgDipFilterOffset                     12U
#define kIoCfgCcioFilterOffset                    64U
#define kIoCfgEncoderVelocityResolutionOffset     72U
#define kIoCfgEncoderConfigurationOffset          77U
/** Per-motor configuration in assembly 150 (class 0x64 mirror), 32 bytes per axis. */
#define kIoCfgStepDirMotor0Offset                 80U
#define kIoCfgStepDirMotorBlockSize               32U
/** Per-motor M-connector config in assembly 151: three SINT + Trigger Pulse Time USINT (PDF). */
#define kIoCfgMcMotor0Offset                      80U
#define kIoCfgMcMotorBlockSize                    4U

#define kAiRange0To10V                            2U
#define kAiRangeDisabled                          100U

#define kAoRange4To20mA                           0U
#define kAoRange0To20mA                           2U
#define kAoRangeDisabled                          100U

#define kIoCfgBitDopPwmFrequencySelect            0x0001U
#define kIoCfgBitCcioEnable                       0x0002U
#define kEncoderCfgBitSwapDirection               0x01U
#define kEncoderCfgBitIndexInverted               0x02U
#define kEncoderCfgBitDisable                     0x80U

/** Class 0x64 / assembly 150 motor Config Register default (PDF Table 21: HLFB invert default on). */
#define kMotorCfgConfigRegisterPdfDefault         8U
/** Class 0x64 attr 9 / assembly default max deceleration (PDF Configuration Assembly 150). */
#define kMotorCfgMaxDecelerationPdfDefault        10000000

/** Teknic ClearLink defines four motor / M-connector instances (connectors 0-3). */
#define kMotorAxisInstanceCount                   4U

#define kIoInStepDirMotor0Offset                  52U
#define kIoInStepDirMotorBlockSize                32U
#define kIoInStepDirAsciiSegmentOffset            180U
#define kIoInMConnectorHlfbRealBaseOffset         52U
#define kIoInMConnectorStatusWordBaseOffset       68U
#define kIoInMConnectorAsciiSegmentOffset         76U

#define kIoOutStepDirMotor0Offset                 24U
#define kIoOutStepDirMotorBlockSize               28U
#define kIoOutMConnectorMotor0Offset              24U
#define kIoOutMConnectorMotorBlockSize            8U

#define kIoOutStepDirAsciiSegmentOffset           136U
#define kIoOutMConnectorAsciiSegmentOffset        56U

#define kAsciiImplicitInStatusRelOffset           0U
#define kAsciiImplicitInOutCharCntRelOffset       4U
#define kAsciiImplicitInInCharCntRelOffset        8U
#define kAsciiImplicitInOutSeqAckRelOffset        12U
#define kAsciiImplicitInInSizeRelOffset           16U
#define kAsciiImplicitInInSequenceRelOffset       20U
#define kAsciiImplicitInDataRelOffset             24U
#define kAsciiImplicitInDataBytes                 128U

#define kAsciiImplicitOutConfigRelOffset          0U
#define kAsciiImplicitOutInSeqAckRelOffset        4U
#define kAsciiImplicitOutOutSizeRelOffset         8U
#define kAsciiImplicitOutOutSequenceRelOffset     12U
#define kAsciiImplicitOutDataRelOffset            16U
#define kAsciiImplicitOutDataBytes                128U

typedef struct {
  EipUint8 ai_range[4];
  EipUint8 ai_filter_ms[4];
  EipUint8 ao_range;
  EipUint16 dop_value_bits;
  EipUint8 dop_pwm[6];
  EipUint8 dop_pwm_frequency_select;
  bool ccio_enabled;
  EipUint16 dip_filter_off_on_us[13];
  EipUint16 dip_filter_on_off_us[13];
  EipUint8 ccio_filter_ms[8];
  EipUint16 aop_value_command;
  EipUint64 ccio_output_bits;
  EipUint32 encoder_velocity_resolution;
  EipUint8 encoder_configuration;
  bool encoder_enabled;
  int32_t last_add_to_position_command;
  bool add_to_position_ack;
} IoRuntimeState;

static IoRuntimeState g_io_state = {
  { kAiRangeDisabled, kAiRangeDisabled, kAiRangeDisabled, kAiRangeDisabled },
  { 10U, 10U, 10U, 10U },
  kAoRangeDisabled,
  0U,
  { 0U, 0U, 0U, 0U, 0U, 0U },
  0U,
  false,
  { 1000U, 1000U, 1000U, 1000U, 1000U, 1000U, 1000U, 1000U, 1000U, 1000U, 1000U, 1000U, 1000U },
  { 1000U, 1000U, 1000U, 1000U, 1000U, 1000U, 1000U, 1000U, 1000U, 1000U, 1000U, 1000U, 1000U },
  { 10U, 10U, 10U, 10U, 10U, 10U, 10U, 10U },
  0U,
  0ULL,
  100U,
  0U,
  true,
  0,
  false
};

static EipUint16 ReadLe16(const EipUint8 *buffer, size_t offset);
static void WriteLe16(EipUint8 *buffer, size_t offset, EipUint16 value);
static void WriteLe32(EipUint8 *buffer, size_t offset, EipUint32 value);
static EipUint16 ClampAopIntToCommand(CipInt value);
static void ApplyAiConfiguration(void);
static void ApplyEncoderConfiguration(void);
static void ApplyDopPwmFrequencySelect(void);
static void ApplyOutputState(void);
static void SyncMConnectorOutput113BlockToAssembly(unsigned axis_idx_zero_based);

static EipUint64 g_ccio_input_value = 0ULL;
static EipUint64 g_ccio_status_value = 0ULL;
static EipUint8 g_ccio_board_count = 0U;
static CipByteArray g_ccio_filter_array = { 8, g_io_state.ccio_filter_ms };
static EipUint8 g_ccio_enable_attr = 0U;
static EipUint32 g_implicit_input_debug_counter = 0U;

static EipUint8 g_board_blink_codes_data[5] = { 0U, 0U, 0U, 0U, 0U };
static CipByteArray g_board_blink_codes = { 5, g_board_blink_codes_data };
/* TODO: Optional NVRAM persistence for board mode (class 0x69 motor-mode attr); see AGENTS.md. */
static CipBool g_board_motor_mode = 0;
static CipBool g_board_reset_blink_code = 0;
static CipReal g_board_aux_voltage = 0.0F;
static CipReal g_board_supply_voltage = 0.0F;

/* Teknic ASCII Object (class 0x70), instance 1 — COM-0 (see ClearLink object reference §ASCII). */
static CipUdint g_ascii_baud_attr = 115200U;
static CipDword g_ascii_config_attr = 0U;
static CipDword g_ascii_status_attr = 0U;
static CipUint g_ascii_output_buffer_length_attr = 128U;
static CipUint g_ascii_input_buffer_length_attr = 128U;
static CipDword g_ascii_input_start_delim_attr = 0U;
static CipDword g_ascii_input_end_delim_attr = 0U;
static CipDword g_ascii_output_start_delim_attr = 0U;
static CipDword g_ascii_output_end_delim_attr = 0U;
static CipUdint g_ascii_output_char_cnt_attr = 0U;
static CipUdint g_ascii_input_char_cnt_attr = 0U;
static CipUdint g_ascii_input_timeout_ms_attr = 0U;

static uint32_t g_ascii_implicit_last_applied_config = UINT32_MAX;
static uint32_t g_ascii_plc_ack_input_sequence = 0U;
static uint32_t g_ascii_implicit_input_sequence = 0U;

/* ODVA-style point / encoder objects (ClearLink EtherNet/IP object reference). */
static CipBool g_dip_value_mirror[13];
static CipBool g_dip_status_mirror[13];
static CipBool g_dop_instance_value_mirror[6];
static CipBool g_dop_instance_status_mirror[6];
static CipBool g_dop_pwm_frequency_bool = 0;
static CipInt g_aip_value_mirror[4];
static CipBool g_aip_status_mirror[4];
static CipInt g_aop_value_mirror = 0;
static CipBool g_aop_status_mirror = 1;
static CipDint g_ps_position = 0;
static CipBool g_ps_direction_toggle = 0;
static CipDint g_ps_velocity = 0;
static CipUint g_ps_velocity_format = 0x1F04U;
static CipWord g_ps_alarms = 0;
static CipWord g_ps_supported_alarms = 1U;
static CipBool g_ps_alarm_flag = 0;
static CipUsint g_ps_reserved_usint = 0U;
static CipDint g_ps_index_location = 0;
static CipBool g_ps_index_active_level = 0;
static CipDint g_ps_add_to_position = 0;
static CipBool g_ps_add_ack = 0;

/**
 * @brief Explicit messaging state for motor classes 64h-67h (four axes).
 *
 * Full Teknic motion sequencing is not executed here; attributes are stored and
 * mirrored so explicit Get/Set matches the ClearLink object reference layout.
 */
typedef struct {
  /* Step/Direction Motor Configuration (class 64h), instance attrs 1-13 */
  CipDword cfg_config_register;
  CipDint cfg_soft_limit_1;
  CipDint cfg_soft_limit_2;
  CipSint cfg_positive_limit_connector;
  CipSint cfg_negative_limit_connector;
  CipSint cfg_home_sensor_connector;
  CipSint cfg_brake_output_connector;
  CipSint cfg_position_capture_connector;
  CipDint cfg_max_deceleration_rate;
  CipSint cfg_stop_sensor_connector;
  CipSint cfg_follow_encoder_axis;
  CipDint cfg_follow_divisor;
  CipDint cfg_follow_multiplier;
  /* Motor Input (65h), attrs 1-8 */
  CipDint in_commanded_position;
  CipDint in_commanded_velocity;
  CipDint in_target_position;
  CipDint in_target_velocity;
  CipDint in_captured_position;
  CipReal in_measured_torque;
  CipDword in_motor_status;
  CipDword in_motor_shutdowns;
  /* Motor Output (66h), attrs 1-7 */
  CipDint out_move_distance;
  CipDint out_jog_velocity;
  CipUdint out_velocity_limit;
  CipUdint out_acceleration_limit;
  CipUdint out_deceleration_limit;
  CipDword out_output_register;
  CipDint out_add_to_position;
  bool out_add_position_ack;
  /* M-Connector (67h), attrs 1-10 */
  CipUsint mc_output_register;
  CipWord mc_status_register;
  CipSint mc_enable_connector_number;
  CipSint mc_a_connector_number;
  CipSint mc_b_connector_number;
  CipReal mc_hlfb_input_duty;
  CipUint mc_enable_trigger_pulses;
  CipUint mc_a_pwm_value;
  CipUint mc_b_pwm_value;
  CipUint mc_enable_pulse_time_ms;
} MotorAxisExplicitState;

static MotorAxisExplicitState g_motor_axis[kMotorAxisInstanceCount];

static void InitializeCustomClassAttributes(CipClass *class) {
  static const CipUint kLocalCipUintZero = 0U;
  CipClass *meta_class = class->class_instance.cip_class;

  InsertAttribute((CipInstance *)class, 1, kCipUint, EncodeCipUint, NULL,
                  (void *)&class->revision, kGetableSingleAndAll);
  InsertAttribute((CipInstance *)class, 2, kCipUint, EncodeCipUint, NULL,
                  (void *)&class->number_of_instances, kGetableSingleAndAll);
  InsertAttribute((CipInstance *)class, 3, kCipUint, EncodeCipUint, NULL,
                  (void *)&class->number_of_instances, kGetableSingle);
  InsertAttribute((CipInstance *)class, 4, kCipUint, EncodeCipUint, NULL,
                  (void *)&kLocalCipUintZero, kNotSetOrGetable);
  InsertAttribute((CipInstance *)class, 5, kCipUint, EncodeCipUint, NULL,
                  (void *)&kLocalCipUintZero, kNotSetOrGetable);
  InsertAttribute((CipInstance *)class, 6, kCipUint, EncodeCipUint, NULL,
                  (void *)&meta_class->highest_attribute_number, kGetableSingleAndAll);
  InsertAttribute((CipInstance *)class, 7, kCipUint, EncodeCipUint, NULL,
                  (void *)&class->highest_attribute_number, kGetableSingleAndAll);

  InsertService(meta_class, kGetAttributeAll, &GetAttributeAll, "GetAttributeAll");
  InsertService(meta_class, kGetAttributeSingle, &GetAttributeSingle, "GetAttributeSingle");
}

static int DecodeClearLinkBoardMotorModeBool(
  CipBool *const data,
  CipMessageRouterRequest *const message_router_request,
  CipMessageRouterResponse *const message_router_response) {
  const CipBool requested = GetBoolFromMessage(&message_router_request->data);

  /*
   * ClearLink host-facing board mode semantics:
   *   0 => Step/Direction
   *   1 => non-Step/Direction personality (M-connector style)
   *
   * BoardMotorMode_Request() expects the inverse polarity:
   * nonzero => Step/Direction.
   */
  if (!BoardMotorMode_Request(requested ? 0 : 1)) {
    message_router_response->general_status = kCipErrorInvalidAttributeValue;
    return 1;
  }
  *data = requested;
  message_router_response->general_status = kCipErrorSuccess;
  return 1;
}

static int DecodePositionSensorVelocityFormatUint(
    CipUint *const data,
    CipMessageRouterRequest *const message_router_request,
    CipMessageRouterResponse *const message_router_response) {
  const int decoded =
      DecodeCipUint(data, message_router_request, message_router_response);
  if (decoded < 0) {
    return decoded;
  }
  if (*data != (CipUint)0x1F04U) {
    message_router_response->general_status = kCipErrorInvalidAttributeValue;
  }
  return decoded;
}

static EipStatus ExplicitObjectPreGetCallback(CipInstance *const instance,
                                              CipAttributeStruct *const attribute,
                                              CipByte service) {
  (void)service;

  if (instance->cip_class->class_code == kCipCcioObjectClassCode &&
      instance->instance_number == 1U) {
    switch (attribute->attribute_number) {
      case 2:
        g_ccio_input_value = g_io_state.ccio_enabled ? Ccio_GetInputBits() : 0ULL;
        break;
      case 3:
        g_ccio_status_value = g_io_state.ccio_enabled ? Ccio_GetStatusBits() : 0ULL;
        break;
      case 4:
        g_ccio_board_count = g_io_state.ccio_enabled ? Ccio_GetBoardCount() : 0U;
        break;
      case 6:
        g_ccio_enable_attr = g_io_state.ccio_enabled ? 1U : 0U;
        break;
      default:
        break;
    }
  } else if (instance->cip_class->class_code == kCipAsciiSerialObjectClassCode &&
             instance->instance_number == 1U) {
    AsciiCom0_RefreshMirrors((uint32_t *)(void *)&g_ascii_baud_attr,
                             (uint32_t *)(void *)&g_ascii_status_attr);
  } else if (instance->cip_class->class_code == kCipClearLinkBoardObjectClassCode &&
             instance->instance_number == 1U) {
    if (attribute->attribute_number == 1U) {
      BoardBlinkCodesSnapshotFiveBytes(g_board_blink_codes_data);
    }
    else if (attribute->attribute_number == 4U ||
             attribute->attribute_number == 5U) {
      float auxiliary = g_board_aux_voltage;
      float supply = g_board_supply_voltage;
      BoardVoltageSamples(&auxiliary, &supply);
      g_board_aux_voltage = auxiliary;
      g_board_supply_voltage = supply;
    }
  } else if (instance->cip_class->class_code == kCipDiscreteInputPointClassCode &&
             instance->instance_number >= 1U && instance->instance_number <= 13U) {
    DiscreteInputRefreshInstanceAttributes(instance->instance_number);
  } else if (instance->cip_class->class_code == kCipDiscreteOutputPointClassCode &&
             instance->instance_number == 0U &&
             attribute->attribute_number == 100U) {
    g_dop_pwm_frequency_bool =
      (g_io_state.dop_pwm_frequency_select != 0U) ? (CipBool)1 : (CipBool)0;
  } else if (instance->cip_class->class_code == kCipDiscreteOutputPointClassCode &&
             instance->instance_number >= 1U && instance->instance_number <= 6U) {
    DiscreteOutputRefreshInstanceAttributes(instance->instance_number);
  } else if (instance->cip_class->class_code == kCipAnalogInputPointClassCode &&
             instance->instance_number >= 1U && instance->instance_number <= 4U) {
    AnalogInputRefreshInstanceAttributes(instance->instance_number);
  } else if (instance->cip_class->class_code == kCipAnalogOutputPointClassCode &&
             instance->instance_number == 1U) {
    g_aop_value_mirror = (CipInt)(int16_t)g_io_state.aop_value_command;
    g_aop_status_mirror =
      (g_io_state.ao_range == kAoRangeDisabled) ? (CipBool)1 : (CipBool)0;
  } else if (instance->cip_class->class_code == kCipPositionSensorClassCode &&
             instance->instance_number == 1U) {
    RefreshPositionSensorMirrors();
  } else if (instance->cip_class->class_code == kCipMotorInputClassCode &&
             instance->instance_number >= 1U &&
             instance->instance_number <= kMotorAxisInstanceCount) {
    RefreshMotorInputInstance(instance->instance_number);
  } else if (instance->cip_class->class_code == kCipMConnectorClassCode &&
             instance->instance_number >= 1U &&
             instance->instance_number <= kMotorAxisInstanceCount) {
    RefreshMConnectorInstance(instance->instance_number);
  }

  return kEipStatusOk;
}

static EipStatus ExplicitObjectPostSetCallback(CipInstance *const instance,
                                               CipAttributeStruct *const attribute,
                                               CipByte service) {
  (void)service;

  if (instance->cip_class->class_code == 0x04U &&
      attribute->attribute_number == 3U) {
    switch (instance->instance_number) {
      case kAssemblyOutputStepDirInstance:
        LoadIoOutputFromBuffer(g_assembly_data_output_stepdir,
                               kClearLinkImplicitAssemblyStepDir);
        break;
      case kAssemblyOutputMConnectorInstance:
        LoadIoOutputFromBuffer(g_assembly_data_output_mconnector,
                               kClearLinkImplicitAssemblyMConnector);
        break;
      case kAssemblyConfigStepDirInstance:
        LoadIoConfigurationFromBuffer(g_assembly_data_config_stepdir,
                                      sizeof(g_assembly_data_config_stepdir));
        break;
      case kAssemblyConfigMConnectorInstance:
        LoadIoConfigurationFromBuffer(g_assembly_data_config_mconnector,
                                      sizeof(g_assembly_data_config_mconnector));
        break;
      default:
        break;
    }
  } else if (instance->cip_class->class_code == kCipCcioObjectClassCode &&
      instance->instance_number == 1U) {
    if (attribute->attribute_number == 1U) {
      g_io_state.ccio_output_bits = *((EipUint64 *)attribute->data);
      if (g_io_state.ccio_enabled) {
        Ccio_SetOutputBits(g_io_state.ccio_output_bits);
      }
    } else if (attribute->attribute_number == 5U) {
      if (g_io_state.ccio_enabled) {
        for (int board = 0; board < 8; ++board) {
          Ccio_SetBoardFilterMs((uint8_t)board, g_io_state.ccio_filter_ms[board]);
        }
      }
      for (int board = 0; board < 8; ++board) {
        g_assembly_data_config_stepdir[kIoCfgCcioFilterOffset + (size_t)board] =
          g_io_state.ccio_filter_ms[board];
        g_assembly_data_config_mconnector[kIoCfgCcioFilterOffset + (size_t)board] =
          g_io_state.ccio_filter_ms[board];
      }
    } else if (attribute->attribute_number == 6U) {
      g_io_state.ccio_enabled = (g_ccio_enable_attr != 0U);
      Ccio_SetEnabled(g_io_state.ccio_enabled ? 1 : 0);
      {
        EipUint16 cfg_stepdir = ReadLe16(g_assembly_data_config_stepdir, kIoCfgDopPwmFreqOffset);
        EipUint16 cfg_mconnector = ReadLe16(g_assembly_data_config_mconnector, kIoCfgDopPwmFreqOffset);
        if (g_io_state.ccio_enabled) {
          cfg_stepdir = (EipUint16)(cfg_stepdir | kIoCfgBitCcioEnable);
          cfg_mconnector = (EipUint16)(cfg_mconnector | kIoCfgBitCcioEnable);
        } else {
          cfg_stepdir = (EipUint16)(cfg_stepdir & (EipUint16)(~kIoCfgBitCcioEnable));
          cfg_mconnector = (EipUint16)(cfg_mconnector & (EipUint16)(~kIoCfgBitCcioEnable));
        }
        WriteLe16(g_assembly_data_config_stepdir, kIoCfgDopPwmFreqOffset, cfg_stepdir);
        WriteLe16(g_assembly_data_config_mconnector, kIoCfgDopPwmFreqOffset, cfg_mconnector);
      }
    }
  } else if (instance->cip_class->class_code == kCipClearLinkBoardObjectClassCode &&
             instance->instance_number == 1U) {
    if (attribute->attribute_number == 1U) {
      BoardApplyBlinkFiveBytes(g_board_blink_codes_data);
    }
    else if (attribute->attribute_number == 3U && g_board_reset_blink_code != 0) {
      memset(g_board_blink_codes.data, 0, g_board_blink_codes.length);
      BoardBlinkCodes_ResetLatchingBlinkCodes();
      g_board_reset_blink_code = 0;
    }
  } else if (instance->cip_class->class_code == kCipAsciiSerialObjectClassCode &&
             instance->instance_number == 1U) {
    if (attribute->attribute_number == 1U || attribute->attribute_number == 2U) {
      AsciiCom0_ApplyCommitted((uint32_t)g_ascii_config_attr, (uint32_t)g_ascii_baud_attr);
    }
  } else if (instance->cip_class->class_code == kCipDiscreteInputPointClassCode &&
             instance->instance_number >= 1U && instance->instance_number <= 13U &&
             (attribute->attribute_number == 5U || attribute->attribute_number == 6U)) {
    const unsigned dip_idx = (unsigned)(instance->instance_number - 1U);
    ConnectorDipApplyFilterUs((uint8_t)dip_idx,
                              g_io_state.dip_filter_off_on_us[dip_idx],
                              g_io_state.dip_filter_on_off_us[dip_idx]);
    MirrorSingleDipFilterToConfigAssembly(dip_idx);
  } else if (instance->cip_class->class_code == kCipDiscreteOutputPointClassCode &&
             instance->instance_number == 0U && attribute->attribute_number == 100U) {
    g_io_state.dop_pwm_frequency_select =
      (g_dop_pwm_frequency_bool != (CipBool)0) ? 1U : 0U;
    MirrorDopPwmFreqBitsToAssemblyBuffers();
    ApplyDopPwmFrequencySelect();
  } else if (instance->cip_class->class_code == kCipDiscreteOutputPointClassCode &&
             instance->instance_number >= 1U && instance->instance_number <= 6U &&
             attribute->attribute_number == 3U) {
    const unsigned idx = (unsigned)(instance->instance_number - 1U);
    const EipUint16 mask = (EipUint16)(1U << idx);
    if (g_dop_instance_value_mirror[idx] != (CipBool)0) {
      g_io_state.dop_value_bits = (EipUint16)(g_io_state.dop_value_bits | mask);
    } else {
      g_io_state.dop_value_bits =
        (EipUint16)(g_io_state.dop_value_bits & (EipUint16)(~mask));
    }
    ApplyOutputState();
    SyncIoOutputBuffersFromIoState();
  } else if (instance->cip_class->class_code == kCipDiscreteOutputPointClassCode &&
             instance->instance_number >= 1U && instance->instance_number <= 6U &&
             attribute->attribute_number == 100U) {
    ApplyOutputState();
    SyncIoOutputBuffersFromIoState();
  } else if (instance->cip_class->class_code == kCipAnalogInputPointClassCode &&
             instance->instance_number >= 1U && instance->instance_number <= 4U &&
             (attribute->attribute_number == 7U ||
              attribute->attribute_number == 100U)) {
    ApplyAiConfiguration();
    MirrorAiConfigurationToAssemblyBuffers();
  } else if (instance->cip_class->class_code == kCipAnalogOutputPointClassCode &&
             instance->instance_number == 1U && attribute->attribute_number == 3U) {
    g_io_state.aop_value_command = ClampAopIntToCommand(g_aop_value_mirror);
    ApplyOutputState();
    SyncIoOutputBuffersFromIoState();
  } else if (instance->cip_class->class_code == kCipAnalogOutputPointClassCode &&
             instance->instance_number == 1U && attribute->attribute_number == 7U) {
    ApplyOutputState();
    MirrorAoConfigurationToAssemblyBuffers();
  } else if (instance->cip_class->class_code == kCipPositionSensorClassCode &&
             instance->instance_number == 1U && attribute->attribute_number == 12U) {
    if (g_ps_direction_toggle != (CipBool)0) {
      g_io_state.encoder_configuration =
        (EipUint8)(g_io_state.encoder_configuration | kEncoderCfgBitSwapDirection);
    } else {
      g_io_state.encoder_configuration =
        (EipUint8)(g_io_state.encoder_configuration &
                   (EipUint8)(~kEncoderCfgBitSwapDirection));
    }
    ApplyEncoderConfiguration();
    MirrorEncoderConfigurationByteToAssemblyBuffers();
  } else if (instance->cip_class->class_code == kCipPositionSensorClassCode &&
             instance->instance_number == 1U && attribute->attribute_number == 102U) {
    if (g_ps_index_active_level != (CipBool)0) {
      g_io_state.encoder_configuration =
        (EipUint8)(g_io_state.encoder_configuration | kEncoderCfgBitIndexInverted);
    } else {
      g_io_state.encoder_configuration =
        (EipUint8)(g_io_state.encoder_configuration &
                   (EipUint8)(~kEncoderCfgBitIndexInverted));
    }
    ApplyEncoderConfiguration();
    MirrorEncoderConfigurationByteToAssemblyBuffers();
  } else if (instance->cip_class->class_code == kCipPositionSensorClassCode &&
             instance->instance_number == 1U && attribute->attribute_number == 26U) {
    MirrorEncoderVelocityResolutionToAssemblyBuffers();
  } else if (instance->cip_class->class_code == kCipMotorConfigurationClassCode &&
             instance->instance_number >= 1U &&
             instance->instance_number <= kMotorAxisInstanceCount) {
    MotorAxisApplyStoredMotorConfiguration(
      (unsigned)(instance->instance_number - 1U), true);
  } else if (instance->cip_class->class_code == kCipPositionSensorClassCode &&
             instance->instance_number == 1U && attribute->attribute_number == 103U) {
    const CipDint cmd = *(CipDint *)attribute->data;
    WriteLe32(g_assembly_data_output_stepdir, kIoOutEncoderAddPositionOffset,
              (EipUint32)cmd);
    WriteLe32(g_assembly_data_output_mconnector, kIoOutEncoderAddPositionOffset,
              (EipUint32)cmd);
    EncoderApplyAddPositionTransition(cmd);
  } else if (instance->cip_class->class_code == kCipMotorOutputClassCode &&
             instance->instance_number >= 1U &&
             instance->instance_number <= kMotorAxisInstanceCount) {
    const unsigned ax = (unsigned)(instance->instance_number - 1U);
    MotorAxisExplicitState *const axis = &g_motor_axis[ax];
    switch (attribute->attribute_number) {
      case 6U: {
        const uint32_t raw_reg = axis->out_output_register;
        ClearLinkMotor_ApplyStepDirOutputs(ax,
                                           axis->out_move_distance,
                                           axis->out_jog_velocity,
                                           axis->out_velocity_limit,
                                           axis->out_acceleration_limit,
                                           axis->out_deceleration_limit,
                                           raw_reg);
        if ((raw_reg & (1UL << 6)) != 0UL) {
          axis->in_motor_shutdowns = 0;
          axis->out_output_register &= ~(1UL << 6);
        }
        ClearLinkMotor_ApplyStepDirOutputs(ax,
                                           axis->out_move_distance,
                                           axis->out_jog_velocity,
                                           axis->out_velocity_limit,
                                           axis->out_acceleration_limit,
                                           axis->out_deceleration_limit,
                                           axis->out_output_register);
        break;
      }
      case 7U:
        MotorAxisApplyMotorOutputAdd(ax, *(const CipDint *)attribute->data);
        break;
      default:
        break;
    }
    if (attribute->attribute_number != 6U) {
      ClearLinkMotor_ApplyStepDirOutputs(ax,
                                         axis->out_move_distance,
                                         axis->out_jog_velocity,
                                         axis->out_velocity_limit,
                                         axis->out_acceleration_limit,
                                         axis->out_deceleration_limit,
                                         axis->out_output_register);
    }
  } else if (instance->cip_class->class_code == kCipMConnectorClassCode &&
             instance->instance_number >= 1U &&
             instance->instance_number <= kMotorAxisInstanceCount) {
    const unsigned ax = (unsigned)(instance->instance_number - 1U);
    MotorAxisExplicitState *const mc = &g_motor_axis[ax];
    if (attribute->attribute_number == 3U || attribute->attribute_number == 4U ||
        attribute->attribute_number == 5U || attribute->attribute_number == 10U) {
      ClearLinkMotor_ApplyMConnectorRouting(
        ax,
        (int16_t)mc->mc_enable_connector_number,
        (int16_t)mc->mc_a_connector_number,
        (int16_t)mc->mc_b_connector_number);
      SyncMcRouting151BlockToAssembly(ax);
    }
    if (attribute->attribute_number == 1U) {
      ClearLinkMotor_ApplyMConnectorOutputs(ax,
                                            0U,
                                            (uint16_t)mc->mc_a_pwm_value,
                                            (uint16_t)mc->mc_b_pwm_value,
                                            mc->mc_output_register,
                                            (uint16_t)mc->mc_enable_pulse_time_ms);
      SyncMConnectorOutput113BlockToAssembly(ax);
    } else if (attribute->attribute_number == 7U) {
      if (mc->mc_enable_trigger_pulses != 0U) {
        mc->mc_status_register =
          (CipWord)(mc->mc_status_register | (CipWord)(1U << 3));
      } else {
        mc->mc_status_register =
          (CipWord)(mc->mc_status_register & (CipWord)(~(1U << 3)));
      }
      ClearLinkMotor_ApplyMConnectorOutputs(ax,
                                            (uint16_t)mc->mc_enable_trigger_pulses,
                                            (uint16_t)mc->mc_a_pwm_value,
                                            (uint16_t)mc->mc_b_pwm_value,
                                            mc->mc_output_register,
                                            (uint16_t)mc->mc_enable_pulse_time_ms);
      SyncMConnectorOutput113BlockToAssembly(ax);
    } else if (attribute->attribute_number == 8U ||
               attribute->attribute_number == 9U ||
               attribute->attribute_number == 10U) {
      ClearLinkMotor_ApplyMConnectorOutputs(ax,
                                            0U,
                                            (uint16_t)mc->mc_a_pwm_value,
                                            (uint16_t)mc->mc_b_pwm_value,
                                            mc->mc_output_register,
                                            (uint16_t)mc->mc_enable_pulse_time_ms);
      SyncMConnectorOutput113BlockToAssembly(ax);
    }
  }

  return kEipStatusOk;
}

static void InitializeCcioObjectClass(CipClass *class) {
  InitializeCustomClassAttributes(class);
  InsertService(class, kGetAttributeSingle, &GetAttributeSingle, "GetAttributeSingle");
  InsertService(class, kSetAttributeSingle, &SetAttributeSingle, "SetAttributeSingle");
  InsertGetSetCallback(class, ExplicitObjectPreGetCallback, kPreGetFunc);
  InsertGetSetCallback(class, ExplicitObjectPostSetCallback, kPostSetFunc);

  CipInstance *instance = GetCipInstance(class, 1);
  OPENER_ASSERT(NULL != instance);
  InsertAttribute(instance, 1, kCipUlint, EncodeCipUlint,
                  (CipAttributeDecodeFromMessage)DecodeCipUlint,
                  &g_io_state.ccio_output_bits, kSetAndGetAble | kPostSetFunc);
  InsertAttribute(instance, 2, kCipUlint, EncodeCipUlint, NULL,
                  &g_ccio_input_value, kGetableSingle | kPreGetFunc);
  InsertAttribute(instance, 3, kCipUlint, EncodeCipUlint, NULL,
                  &g_ccio_status_value, kGetableSingle | kPreGetFunc);
  InsertAttribute(instance, 4, kCipUsint, EncodeCipUsint, NULL,
                  &g_ccio_board_count, kGetableSingle | kPreGetFunc);
  InsertAttribute(instance, 5, kCipByteArray, EncodeCipByteArray,
                  (CipAttributeDecodeFromMessage)DecodeCipByteArray,
                  &g_ccio_filter_array, kSetAndGetAble | kPostSetFunc);
  InsertAttribute(instance, 6, kCipUsint, EncodeCipUsint,
                  (CipAttributeDecodeFromMessage)DecodeCipUsint,
                  &g_ccio_enable_attr, kSetAndGetAble | kPreGetFunc | kPostSetFunc);
}

/**
 * @brief Vendor EtherNet/IP Board Object (class 0x69, instance 1).
 *
 * Mirrors the Teknic ClearLink object reference wiring into libClearCore: blink patterns,
 * motor connector multiplex mode, blink reset, and analog rail probes.
 */
static void InitializeClearLinkBoardObjectClass(CipClass *class) {
  InitializeCustomClassAttributes(class);
  InsertService(class, kGetAttributeSingle, &GetAttributeSingle, "GetAttributeSingle");
  InsertService(class, kSetAttributeSingle, &SetAttributeSingle, "SetAttributeSingle");
  InsertGetSetCallback(class, ExplicitObjectPreGetCallback, kPreGetFunc);
  InsertGetSetCallback(class, ExplicitObjectPostSetCallback, kPostSetFunc);

  CipInstance *instance = GetCipInstance(class, 1);
  OPENER_ASSERT(NULL != instance);
  InsertAttribute(instance, 1, kCipByteArray, EncodeCipByteArray,
                  (CipAttributeDecodeFromMessage)DecodeCipByteArray,
                  &g_board_blink_codes, kSetAndGetAble | kPreGetFunc | kPostSetFunc);
  InsertAttribute(instance, 2, kCipBool, EncodeCipBool,
                  (CipAttributeDecodeFromMessage)DecodeClearLinkBoardMotorModeBool,
                  &g_board_motor_mode, kSetAndGetAble);
  InsertAttribute(instance, 3, kCipBool, EncodeCipBool,
                  (CipAttributeDecodeFromMessage)DecodeCipBool,
                  &g_board_reset_blink_code, kSetAndGetAble | kPostSetFunc);
  InsertAttribute(instance, 4, kCipReal, EncodeCipReal, NULL,
                  &g_board_aux_voltage, kGetableSingle | kPreGetFunc);
  InsertAttribute(instance, 5, kCipReal, EncodeCipReal, NULL,
                  &g_board_supply_voltage, kGetableSingle | kPreGetFunc);
}

/** @brief ClearLink ASCII Serial Object (class 0x70, instance 1, COM-0). */
static void InitializeAsciiSerialObjectClass(CipClass *class) {
  InitializeCustomClassAttributes(class);
  InsertService(class, kGetAttributeSingle, &GetAttributeSingle, "GetAttributeSingle");
  InsertService(class, kSetAttributeSingle, &SetAttributeSingle, "SetAttributeSingle");
  InsertGetSetCallback(class, ExplicitObjectPreGetCallback, kPreGetFunc);
  InsertGetSetCallback(class, ExplicitObjectPostSetCallback, kPostSetFunc);

  CipInstance *instance = GetCipInstance(class, 1);
  OPENER_ASSERT(NULL != instance);
  InsertAttribute(instance, 1, kCipUdint, EncodeCipUdint,
                  (CipAttributeDecodeFromMessage)DecodeCipUdint,
                  &g_ascii_baud_attr, kSetAndGetAble | kPreGetFunc | kPostSetFunc);
  InsertAttribute(instance, 2, kCipDword, EncodeCipDword,
                  (CipAttributeDecodeFromMessage)DecodeCipDword,
                  &g_ascii_config_attr, kSetAndGetAble | kPreGetFunc | kPostSetFunc);
  InsertAttribute(instance, 3, kCipDword, EncodeCipDword, NULL,
                  &g_ascii_status_attr, kGetableSingle | kPreGetFunc);
  InsertAttribute(instance, 4, kCipUint, EncodeCipUint, NULL,
                  &g_ascii_output_buffer_length_attr, kGetableSingle | kPreGetFunc);
  InsertAttribute(instance, 5, kCipUint, EncodeCipUint, NULL,
                  &g_ascii_input_buffer_length_attr, kGetableSingle | kPreGetFunc);
  InsertAttribute(instance, 6, kCipDword, EncodeCipDword,
                  (CipAttributeDecodeFromMessage)DecodeCipDword,
                  &g_ascii_input_start_delim_attr,
                  kSetAndGetAble | kPreGetFunc);
  InsertAttribute(instance, 7, kCipDword, EncodeCipDword,
                  (CipAttributeDecodeFromMessage)DecodeCipDword,
                  &g_ascii_input_end_delim_attr, kSetAndGetAble | kPreGetFunc);
  InsertAttribute(instance, 8, kCipDword, EncodeCipDword,
                  (CipAttributeDecodeFromMessage)DecodeCipDword,
                  &g_ascii_output_start_delim_attr, kSetAndGetAble | kPreGetFunc);
  InsertAttribute(instance, 9, kCipDword, EncodeCipDword,
                  (CipAttributeDecodeFromMessage)DecodeCipDword,
                  &g_ascii_output_end_delim_attr, kSetAndGetAble | kPreGetFunc);
  InsertAttribute(instance, 10, kCipUdint, EncodeCipUdint,
                  (CipAttributeDecodeFromMessage)DecodeCipUdint,
                  &g_ascii_output_char_cnt_attr, kSetAndGetAble | kPreGetFunc);
  InsertAttribute(instance, 11, kCipUdint, EncodeCipUdint,
                  (CipAttributeDecodeFromMessage)DecodeCipUdint,
                  &g_ascii_input_char_cnt_attr, kSetAndGetAble | kPreGetFunc);
  InsertAttribute(instance, 12, kCipUdint, EncodeCipUdint,
                  (CipAttributeDecodeFromMessage)DecodeCipUdint,
                  &g_ascii_input_timeout_ms_attr, kSetAndGetAble | kPreGetFunc);
}

static void InitializeDiscreteInputPointClass(CipClass *class) {
  InitializeCustomClassAttributes(class);
  InsertService(class, kGetAttributeSingle, &GetAttributeSingle, "GetAttributeSingle");
  InsertService(class, kSetAttributeSingle, &SetAttributeSingle, "SetAttributeSingle");
  InsertGetSetCallback(class, ExplicitObjectPreGetCallback, kPreGetFunc);
  InsertGetSetCallback(class, ExplicitObjectPostSetCallback, kPostSetFunc);

  for (unsigned inst = 1U; inst <= 13U; ++inst) {
    CipInstance *instance = GetCipInstance(class, inst);
    OPENER_ASSERT(NULL != instance);
    const unsigned idx = inst - 1U;
    InsertAttribute(instance, 3, kCipBool, EncodeCipBool, NULL,
                    &g_dip_value_mirror[idx], kGetableSingle | kPreGetFunc);
    InsertAttribute(instance, 4, kCipBool, EncodeCipBool, NULL,
                    &g_dip_status_mirror[idx], kGetableSingle | kPreGetFunc);
    InsertAttribute(instance, 5, kCipUint, EncodeCipUint,
                    (CipAttributeDecodeFromMessage)DecodeCipUint,
                    &g_io_state.dip_filter_off_on_us[idx],
                    kSetAndGetAble | kPostSetFunc);
    InsertAttribute(instance, 6, kCipUint, EncodeCipUint,
                    (CipAttributeDecodeFromMessage)DecodeCipUint,
                    &g_io_state.dip_filter_on_off_us[idx],
                    kSetAndGetAble | kPostSetFunc);
  }
}

static void InitializeDiscreteOutputPointClass(CipClass *class) {
  InitializeCustomClassAttributes(class);
  InsertAttribute((CipInstance *)class, 100, kCipBool, EncodeCipBool,
                  (CipAttributeDecodeFromMessage)DecodeCipBool,
                  &g_dop_pwm_frequency_bool,
                  kSetAndGetAble | kPreGetFunc | kPostSetFunc);
  InsertService(class, kGetAttributeSingle, &GetAttributeSingle, "GetAttributeSingle");
  InsertService(class, kSetAttributeSingle, &SetAttributeSingle, "SetAttributeSingle");
  InsertGetSetCallback(class, ExplicitObjectPreGetCallback, kPreGetFunc);
  InsertGetSetCallback(class, ExplicitObjectPostSetCallback, kPostSetFunc);

  for (unsigned inst = 1U; inst <= 6U; ++inst) {
    CipInstance *instance = GetCipInstance(class, inst);
    OPENER_ASSERT(NULL != instance);
    const unsigned idx = inst - 1U;
    InsertAttribute(instance, 3, kCipBool, EncodeCipBool,
                    (CipAttributeDecodeFromMessage)DecodeCipBool,
                    &g_dop_instance_value_mirror[idx],
                    kSetAndGetAble | kPreGetFunc | kPostSetFunc);
    InsertAttribute(instance, 4, kCipBool, EncodeCipBool, NULL,
                    &g_dop_instance_status_mirror[idx],
                    kGetableSingle | kPreGetFunc);
    InsertAttribute(instance, 100, kCipUsint, EncodeCipUsint,
                    (CipAttributeDecodeFromMessage)DecodeCipUsint,
                    &g_io_state.dop_pwm[idx],
                    kSetAndGetAble | kPreGetFunc | kPostSetFunc);
  }
}

static void InitializeAnalogInputPointClass(CipClass *class) {
  InitializeCustomClassAttributes(class);
  InsertService(class, kGetAttributeSingle, &GetAttributeSingle, "GetAttributeSingle");
  InsertService(class, kSetAttributeSingle, &SetAttributeSingle, "SetAttributeSingle");
  InsertGetSetCallback(class, ExplicitObjectPreGetCallback, kPreGetFunc);
  InsertGetSetCallback(class, ExplicitObjectPostSetCallback, kPostSetFunc);

  for (unsigned inst = 1U; inst <= 4U; ++inst) {
    CipInstance *instance = GetCipInstance(class, inst);
    OPENER_ASSERT(NULL != instance);
    const unsigned idx = inst - 1U;
    InsertAttribute(instance, 3, kCipInt, EncodeCipInt, NULL,
                    &g_aip_value_mirror[idx], kGetableSingle | kPreGetFunc);
    InsertAttribute(instance, 4, kCipBool, EncodeCipBool, NULL,
                    &g_aip_status_mirror[idx], kGetableSingle | kPreGetFunc);
    InsertAttribute(instance, 7, kCipUsint, EncodeCipUsint,
                    (CipAttributeDecodeFromMessage)DecodeCipUsint,
                    &g_io_state.ai_range[idx],
                    kSetAndGetAble | kPostSetFunc);
    InsertAttribute(instance, 100, kCipUsint, EncodeCipUsint,
                    (CipAttributeDecodeFromMessage)DecodeCipUsint,
                    &g_io_state.ai_filter_ms[idx],
                    kSetAndGetAble | kPostSetFunc);
  }
}

static void InitializeAnalogOutputPointClass(CipClass *class) {
  InitializeCustomClassAttributes(class);
  InsertService(class, kGetAttributeSingle, &GetAttributeSingle, "GetAttributeSingle");
  InsertService(class, kSetAttributeSingle, &SetAttributeSingle, "SetAttributeSingle");
  InsertGetSetCallback(class, ExplicitObjectPreGetCallback, kPreGetFunc);
  InsertGetSetCallback(class, ExplicitObjectPostSetCallback, kPostSetFunc);

  CipInstance *instance = GetCipInstance(class, 1);
  OPENER_ASSERT(NULL != instance);
  InsertAttribute(instance, 3, kCipInt, EncodeCipInt,
                  (CipAttributeDecodeFromMessage)DecodeCipInt,
                  &g_aop_value_mirror,
                  kSetAndGetAble | kPreGetFunc | kPostSetFunc);
  InsertAttribute(instance, 4, kCipBool, EncodeCipBool, NULL,
                  &g_aop_status_mirror,
                  kGetableSingle | kPreGetFunc);
  InsertAttribute(instance, 7, kCipUsint, EncodeCipUsint,
                  (CipAttributeDecodeFromMessage)DecodeCipUsint,
                  &g_io_state.ao_range,
                  kSetAndGetAble | kPostSetFunc);
}

static void InitializePositionSensorClass(CipClass *class) {
  InitializeCustomClassAttributes(class);
  InsertService(class, kGetAttributeSingle, &GetAttributeSingle, "GetAttributeSingle");
  InsertService(class, kSetAttributeSingle, &SetAttributeSingle, "SetAttributeSingle");
  InsertGetSetCallback(class, ExplicitObjectPreGetCallback, kPreGetFunc);
  InsertGetSetCallback(class, ExplicitObjectPostSetCallback, kPostSetFunc);

  CipInstance *instance = GetCipInstance(class, 1);
  OPENER_ASSERT(NULL != instance);
  InsertAttribute(instance, 10, kCipDint, EncodeCipDint, NULL,
                  &g_ps_position, kGetableSingle | kPreGetFunc);
  InsertAttribute(instance, 12, kCipBool, EncodeCipBool,
                  (CipAttributeDecodeFromMessage)DecodeCipBool,
                  &g_ps_direction_toggle,
                  kSetAndGetAble | kPreGetFunc | kPostSetFunc);
  InsertAttribute(instance, 24, kCipDint, EncodeCipDint, NULL,
                  &g_ps_velocity, kGetableSingle | kPreGetFunc);
  InsertAttribute(instance, 25, kCipUint, EncodeCipUint,
                  (CipAttributeDecodeFromMessage)DecodePositionSensorVelocityFormatUint,
                  &g_ps_velocity_format,
                  kSetAndGetAble | kPreGetFunc);
  InsertAttribute(instance, 26, kCipUdint, EncodeCipUdint,
                  (CipAttributeDecodeFromMessage)DecodeCipUdint,
                  &g_io_state.encoder_velocity_resolution,
                  kSetAndGetAble | kPostSetFunc);
  InsertAttribute(instance, 44, kCipWord, EncodeCipWord, NULL,
                  &g_ps_alarms, kGetableSingle | kPreGetFunc);
  InsertAttribute(instance, 45, kCipWord, EncodeCipWord, NULL,
                  &g_ps_supported_alarms, kGetableSingle | kPreGetFunc);
  InsertAttribute(instance, 46, kCipBool, EncodeCipBool, NULL,
                  &g_ps_alarm_flag, kGetableSingle | kPreGetFunc);
  InsertAttribute(instance, 100, kCipUsint, EncodeCipUsint,
                  (CipAttributeDecodeFromMessage)DecodeCipUsint,
                  &g_ps_reserved_usint,
                  kSetAndGetAble | kPreGetFunc);
  InsertAttribute(instance, 101, kCipDint, EncodeCipDint, NULL,
                  &g_ps_index_location, kGetableSingle | kPreGetFunc);
  InsertAttribute(instance, 102, kCipBool, EncodeCipBool,
                  (CipAttributeDecodeFromMessage)DecodeCipBool,
                  &g_ps_index_active_level,
                  kSetAndGetAble | kPreGetFunc | kPostSetFunc);
  InsertAttribute(instance, 103, kCipDint, EncodeCipDint,
                  (CipAttributeDecodeFromMessage)DecodeCipDint,
                  &g_ps_add_to_position,
                  kSetAndGetAble | kPreGetFunc | kPostSetFunc);
  InsertAttribute(instance, 104, kCipBool, EncodeCipBool, NULL,
                  &g_ps_add_ack, kGetableSingle | kPreGetFunc);
}

static void InitializeMotorConfigurationClass(CipClass *class) {
  InitializeCustomClassAttributes(class);
  InsertService(class, kGetAttributeSingle, &GetAttributeSingle, "GetAttributeSingle");
  InsertService(class, kSetAttributeSingle, &SetAttributeSingle, "SetAttributeSingle");
  InsertGetSetCallback(class, ExplicitObjectPreGetCallback, kPreGetFunc);
  InsertGetSetCallback(class, ExplicitObjectPostSetCallback, kPostSetFunc);

  for (unsigned inst = 1U; inst <= kMotorAxisInstanceCount; ++inst) {
    CipInstance *instance = GetCipInstance(class, inst);
    OPENER_ASSERT(NULL != instance);
    const unsigned idx = inst - 1U;
    MotorAxisExplicitState *const m = &g_motor_axis[idx];

    InsertAttribute(instance, 1, kCipDword, EncodeCipDword,
                    (CipAttributeDecodeFromMessage)DecodeCipDword,
                    &m->cfg_config_register, kSetAndGetAble | kPostSetFunc);
    InsertAttribute(instance, 2, kCipDint, EncodeCipDint,
                    (CipAttributeDecodeFromMessage)DecodeCipDint,
                    &m->cfg_soft_limit_1, kSetAndGetAble | kPostSetFunc);
    InsertAttribute(instance, 3, kCipDint, EncodeCipDint,
                    (CipAttributeDecodeFromMessage)DecodeCipDint,
                    &m->cfg_soft_limit_2, kSetAndGetAble | kPostSetFunc);
    InsertAttribute(instance, 4, kCipSint, EncodeCipSint,
                    (CipAttributeDecodeFromMessage)DecodeCipSint,
                    &m->cfg_positive_limit_connector,
                    kSetAndGetAble | kPostSetFunc);
    InsertAttribute(instance, 5, kCipSint, EncodeCipSint,
                    (CipAttributeDecodeFromMessage)DecodeCipSint,
                    &m->cfg_negative_limit_connector,
                    kSetAndGetAble | kPostSetFunc);
    InsertAttribute(instance, 6, kCipSint, EncodeCipSint,
                    (CipAttributeDecodeFromMessage)DecodeCipSint,
                    &m->cfg_home_sensor_connector, kSetAndGetAble | kPostSetFunc);
    InsertAttribute(instance, 7, kCipSint, EncodeCipSint,
                    (CipAttributeDecodeFromMessage)DecodeCipSint,
                    &m->cfg_brake_output_connector, kSetAndGetAble | kPostSetFunc);
    InsertAttribute(instance, 8, kCipSint, EncodeCipSint,
                    (CipAttributeDecodeFromMessage)DecodeCipSint,
                    &m->cfg_position_capture_connector,
                    kSetAndGetAble | kPostSetFunc);
    InsertAttribute(instance, 9, kCipDint, EncodeCipDint,
                    (CipAttributeDecodeFromMessage)DecodeCipDint,
                    &m->cfg_max_deceleration_rate, kSetAndGetAble | kPostSetFunc);
    InsertAttribute(instance, 10, kCipSint, EncodeCipSint,
                    (CipAttributeDecodeFromMessage)DecodeCipSint,
                    &m->cfg_stop_sensor_connector, kSetAndGetAble | kPostSetFunc);
    InsertAttribute(instance, 11, kCipSint, EncodeCipSint,
                    (CipAttributeDecodeFromMessage)DecodeCipSint,
                    &m->cfg_follow_encoder_axis, kSetAndGetAble | kPostSetFunc);
    InsertAttribute(instance, 12, kCipDint, EncodeCipDint,
                    (CipAttributeDecodeFromMessage)DecodeCipDint,
                    &m->cfg_follow_divisor, kSetAndGetAble | kPostSetFunc);
    InsertAttribute(instance, 13, kCipDint, EncodeCipDint,
                    (CipAttributeDecodeFromMessage)DecodeCipDint,
                    &m->cfg_follow_multiplier, kSetAndGetAble | kPostSetFunc);
  }
}

static void InitializeMotorInputClass(CipClass *class) {
  InitializeCustomClassAttributes(class);
  InsertService(class, kGetAttributeSingle, &GetAttributeSingle, "GetAttributeSingle");
  InsertService(class, kSetAttributeSingle, &SetAttributeSingle, "SetAttributeSingle");
  InsertGetSetCallback(class, ExplicitObjectPreGetCallback, kPreGetFunc);
  InsertGetSetCallback(class, ExplicitObjectPostSetCallback, kPostSetFunc);

  for (unsigned inst = 1U; inst <= kMotorAxisInstanceCount; ++inst) {
    CipInstance *instance = GetCipInstance(class, inst);
    OPENER_ASSERT(NULL != instance);
    const unsigned idx = inst - 1U;
    MotorAxisExplicitState *const m = &g_motor_axis[idx];

    InsertAttribute(instance, 1, kCipDint, EncodeCipDint, NULL,
                    &m->in_commanded_position, kGetableSingle | kPreGetFunc);
    InsertAttribute(instance, 2, kCipDint, EncodeCipDint, NULL,
                    &m->in_commanded_velocity, kGetableSingle | kPreGetFunc);
    InsertAttribute(instance, 3, kCipDint, EncodeCipDint, NULL,
                    &m->in_target_position, kGetableSingle | kPreGetFunc);
    InsertAttribute(instance, 4, kCipDint, EncodeCipDint, NULL,
                    &m->in_target_velocity, kGetableSingle | kPreGetFunc);
    InsertAttribute(instance, 5, kCipDint, EncodeCipDint, NULL,
                    &m->in_captured_position, kGetableSingle | kPreGetFunc);
    InsertAttribute(instance, 6, kCipReal, EncodeCipReal, NULL,
                    &m->in_measured_torque, kGetableSingle | kPreGetFunc);
    InsertAttribute(instance, 7, kCipDword, EncodeCipDword, NULL,
                    &m->in_motor_status, kGetableSingle | kPreGetFunc);
    InsertAttribute(instance, 8, kCipDword, EncodeCipDword, NULL,
                    &m->in_motor_shutdowns, kGetableSingle | kPreGetFunc);
  }
}

static void InitializeMotorOutputClass(CipClass *class) {
  InitializeCustomClassAttributes(class);
  InsertService(class, kGetAttributeSingle, &GetAttributeSingle, "GetAttributeSingle");
  InsertService(class, kSetAttributeSingle, &SetAttributeSingle, "SetAttributeSingle");
  InsertGetSetCallback(class, ExplicitObjectPreGetCallback, kPreGetFunc);
  InsertGetSetCallback(class, ExplicitObjectPostSetCallback, kPostSetFunc);

  for (unsigned inst = 1U; inst <= kMotorAxisInstanceCount; ++inst) {
    CipInstance *instance = GetCipInstance(class, inst);
    OPENER_ASSERT(NULL != instance);
    const unsigned idx = inst - 1U;
    MotorAxisExplicitState *const m = &g_motor_axis[idx];

    InsertAttribute(instance, 1, kCipDint, EncodeCipDint,
                    (CipAttributeDecodeFromMessage)DecodeCipDint,
                    &m->out_move_distance,
                    kSetAndGetAble | kPreGetFunc | kPostSetFunc);
    InsertAttribute(instance, 2, kCipDint, EncodeCipDint,
                    (CipAttributeDecodeFromMessage)DecodeCipDint,
                    &m->out_jog_velocity,
                    kSetAndGetAble | kPreGetFunc | kPostSetFunc);
    InsertAttribute(instance, 3, kCipUdint, EncodeCipUdint,
                    (CipAttributeDecodeFromMessage)DecodeCipUdint,
                    &m->out_velocity_limit,
                    kSetAndGetAble | kPreGetFunc | kPostSetFunc);
    InsertAttribute(instance, 4, kCipUdint, EncodeCipUdint,
                    (CipAttributeDecodeFromMessage)DecodeCipUdint,
                    &m->out_acceleration_limit,
                    kSetAndGetAble | kPreGetFunc | kPostSetFunc);
    InsertAttribute(instance, 5, kCipUdint, EncodeCipUdint,
                    (CipAttributeDecodeFromMessage)DecodeCipUdint,
                    &m->out_deceleration_limit,
                    kSetAndGetAble | kPreGetFunc | kPostSetFunc);
    InsertAttribute(instance, 6, kCipDword, EncodeCipDword,
                    (CipAttributeDecodeFromMessage)DecodeCipDword,
                    &m->out_output_register,
                    kSetAndGetAble | kPreGetFunc | kPostSetFunc);
    InsertAttribute(instance, 7, kCipDint, EncodeCipDint,
                    (CipAttributeDecodeFromMessage)DecodeCipDint,
                    &m->out_add_to_position,
                    kSetAndGetAble | kPreGetFunc | kPostSetFunc);
  }
}

static void InitializeMConnectorClass(CipClass *class) {
  InitializeCustomClassAttributes(class);
  InsertService(class, kGetAttributeSingle, &GetAttributeSingle, "GetAttributeSingle");
  InsertService(class, kSetAttributeSingle, &SetAttributeSingle, "SetAttributeSingle");
  InsertGetSetCallback(class, ExplicitObjectPreGetCallback, kPreGetFunc);
  InsertGetSetCallback(class, ExplicitObjectPostSetCallback, kPostSetFunc);

  for (unsigned inst = 1U; inst <= kMotorAxisInstanceCount; ++inst) {
    CipInstance *instance = GetCipInstance(class, inst);
    OPENER_ASSERT(NULL != instance);
    const unsigned idx = inst - 1U;
    MotorAxisExplicitState *const m = &g_motor_axis[idx];

    InsertAttribute(instance, 1, kCipUsint, EncodeCipUsint,
                    (CipAttributeDecodeFromMessage)DecodeCipUsint,
                    &m->mc_output_register,
                    kSetAndGetAble | kPreGetFunc | kPostSetFunc);
    InsertAttribute(instance, 2, kCipWord, EncodeCipWord, NULL,
                    &m->mc_status_register,
                    kGetableSingle | kPreGetFunc);
    InsertAttribute(instance, 3, kCipSint, EncodeCipSint,
                    (CipAttributeDecodeFromMessage)DecodeCipSint,
                    &m->mc_enable_connector_number,
                    kSetAndGetAble | kPreGetFunc | kPostSetFunc);
    InsertAttribute(instance, 4, kCipSint, EncodeCipSint,
                    (CipAttributeDecodeFromMessage)DecodeCipSint,
                    &m->mc_a_connector_number,
                    kSetAndGetAble | kPreGetFunc | kPostSetFunc);
    InsertAttribute(instance, 5, kCipSint, EncodeCipSint,
                    (CipAttributeDecodeFromMessage)DecodeCipSint,
                    &m->mc_b_connector_number,
                    kSetAndGetAble | kPreGetFunc | kPostSetFunc);
    InsertAttribute(instance, 6, kCipReal, EncodeCipReal, NULL,
                    &m->mc_hlfb_input_duty,
                    kGetableSingle | kPreGetFunc);
    InsertAttribute(instance, 7, kCipUint, EncodeCipUint,
                    (CipAttributeDecodeFromMessage)DecodeCipUint,
                    &m->mc_enable_trigger_pulses,
                    kSetAndGetAble | kPreGetFunc | kPostSetFunc);
    InsertAttribute(instance, 8, kCipUint, EncodeCipUint,
                    (CipAttributeDecodeFromMessage)DecodeCipUint,
                    &m->mc_a_pwm_value,
                    kSetAndGetAble | kPreGetFunc | kPostSetFunc);
    InsertAttribute(instance, 9, kCipUint, EncodeCipUint,
                    (CipAttributeDecodeFromMessage)DecodeCipUint,
                    &m->mc_b_pwm_value,
                    kSetAndGetAble | kPreGetFunc | kPostSetFunc);
    InsertAttribute(instance, 10, kCipUint, EncodeCipUint,
                    (CipAttributeDecodeFromMessage)DecodeCipUint,
                    &m->mc_enable_pulse_time_ms,
                    kSetAndGetAble | kPreGetFunc | kPostSetFunc);
  }
}

static EipStatus InitializeExplicitConfigObjects(void) {
  CipClass *ccio_class = GetCipClass(kCipCcioObjectClassCode);
  if (ccio_class == NULL) {
    ccio_class = CreateCipClass(kCipCcioObjectClassCode,
                                0, 7, 2, 6, 6, 2, 1,
                                "ccio object", 2, &InitializeCcioObjectClass);
  }
  if (ccio_class == NULL) {
    return kEipStatusError;
  }

  CipClass *board_class = GetCipClass(kCipClearLinkBoardObjectClassCode);
  if (board_class == NULL) {
    board_class = CreateCipClass(kCipClearLinkBoardObjectClassCode,
                                 0, 7, 2, 5, 5, 2, 1,
                                 "clearlink board object", 2,
                                 &InitializeClearLinkBoardObjectClass);
  }
  if (board_class == NULL) {
    return kEipStatusError;
  }

  CipClass *ascii_serial_class = GetCipClass(kCipAsciiSerialObjectClassCode);
  if (ascii_serial_class == NULL) {
    ascii_serial_class =
      CreateCipClass(kCipAsciiSerialObjectClassCode,
                     0, 7, 2, 12, 12, 2, 1,
                     "clearlink ascii serial", 2, &InitializeAsciiSerialObjectClass);
  }
  if (ascii_serial_class == NULL) {
    return kEipStatusError;
  }

  CipClass *dip_class = GetCipClass(kCipDiscreteInputPointClassCode);
  if (dip_class == NULL) {
    dip_class =
      CreateCipClass(kCipDiscreteInputPointClassCode,
                     0, 7, 2, 4, 6, 2, 13,
                     "discrete input point", 2, &InitializeDiscreteInputPointClass);
  }
  if (dip_class == NULL) {
    return kEipStatusError;
  }

  CipClass *dop_class = GetCipClass(kCipDiscreteOutputPointClassCode);
  if (dop_class == NULL) {
    dop_class =
      CreateCipClass(kCipDiscreteOutputPointClassCode,
                     1, 100, 2, 3, 100, 2, 6,
                     "discrete output point", 1, &InitializeDiscreteOutputPointClass);
  }
  if (dop_class == NULL) {
    return kEipStatusError;
  }

  CipClass *aip_class = GetCipClass(kCipAnalogInputPointClassCode);
  if (aip_class == NULL) {
    aip_class =
      CreateCipClass(kCipAnalogInputPointClassCode,
                     0, 7, 2, 4, 100, 2, 4,
                     "analog input point", 2, &InitializeAnalogInputPointClass);
  }
  if (aip_class == NULL) {
    return kEipStatusError;
  }

  CipClass *aop_class = GetCipClass(kCipAnalogOutputPointClassCode);
  if (aop_class == NULL) {
    aop_class =
      CreateCipClass(kCipAnalogOutputPointClassCode,
                     0, 7, 2, 3, 7, 2, 1,
                     "analog output point", 2, &InitializeAnalogOutputPointClass);
  }
  if (aop_class == NULL) {
    return kEipStatusError;
  }

  CipClass *position_sensor_class = GetCipClass(kCipPositionSensorClassCode);
  if (position_sensor_class == NULL) {
    position_sensor_class =
      CreateCipClass(kCipPositionSensorClassCode,
                     0, 7, 2, 13, 104, 2, 1,
                     "position sensor", 2, &InitializePositionSensorClass);
  }
  if (position_sensor_class == NULL) {
    return kEipStatusError;
  }

  CipClass *motor_cfg_class = GetCipClass(kCipMotorConfigurationClassCode);
  if (motor_cfg_class == NULL) {
    motor_cfg_class =
      CreateCipClass(kCipMotorConfigurationClassCode,
                     0, 7, 2, 13, 13, 2, kMotorAxisInstanceCount,
                     "motor configuration", 1, &InitializeMotorConfigurationClass);
  }
  if (motor_cfg_class == NULL) {
    return kEipStatusError;
  }

  CipClass *motor_in_class = GetCipClass(kCipMotorInputClassCode);
  if (motor_in_class == NULL) {
    motor_in_class =
      CreateCipClass(kCipMotorInputClassCode,
                     0, 7, 2, 8, 8, 2, kMotorAxisInstanceCount,
                     "motor input", 1, &InitializeMotorInputClass);
  }
  if (motor_in_class == NULL) {
    return kEipStatusError;
  }

  CipClass *motor_out_class = GetCipClass(kCipMotorOutputClassCode);
  if (motor_out_class == NULL) {
    motor_out_class =
      CreateCipClass(kCipMotorOutputClassCode,
                     0, 7, 2, 7, 7, 2, kMotorAxisInstanceCount,
                     "motor output", 2, &InitializeMotorOutputClass);
  }
  if (motor_out_class == NULL) {
    return kEipStatusError;
  }

  CipClass *mconnector_class = GetCipClass(kCipMConnectorClassCode);
  if (mconnector_class == NULL) {
    mconnector_class =
      CreateCipClass(kCipMConnectorClassCode,
                     0, 7, 2, 10, 10, 2, kMotorAxisInstanceCount,
                     "m-connector", 1, &InitializeMConnectorClass);
  }
  return (mconnector_class == NULL) ? kEipStatusError : kEipStatusOk;
}

static EipUint16 ReadLe16(const EipUint8 *buffer, size_t offset) {
  return (EipUint16)((EipUint16)buffer[offset] |
                     ((EipUint16)buffer[offset + 1U] << 8U));
}

static void WriteLe16(EipUint8 *buffer, size_t offset, EipUint16 value) {
  buffer[offset] = (EipUint8)(value & 0xFFU);
  buffer[offset + 1U] = (EipUint8)((value >> 8U) & 0xFFU);
}

static EipUint32 ReadLe32(const EipUint8 *buffer, size_t offset) {
  return (EipUint32)((EipUint32)buffer[offset] |
                     ((EipUint32)buffer[offset + 1U] << 8U) |
                     ((EipUint32)buffer[offset + 2U] << 16U) |
                     ((EipUint32)buffer[offset + 3U] << 24U));
}

static void WriteLe32(EipUint8 *buffer, size_t offset, EipUint32 value) {
  buffer[offset] = (EipUint8)(value & 0xFFU);
  buffer[offset + 1U] = (EipUint8)((value >> 8U) & 0xFFU);
  buffer[offset + 2U] = (EipUint8)((value >> 16U) & 0xFFU);
  buffer[offset + 3U] = (EipUint8)((value >> 24U) & 0xFFU);
}

static int32_t ReadLeDint(const EipUint8 *buffer, size_t offset) {
  const EipUint32 u = ReadLe32(buffer, offset);
  int32_t v;
  memcpy(&v, &u, sizeof(v));
  return v;
}

static void WriteLeDint(EipUint8 *buffer, size_t offset, int32_t value) {
  memcpy(buffer + offset, &value, sizeof(value));
}

static void WriteLeFloat(EipUint8 *buffer, size_t offset, float value) {
  memcpy(buffer + offset, &value, sizeof(value));
}

static EipUint64 ReadLe64(const EipUint8 *buffer, size_t offset) {
  EipUint64 value = 0ULL;
  for (size_t index = 0; index < 8U; ++index) {
    value |= ((EipUint64)buffer[offset + index] << (index * 8U));
  }
  return value;
}

static void WriteLe64(EipUint8 *buffer, size_t offset, EipUint64 value) {
  for (size_t index = 0; index < 8U; ++index) {
    buffer[offset + index] = (EipUint8)((value >> (index * 8U)) & 0xFFU);
  }
}

static EipUint16 ClampAopIntToCommand(CipInt value) {
  if (value <= 0) {
    return 0U;
  }
  if (value >= (CipInt)32767) {
    return 32767U;
  }
  return (EipUint16)value;
}

static EipUint16 ClampAopValue(EipUint16 value) {
  return (value > 32767U) ? 32767U : value;
}

static EipUint16 AopValueToCurrentMicroamps(EipUint16 value, EipUint8 ao_range) {
  EipUint32 scaled = (EipUint32)value;
  if (ao_range == kAoRange4To20mA) {
    return (EipUint16)(4000U + ((scaled * 16000U) / 32767U));
  }
  return (EipUint16)((scaled * 20000U) / 32767U);
}

static void ApplyAiConfiguration(void) {
  if (g_io_state.ai_range[0] == kAiRange0To10V) {
    ConnectorA9_SetAnalogInputMode();
    ConnectorA9_SetAnalogFilterMs(g_io_state.ai_filter_ms[0]);
  } else {
    ConnectorA9_SetDigitalInputMode();
  }

  if (g_io_state.ai_range[1] == kAiRange0To10V) {
    ConnectorA10_SetAnalogInputMode();
    ConnectorA10_SetAnalogFilterMs(g_io_state.ai_filter_ms[1]);
  } else {
    ConnectorA10_SetDigitalInputMode();
  }

  if (g_io_state.ai_range[2] == kAiRange0To10V) {
    ConnectorA11_SetAnalogInputMode();
    ConnectorA11_SetAnalogFilterMs(g_io_state.ai_filter_ms[2]);
  } else {
    ConnectorA11_SetDigitalInputMode();
  }

  if (g_io_state.ai_range[3] == kAiRange0To10V) {
    ConnectorA12_SetAnalogInputMode();
    ConnectorA12_SetAnalogFilterMs(g_io_state.ai_filter_ms[3]);
  } else {
    ConnectorA12_SetDigitalInputMode();
  }
}

static void ApplyDipFilters(void) {
  for (int idx = 0; idx < 13; ++idx) {
    ConnectorDipApplyFilterUs((EipUint8)idx,
                              g_io_state.dip_filter_off_on_us[idx],
                              g_io_state.dip_filter_on_off_us[idx]);
  }
}

static void ApplyCcioConfiguration(void) {
  Ccio_SetEnabled(g_io_state.ccio_enabled ? 1 : 0);
  if (!g_io_state.ccio_enabled) {
    return;
  }
  for (int board = 0; board < 8; ++board) {
    Ccio_SetBoardFilterMs((uint8_t)board, g_io_state.ccio_filter_ms[board]);
  }
}

static void ApplyEncoderConfiguration(void) {
  bool swap_direction = (g_io_state.encoder_configuration & kEncoderCfgBitSwapDirection) != 0U;
  bool invert_index = (g_io_state.encoder_configuration & kEncoderCfgBitIndexInverted) != 0U;
  g_io_state.encoder_enabled =
    (g_io_state.encoder_configuration & kEncoderCfgBitDisable) == 0U;
  Encoder_SetSwapDirection(swap_direction ? 1 : 0);
  Encoder_SetIndexInverted(invert_index ? 1 : 0);
  Encoder_SetEnabled(g_io_state.encoder_enabled ? 1 : 0);
}

static void ApplyDopPwmFrequencySelect(void) {
  /* ClearCore's connector API does not currently expose a runtime PWM carrier
   * selector for IO-0..IO-5. Keep the requested selection latched so behavior
   * can be applied immediately once the lower-level hook is available. */
}

static void ApplyDopOutput(int index, int state, EipUint8 pwm_duty) {
  if (index >= 0 && index < 6 &&
      ClearLinkMotor_IsIoReservedForBrakeOutput((uint8_t)index) != 0) {
    return;
  }
  switch (index) {
    case 0:
      if (g_io_state.ao_range == kAoRange4To20mA || g_io_state.ao_range == kAoRange0To20mA) {
        ConnectorIO0_SetAnalogOutputMode();
        ConnectorIO0_SetOutputCurrentMicroamps(
          AopValueToCurrentMicroamps(g_io_state.aop_value_command, g_io_state.ao_range));
      } else if (pwm_duty != 0U) {
        ConnectorIO0_SetPwmOutputMode();
        ConnectorIO0_SetPwmDuty(pwm_duty);
      } else {
        if (state != 0) {
          ConnectorIO0_SetDigitalOutputMode();
          ConnectorIO0_SetState(1);
        } else {
          ConnectorIO0_SetDigitalInputMode();
        }
      }
      break;
    case 1:
      if (pwm_duty != 0U) {
        ConnectorIO1_SetPwmOutputMode();
        ConnectorIO1_SetPwmDuty(pwm_duty);
      } else {
        if (state != 0) {
          ConnectorIO1_SetDigitalOutputMode();
          ConnectorIO1_SetState(1);
        } else {
          ConnectorIO1_SetDigitalInputMode();
        }
      }
      break;
    case 2:
      if (pwm_duty != 0U) {
        ConnectorIO2_SetPwmOutputMode();
        ConnectorIO2_SetPwmDuty(pwm_duty);
      } else {
        if (state != 0) {
          ConnectorIO2_SetDigitalOutputMode();
          ConnectorIO2_SetState(1);
        } else {
          ConnectorIO2_SetDigitalInputMode();
        }
      }
      break;
    case 3:
      if (pwm_duty != 0U) {
        ConnectorIO3_SetPwmOutputMode();
        ConnectorIO3_SetPwmDuty(pwm_duty);
      } else {
        if (state != 0) {
          ConnectorIO3_SetDigitalOutputMode();
          ConnectorIO3_SetState(1);
        } else {
          ConnectorIO3_SetDigitalInputMode();
        }
      }
      break;
    case 4:
      if (pwm_duty != 0U) {
        ConnectorIO4_SetPwmOutputMode();
        ConnectorIO4_SetPwmDuty(pwm_duty);
      } else {
        if (state != 0) {
          ConnectorIO4_SetDigitalOutputMode();
          ConnectorIO4_SetState(1);
        } else {
          ConnectorIO4_SetDigitalInputMode();
        }
      }
      break;
    case 5:
      if (pwm_duty != 0U) {
        ConnectorIO5_SetPwmOutputMode();
        ConnectorIO5_SetPwmDuty(pwm_duty);
      } else {
        if (state != 0) {
          ConnectorIO5_SetDigitalOutputMode();
          ConnectorIO5_SetState(1);
        } else {
          ConnectorIO5_SetDigitalInputMode();
        }
      }
      break;
    default:
      break;
  }
}

static void ApplyOutputState(void) {
  for (int idx = 0; idx < 6; ++idx) {
    int bit_set = ((g_io_state.dop_value_bits >> idx) & 0x1U) ? 1 : 0;
    ApplyDopOutput(idx, bit_set, g_io_state.dop_pwm[idx]);
  }
  if (g_io_state.ccio_enabled) {
    Ccio_SetOutputBits(g_io_state.ccio_output_bits);
  }
}

static void EncoderApplyAddPositionTransition(const int32_t add_command) {
  if (add_command != g_io_state.last_add_to_position_command) {
    if (add_command != 0 && g_io_state.encoder_enabled) {
      Encoder_AddToPosition(add_command);
      g_io_state.add_to_position_ack = true;
    } else {
      g_io_state.add_to_position_ack = false;
    }
    g_io_state.last_add_to_position_command = add_command;
  }
}

static void MotorAxisApplyMotorOutputAdd(const unsigned axis_idx_zero_based,
                                         const CipDint cmd) {
  MotorAxisExplicitState *const ax = &g_motor_axis[axis_idx_zero_based];

  if (cmd == 0) {
    ax->out_add_position_ack = false;
    ax->in_motor_status &= ~(1UL << 18);
    return;
  }
  if (!ax->out_add_position_ack) {
    ClearLinkMotor_AddToPosition(axis_idx_zero_based, (int32_t)cmd);
    ax->out_add_position_ack = true;
    ax->in_motor_status |= (1UL << 18);
  }
}

/** @brief One CIP SINT wire byte in config assemblies 150/151 (ClearLink object reference). */
static CipSint ReadCfgAssemblySint8(const EipUint8 *const buf, const size_t off) {
  return (CipSint)(int8_t)buf[off];
}

static void WriteCfgAssemblySint8(EipUint8 *const buf, const size_t off, const CipSint value) {
  int32_t v = (int32_t)value;
  if (v > 127) {
    v = 127;
  } else if (v < -128) {
    v = -128;
  }
  buf[off] = (EipUint8)(uint8_t)(int8_t)v;
}

static void SyncStepDirMotorConfigBlockToAssembly(const unsigned axis_idx_zero_based) {
  if (axis_idx_zero_based >= kMotorAxisInstanceCount) {
    return;
  }
  const MotorAxisExplicitState *const m = &g_motor_axis[axis_idx_zero_based];
  const size_t off =
      kIoCfgStepDirMotor0Offset +
      (size_t)axis_idx_zero_based * kIoCfgStepDirMotorBlockSize;
  WriteLe32(g_assembly_data_config_stepdir, off + 0U, (EipUint32)m->cfg_config_register);
  WriteLeDint(g_assembly_data_config_stepdir, off + 4U, (int32_t)m->cfg_follow_divisor);
  WriteLeDint(g_assembly_data_config_stepdir, off + 8U, (int32_t)m->cfg_follow_multiplier);
  WriteLeDint(g_assembly_data_config_stepdir, off + 12U,
              (int32_t)m->cfg_max_deceleration_rate);
  WriteLeDint(g_assembly_data_config_stepdir, off + 16U, (int32_t)m->cfg_soft_limit_1);
  WriteLeDint(g_assembly_data_config_stepdir, off + 20U, (int32_t)m->cfg_soft_limit_2);
  {
    const size_t tail = off + 24U;
    WriteCfgAssemblySint8(g_assembly_data_config_stepdir, tail + 0U,
                          m->cfg_positive_limit_connector);
    WriteCfgAssemblySint8(g_assembly_data_config_stepdir, tail + 1U,
                          m->cfg_negative_limit_connector);
    WriteCfgAssemblySint8(g_assembly_data_config_stepdir, tail + 2U,
                          m->cfg_home_sensor_connector);
    WriteCfgAssemblySint8(g_assembly_data_config_stepdir, tail + 3U,
                          m->cfg_brake_output_connector);
    WriteCfgAssemblySint8(g_assembly_data_config_stepdir, tail + 4U,
                          m->cfg_stop_sensor_connector);
    WriteCfgAssemblySint8(g_assembly_data_config_stepdir, tail + 5U,
                          m->cfg_position_capture_connector);
    WriteCfgAssemblySint8(g_assembly_data_config_stepdir, tail + 6U,
                          m->cfg_follow_encoder_axis);
    g_assembly_data_config_stepdir[tail + 7U] = 0U;
  }
}

static void SyncMcRouting151BlockToAssembly(const unsigned axis_idx_zero_based) {
  if (axis_idx_zero_based >= kMotorAxisInstanceCount) {
    return;
  }
  const MotorAxisExplicitState *const m = &g_motor_axis[axis_idx_zero_based];
  const size_t off =
      kIoCfgMcMotor0Offset +
      (size_t)axis_idx_zero_based * kIoCfgMcMotorBlockSize;
  WriteCfgAssemblySint8(g_assembly_data_config_mconnector, off + 0U,
                        m->mc_enable_connector_number);
  WriteCfgAssemblySint8(g_assembly_data_config_mconnector, off + 1U,
                        m->mc_a_connector_number);
  WriteCfgAssemblySint8(g_assembly_data_config_mconnector, off + 2U,
                        m->mc_b_connector_number);
  {
    CipUint pulse = m->mc_enable_pulse_time_ms;
    if (pulse > 255U) {
      pulse = 255U;
    }
    g_assembly_data_config_mconnector[off + 3U] = (EipUint8)pulse;
  }
}

static void SyncMConnectorOutput113BlockToAssembly(const unsigned axis_idx_zero_based) {
  if (axis_idx_zero_based >= kMotorAxisInstanceCount) {
    return;
  }
  const MotorAxisExplicitState *const m = &g_motor_axis[axis_idx_zero_based];
  const size_t off =
      kIoOutMConnectorMotor0Offset +
      (size_t)axis_idx_zero_based * kIoOutMConnectorMotorBlockSize;
  WriteLe16(g_assembly_data_output_mconnector, off + 0U,
            (EipUint16)m->mc_enable_trigger_pulses);
  WriteLe16(g_assembly_data_output_mconnector, off + 2U,
            (EipUint16)m->mc_a_pwm_value);
  WriteLe16(g_assembly_data_output_mconnector, off + 4U,
            (EipUint16)m->mc_b_pwm_value);
  g_assembly_data_output_mconnector[off + 6U] = (EipUint8)m->mc_output_register;
  g_assembly_data_output_mconnector[off + 7U] =
      (EipUint8)((m->mc_enable_pulse_time_ms > 255U) ? 255U : m->mc_enable_pulse_time_ms);
}

static void MotorAxisApplyStoredMotorConfiguration(const unsigned axis_idx_zero_based,
                                                   const bool sync_stepdir_cfg150_assembly) {
  if (axis_idx_zero_based >= kMotorAxisInstanceCount) {
    return;
  }
  const MotorAxisExplicitState *const ma = &g_motor_axis[axis_idx_zero_based];
  ClearLinkMotor_ApplyMotorConfiguration(
    axis_idx_zero_based,
    (uint32_t)ma->cfg_config_register,
    (int32_t)ma->cfg_soft_limit_1,
    (int32_t)ma->cfg_soft_limit_2,
    (int16_t)ma->cfg_positive_limit_connector,
    (int16_t)ma->cfg_negative_limit_connector,
    (int16_t)ma->cfg_home_sensor_connector,
    (int16_t)ma->cfg_brake_output_connector,
    (int16_t)ma->cfg_stop_sensor_connector,
    (int32_t)ma->cfg_max_deceleration_rate);
  if (sync_stepdir_cfg150_assembly) {
    SyncStepDirMotorConfigBlockToAssembly(axis_idx_zero_based);
  }
}

static void MotorAxisExplicitDefaultsInit(void) {
  memset(g_motor_axis, 0, sizeof(g_motor_axis));
  for (unsigned i = 0; i < kMotorAxisInstanceCount; ++i) {
    g_motor_axis[i].cfg_config_register = (CipDword)kMotorCfgConfigRegisterPdfDefault;
    g_motor_axis[i].cfg_follow_divisor = (CipDint)1;
    g_motor_axis[i].cfg_follow_multiplier = (CipDint)1;
    g_motor_axis[i].cfg_max_deceleration_rate =
      (CipDint)kMotorCfgMaxDecelerationPdfDefault;
    g_motor_axis[i].cfg_positive_limit_connector = (CipSint)-1;
    g_motor_axis[i].cfg_negative_limit_connector = (CipSint)-1;
    g_motor_axis[i].cfg_home_sensor_connector = (CipSint)-1;
    g_motor_axis[i].cfg_brake_output_connector = (CipSint)-1;
    g_motor_axis[i].cfg_position_capture_connector = (CipSint)-1;
    g_motor_axis[i].cfg_stop_sensor_connector = (CipSint)-1;
    g_motor_axis[i].cfg_follow_encoder_axis = (CipSint)-1;
    g_motor_axis[i].mc_enable_connector_number = (CipSint)-1;
    g_motor_axis[i].mc_a_connector_number = (CipSint)-1;
    g_motor_axis[i].mc_b_connector_number = (CipSint)-1;
    g_motor_axis[i].in_measured_torque = -9999.0F;
    g_motor_axis[i].mc_hlfb_input_duty = -9999.0F;
    g_motor_axis[i].mc_enable_pulse_time_ms = 30U;
  }
  for (unsigned ax = 0U; ax < kMotorAxisInstanceCount; ++ax) {
    MotorAxisApplyStoredMotorConfiguration(ax, false);
    ClearLinkMotor_ApplyMConnectorRouting(
      ax,
      (int16_t)g_motor_axis[ax].mc_enable_connector_number,
      (int16_t)g_motor_axis[ax].mc_a_connector_number,
      (int16_t)g_motor_axis[ax].mc_b_connector_number);
  }
}

static void MirrorSingleDipFilterToConfigAssembly(const unsigned dip_idx_zero_based) {
  const size_t entry_offset =
      kIoCfgDipFilterOffset + (size_t)(dip_idx_zero_based * 4U);
  WriteLe16(g_assembly_data_config_stepdir, entry_offset,
            g_io_state.dip_filter_off_on_us[dip_idx_zero_based]);
  WriteLe16(g_assembly_data_config_stepdir, entry_offset + 2U,
            g_io_state.dip_filter_on_off_us[dip_idx_zero_based]);
  memcpy(&g_assembly_data_config_mconnector[entry_offset],
         &g_assembly_data_config_stepdir[entry_offset], 4U);
}

static void MirrorAiConfigurationToAssemblyBuffers(void) {
  for (int i = 0; i < 4; ++i) {
    g_assembly_data_config_stepdir[kIoCfgAiRangeOffset + (size_t)i] =
      g_io_state.ai_range[i];
    g_assembly_data_config_stepdir[kIoCfgAipFilterOffset + (size_t)i] =
      g_io_state.ai_filter_ms[i];
    g_assembly_data_config_mconnector[kIoCfgAiRangeOffset + (size_t)i] =
      g_io_state.ai_range[i];
    g_assembly_data_config_mconnector[kIoCfgAipFilterOffset + (size_t)i] =
      g_io_state.ai_filter_ms[i];
  }
}

static void MirrorAoConfigurationToAssemblyBuffers(void) {
  g_assembly_data_config_stepdir[kIoCfgAoRangeOffset] = g_io_state.ao_range;
  g_assembly_data_config_mconnector[kIoCfgAoRangeOffset] = g_io_state.ao_range;
}

static void MirrorDopPwmFreqBitsToAssemblyBuffers(void) {
  EipUint16 cfg_stepdir =
      ReadLe16(g_assembly_data_config_stepdir, kIoCfgDopPwmFreqOffset);
  EipUint16 cfg_mconnector =
      ReadLe16(g_assembly_data_config_mconnector, kIoCfgDopPwmFreqOffset);
  if (g_io_state.dop_pwm_frequency_select != 0U) {
    cfg_stepdir = (EipUint16)(cfg_stepdir | kIoCfgBitDopPwmFrequencySelect);
    cfg_mconnector =
      (EipUint16)(cfg_mconnector | kIoCfgBitDopPwmFrequencySelect);
  } else {
    cfg_stepdir =
      (EipUint16)(cfg_stepdir & (EipUint16)(~kIoCfgBitDopPwmFrequencySelect));
    cfg_mconnector =
      (EipUint16)(cfg_mconnector & (EipUint16)(~kIoCfgBitDopPwmFrequencySelect));
  }
  WriteLe16(g_assembly_data_config_stepdir, kIoCfgDopPwmFreqOffset,
            cfg_stepdir);
  WriteLe16(g_assembly_data_config_mconnector, kIoCfgDopPwmFreqOffset,
            cfg_mconnector);
}

static void MirrorEncoderVelocityResolutionToAssemblyBuffers(void) {
  WriteLe32(g_assembly_data_config_stepdir,
            kIoCfgEncoderVelocityResolutionOffset,
            g_io_state.encoder_velocity_resolution);
  WriteLe32(g_assembly_data_config_mconnector,
            kIoCfgEncoderVelocityResolutionOffset,
            g_io_state.encoder_velocity_resolution);
}

static void MirrorEncoderConfigurationByteToAssemblyBuffers(void) {
  g_assembly_data_config_stepdir[kIoCfgEncoderConfigurationOffset] =
      g_io_state.encoder_configuration;
  g_assembly_data_config_mconnector[kIoCfgEncoderConfigurationOffset] =
      g_io_state.encoder_configuration;
}

static void SyncIoOutputBuffersFromIoState(void) {
  WriteLe16(g_assembly_data_output_stepdir, kIoOutAopValueOffset,
            g_io_state.aop_value_command);
  WriteLe16(g_assembly_data_output_mconnector, kIoOutAopValueOffset,
            g_io_state.aop_value_command);
  WriteLe16(g_assembly_data_output_stepdir, kIoOutDopValueOffset,
            g_io_state.dop_value_bits);
  WriteLe16(g_assembly_data_output_mconnector, kIoOutDopValueOffset,
            g_io_state.dop_value_bits);
  for (int i = 0; i < 6; ++i) {
    g_assembly_data_output_stepdir[kIoOutDopPwmOffset + (size_t)i] =
      g_io_state.dop_pwm[i];
    g_assembly_data_output_mconnector[kIoOutDopPwmOffset + (size_t)i] =
      g_io_state.dop_pwm[i];
  }
  WriteLe64(g_assembly_data_output_stepdir, kIoOutCcioValueOffset,
            g_io_state.ccio_output_bits);
  WriteLe64(g_assembly_data_output_mconnector, kIoOutCcioValueOffset,
            g_io_state.ccio_output_bits);
}

/**
 * @brief Pull assembly 150 per-motor class 0x64 blocks into explicit state and hardware.
 *
 * Layout: ClearLink object reference, Configuration Assembly 150 — 32 bytes per motor
 * from offset 80 (DWORD config, follow div/mult, max decel, soft limits, seven SINT
 * connector indices, one reserved byte).
 */
static void LoadStepDirMotorConfigBlocksFromBuffer(const EipUint8 *cfg) {
  for (unsigned ax = 0U; ax < kMotorAxisInstanceCount; ++ax) {
    const size_t off =
        kIoCfgStepDirMotor0Offset + (size_t)ax * kIoCfgStepDirMotorBlockSize;
    MotorAxisExplicitState *const m = &g_motor_axis[ax];
    m->cfg_config_register = (CipDword)ReadLe32(cfg, off + 0U);
    m->cfg_follow_divisor = (CipDint)ReadLeDint(cfg, off + 4U);
    m->cfg_follow_multiplier = (CipDint)ReadLeDint(cfg, off + 8U);
    m->cfg_max_deceleration_rate = (CipDint)ReadLeDint(cfg, off + 12U);
    m->cfg_soft_limit_1 = (CipDint)ReadLeDint(cfg, off + 16U);
    m->cfg_soft_limit_2 = (CipDint)ReadLeDint(cfg, off + 20U);
    {
      const size_t tail = off + 24U;
      m->cfg_positive_limit_connector = ReadCfgAssemblySint8(cfg, tail + 0U);
      m->cfg_negative_limit_connector = ReadCfgAssemblySint8(cfg, tail + 1U);
      m->cfg_home_sensor_connector = ReadCfgAssemblySint8(cfg, tail + 2U);
      m->cfg_brake_output_connector = ReadCfgAssemblySint8(cfg, tail + 3U);
      m->cfg_stop_sensor_connector = ReadCfgAssemblySint8(cfg, tail + 4U);
      m->cfg_position_capture_connector = ReadCfgAssemblySint8(cfg, tail + 5U);
      m->cfg_follow_encoder_axis = ReadCfgAssemblySint8(cfg, tail + 6U);
    }
    MotorAxisApplyStoredMotorConfiguration(ax, false);
  }
}

/**
 * @brief Assembly 151: four bytes per motor — Enable, A, B (SINT), Trigger Pulse Time (USINT).
 */
static void LoadMcRoutingFromConfig151(const EipUint8 *cfg) {
  for (unsigned ax = 0U; ax < kMotorAxisInstanceCount; ++ax) {
    const size_t off =
        kIoCfgMcMotor0Offset + (size_t)ax * kIoCfgMcMotorBlockSize;
    MotorAxisExplicitState *const m = &g_motor_axis[ax];
    m->mc_enable_connector_number = ReadCfgAssemblySint8(cfg, off + 0U);
    m->mc_a_connector_number = ReadCfgAssemblySint8(cfg, off + 1U);
    m->mc_b_connector_number = ReadCfgAssemblySint8(cfg, off + 2U);
    m->mc_enable_pulse_time_ms = (CipUint)cfg[off + 3U];
    ClearLinkMotor_ApplyMConnectorRouting(
      ax,
      (int16_t)m->mc_enable_connector_number,
      (int16_t)m->mc_a_connector_number,
      (int16_t)m->mc_b_connector_number);
    ClearLinkMotor_ApplyMConnectorOutputs(
      ax,
      (uint16_t)m->mc_enable_trigger_pulses,
      (uint16_t)m->mc_a_pwm_value,
      (uint16_t)m->mc_b_pwm_value,
      (uint8_t)m->mc_output_register,
      (uint16_t)m->mc_enable_pulse_time_ms);
  }
}

static void LoadIoConfigurationFromBuffer(const EipUint8 *cfg, const size_t cfg_byte_count) {
  EipUint16 config_bitfield = ReadLe16(cfg, kIoCfgDopPwmFreqOffset);
  for (int i = 0; i < 4; ++i) {
    g_io_state.ai_range[i] = cfg[kIoCfgAiRangeOffset + (size_t)i];
    g_io_state.ai_filter_ms[i] = cfg[kIoCfgAipFilterOffset + (size_t)i];
  }
  g_io_state.ao_range = cfg[kIoCfgAoRangeOffset];
  g_io_state.dop_pwm_frequency_select =
    (config_bitfield & kIoCfgBitDopPwmFrequencySelect) != 0U ? 1U : 0U;
  g_io_state.ccio_enabled = (config_bitfield & kIoCfgBitCcioEnable) != 0U;
  g_ccio_enable_attr = g_io_state.ccio_enabled ? 1U : 0U;
  for (int dip = 0; dip < 13; ++dip) {
    size_t entry_offset = kIoCfgDipFilterOffset + (size_t)(dip * 4);
    g_io_state.dip_filter_off_on_us[dip] = ReadLe16(cfg, entry_offset);
    g_io_state.dip_filter_on_off_us[dip] = ReadLe16(cfg, entry_offset + 2U);
  }
  for (int board = 0; board < 8; ++board) {
    g_io_state.ccio_filter_ms[board] = cfg[kIoCfgCcioFilterOffset + (size_t)board];
  }
  g_io_state.encoder_velocity_resolution =
    ReadLe32(cfg, kIoCfgEncoderVelocityResolutionOffset);
  g_io_state.encoder_configuration = cfg[kIoCfgEncoderConfigurationOffset];
  ApplyDopPwmFrequencySelect();
  ApplyAiConfiguration();
  ApplyDipFilters();
  ApplyCcioConfiguration();
  ApplyEncoderConfiguration();
  ApplyOutputState();
  if (cfg_byte_count == kAssemblyConfigMConnectorSize) {
    LoadMcRoutingFromConfig151(cfg);
  }
  if (cfg_byte_count >= kAssemblyConfigStepDirSize) {
    LoadStepDirMotorConfigBlocksFromBuffer(cfg);
    /* Re-apply implicit O2T step/dir slice so limits / polarity / decel track config-only updates. */
    LoadStepDirMotorOutputsFromImplicitAssembly(g_assembly_data_output_stepdir);
  }
}

/** @brief Pack class 65h motor input mirror blocks into assembly 100 (docs/ASSEMBLY_LAYOUT.md). */
static void PackStepDirMotorInputBlocks(EipUint8 *assembly_base) {
  for (unsigned ax = 0U; ax < kMotorAxisInstanceCount; ++ax) {
    RefreshMotorInputInstance((CipInstanceNum)(ax + 1U));
    MotorAxisExplicitState *const m = &g_motor_axis[ax];
    const size_t off =
        kIoInStepDirMotor0Offset + (size_t)ax * kIoInStepDirMotorBlockSize;
    WriteLeDint(assembly_base, off + 0U, (int32_t)m->in_commanded_position);
    WriteLeDint(assembly_base, off + 4U, (int32_t)m->in_commanded_velocity);
    WriteLeDint(assembly_base, off + 8U, (int32_t)m->in_target_position);
    WriteLeDint(assembly_base, off + 12U, (int32_t)m->in_target_velocity);
    WriteLeDint(assembly_base, off + 16U, (int32_t)m->in_captured_position);
    WriteLeFloat(assembly_base, off + 20U, m->in_measured_torque);
    WriteLe32(assembly_base, off + 24U, (EipUint32)m->in_motor_status);
    WriteLe32(assembly_base, off + 28U, (EipUint32)m->in_motor_shutdowns);
  }
}

/** @brief Pack class 67h HLFB/status fields into assembly 101 shared motor-connector slice. */
static void PackMConnectorMotorInputFields(EipUint8 *assembly_base) {
  for (unsigned ax = 0U; ax < kMotorAxisInstanceCount; ++ax) {
    RefreshMConnectorInstance((CipInstanceNum)(ax + 1U));
    MotorAxisExplicitState *const m = &g_motor_axis[ax];
    WriteLeFloat(assembly_base,
                 kIoInMConnectorHlfbRealBaseOffset + (size_t)ax * 4U,
                 m->mc_hlfb_input_duty);
    WriteLe16(assembly_base,
              kIoInMConnectorStatusWordBaseOffset + (size_t)ax * 2U,
              (EipUint16)m->mc_status_register);
  }
}

/**
 * @brief Populate implicit ASCII input segment on T2O assemblies (COM-0 RX snapshot).
 *
 * Drains up to 128 bytes from the UART RX FIFO into the EtherNet/IP buffer each cyclic send.
 */
static void PopulateAsciiImplicitInputSegment(EipUint8 *assembly_base,
                                              size_t ascii_segment_offset) {
  uint8_t *const data_ptr =
      assembly_base + ascii_segment_offset + kAsciiImplicitInDataRelOffset;

  AsciiCom0_RefreshMirrors((uint32_t *)(void *)&g_ascii_baud_attr,
                           (uint32_t *)(void *)&g_ascii_status_attr);

  uint32_t bytes_read = 0U;
  AsciiCom0_ReadRxIntoBuffer(data_ptr, kAsciiImplicitInDataBytes, &bytes_read);

  g_ascii_input_char_cnt_attr = (CipUdint)bytes_read;
  if (bytes_read > 0U) {
    ++g_ascii_implicit_input_sequence;
  }

  WriteLe32(assembly_base,
            ascii_segment_offset + kAsciiImplicitInStatusRelOffset,
            (EipUint32)g_ascii_status_attr);
  WriteLe32(assembly_base,
            ascii_segment_offset + kAsciiImplicitInOutCharCntRelOffset,
            (EipUint32)g_ascii_output_char_cnt_attr);
  WriteLe32(assembly_base,
            ascii_segment_offset + kAsciiImplicitInInCharCntRelOffset,
            bytes_read);
  WriteLe32(assembly_base,
            ascii_segment_offset + kAsciiImplicitInOutSeqAckRelOffset,
            g_ascii_plc_ack_input_sequence);
  WriteLe32(assembly_base,
            ascii_segment_offset + kAsciiImplicitInInSizeRelOffset,
            (EipUint32)kAsciiImplicitInDataBytes);
  WriteLe32(assembly_base,
            ascii_segment_offset + kAsciiImplicitInInSequenceRelOffset,
            g_ascii_implicit_input_sequence);
}

/** @brief Apply implicit ASCII output segment from O2T assemblies (COM-0 TX / config). */
static void ApplyAsciiImplicitOutputSegment(const EipUint8 *output,
                                            size_t ascii_base_offset) {
  AsciiCom0_RefreshMirrors((uint32_t *)(void *)&g_ascii_baud_attr,
                           (uint32_t *)(void *)&g_ascii_status_attr);

  const uint32_t cfg =
      ReadLe32(output, ascii_base_offset + kAsciiImplicitOutConfigRelOffset);
  const uint32_t seq_ack_from_plc =
      ReadLe32(output, ascii_base_offset + kAsciiImplicitOutInSeqAckRelOffset);
  uint32_t out_sz =
      ReadLe32(output, ascii_base_offset + kAsciiImplicitOutOutSizeRelOffset);
  (void)ReadLe32(output,
                 ascii_base_offset + kAsciiImplicitOutOutSequenceRelOffset);

  g_ascii_plc_ack_input_sequence = seq_ack_from_plc;

  if (cfg != g_ascii_implicit_last_applied_config) {
    AsciiCom0_ApplyCommitted(cfg, (uint32_t)g_ascii_baud_attr);
    g_ascii_implicit_last_applied_config = cfg;
  }

  if (out_sz > kAsciiImplicitOutDataBytes) {
    out_sz = kAsciiImplicitOutDataBytes;
  }
  if (out_sz > 0U) {
    AsciiCom0_SendTxBytes(
      output + ascii_base_offset + kAsciiImplicitOutDataRelOffset,
      (size_t)out_sz);
    g_ascii_output_char_cnt_attr = (CipUdint)out_sz;
  }
}

/** @brief Parse class 66h implicit motor command blocks from assembly 112. */
static void LoadStepDirMotorOutputsFromImplicitAssembly(const EipUint8 *output) {
  for (unsigned ax = 0U; ax < kMotorAxisInstanceCount; ++ax) {
    const size_t off =
        kIoOutStepDirMotor0Offset + (size_t)ax * kIoOutStepDirMotorBlockSize;
    MotorAxisExplicitState *const axis = &g_motor_axis[ax];

    axis->out_move_distance = ReadLeDint(output, off + 0U);
    axis->out_velocity_limit = ReadLe32(output, off + 4U);
    axis->out_acceleration_limit = ReadLe32(output, off + 8U);
    axis->out_deceleration_limit = ReadLe32(output, off + 12U);
    axis->out_jog_velocity = ReadLeDint(output, off + 16U);
    const int32_t add_cmd = ReadLeDint(output, off + 20U);
    const uint32_t raw_output_register = ReadLe32(output, off + 24U);
    axis->out_output_register = raw_output_register;

    if ((raw_output_register & (1UL << 6)) != 0UL) {
      axis->in_motor_shutdowns = 0;
      axis->out_output_register &= ~(1UL << 6);
    }
    MotorAxisApplyMotorOutputAdd(ax, (CipDint)add_cmd);
    ClearLinkMotor_ApplyStepDirOutputs(ax,
                                       axis->out_move_distance,
                                       axis->out_jog_velocity,
                                       axis->out_velocity_limit,
                                       axis->out_acceleration_limit,
                                       axis->out_deceleration_limit,
                                       raw_output_register);
    RefreshMotorInputInstance((CipInstanceNum)(ax + 1U));
  }
}

/** @brief Parse class 67h implicit connector command blocks from assembly 113. */
static void LoadMConnectorMotorOutputsFromImplicitAssembly(const EipUint8 *output) {
  for (unsigned ax = 0U; ax < kMotorAxisInstanceCount; ++ax) {
    const size_t off =
        kIoOutMConnectorMotor0Offset + (size_t)ax * kIoOutMConnectorMotorBlockSize;
    MotorAxisExplicitState *const axis = &g_motor_axis[ax];
    axis->mc_enable_trigger_pulses = ReadLe16(output, off + 0U);
    axis->mc_a_pwm_value = ReadLe16(output, off + 2U);
    axis->mc_b_pwm_value = ReadLe16(output, off + 4U);
    axis->mc_output_register = output[off + 6U];

    if (axis->mc_enable_trigger_pulses != 0U) {
      axis->mc_status_register =
        (CipWord)(axis->mc_status_register | (CipWord)(1U << 3));
    } else {
      axis->mc_status_register =
        (CipWord)(axis->mc_status_register & (CipWord)(~(1U << 3)));
    }

    ClearLinkMotor_ApplyMConnectorOutputs(ax,
                                          (uint16_t)axis->mc_enable_trigger_pulses,
                                          (uint16_t)axis->mc_a_pwm_value,
                                          (uint16_t)axis->mc_b_pwm_value,
                                          axis->mc_output_register,
                                          (uint16_t)axis->mc_enable_pulse_time_ms);
  }
}

static void LoadIoOutputFromBuffer(const EipUint8 *output,
                                   ClearLinkImplicitAssemblyFamily family) {
  const int want_step_and_dir =
      (family == kClearLinkImplicitAssemblyStepDir) ? 1 : 0;
  if (want_step_and_dir != s_applied_clearcore_motor_family) {
    if (BoardMotorMode_Request(want_step_and_dir)) {
      s_applied_clearcore_motor_family = want_step_and_dir;
    }
  }

  const int32_t add_to_position_command =
      ReadLeDint(output, kIoOutEncoderAddPositionOffset);
  g_io_state.aop_value_command =
      ClampAopValue(ReadLe16(output, kIoOutAopValueOffset));
  g_io_state.dop_value_bits = ReadLe16(output, kIoOutDopValueOffset);
  g_io_state.ccio_output_bits = ReadLe64(output, kIoOutCcioValueOffset);
  for (int i = 0; i < 6; ++i) {
    g_io_state.dop_pwm[i] = output[kIoOutDopPwmOffset + (size_t)i];
  }
  EncoderApplyAddPositionTransition(add_to_position_command);
  ApplyOutputState();

  if (family == kClearLinkImplicitAssemblyStepDir) {
    LoadStepDirMotorOutputsFromImplicitAssembly(output);
    ApplyAsciiImplicitOutputSegment(output, kIoOutStepDirAsciiSegmentOffset);
  } else {
    LoadMConnectorMotorOutputsFromImplicitAssembly(output);
    ApplyAsciiImplicitOutputSegment(output, kIoOutMConnectorAsciiSegmentOffset);
  }
}

static void SetBit(EipUint8 *buffer, size_t byte_offset, EipUint8 bit_index, bool value) {
  EipUint16 word = ReadLe16(buffer, byte_offset);
  EipUint16 mask = (EipUint16)(1U << bit_index);
  if (value) {
    word = (EipUint16)(word | mask);
  } else {
    word = (EipUint16)(word & (EipUint16)(~mask));
  }
  WriteLe16(buffer, byte_offset, word);
}

static void SetByteBit(EipUint8 *buffer, size_t byte_offset, EipUint8 bit_index, bool value) {
  EipUint8 mask = (EipUint8)(1U << bit_index);
  if (value) {
    buffer[byte_offset] = (EipUint8)(buffer[byte_offset] | mask);
  } else {
    buffer[byte_offset] = (EipUint8)(buffer[byte_offset] & (EipUint8)(~mask));
  }
}

static bool IoOutputActiveForDipStatus(int index) {
  if (index == 0) {
    if (g_io_state.ao_range != kAoRangeDisabled) {
      return true;
    }
  }
  if (g_io_state.dop_pwm[index] != 0U) {
    return true;
  }
  return ((g_io_state.dop_value_bits >> index) & 0x1U) != 0U;
}

static bool IoOutputInFault(int index) {
  switch (index) {
    case 0: return ConnectorIO0_IsInFault() != 0;
    case 1: return ConnectorIO1_IsInFault() != 0;
    case 2: return ConnectorIO2_IsInFault() != 0;
    case 3: return ConnectorIO3_IsInFault() != 0;
    case 4: return ConnectorIO4_IsInFault() != 0;
    case 5: return ConnectorIO5_IsInFault() != 0;
    default: return false;
  }
}

static void DiscreteInputRefreshInstanceAttributes(const CipInstanceNum instance_number) {
  if (instance_number < 1U || instance_number > 13U) {
    return;
  }
  const unsigned idx = (unsigned)(instance_number - 1U);
  bool value = false;
  bool status = false;

  if (idx < 6U) {
    const int io_idx = (int)idx;
    status = IoOutputActiveForDipStatus(io_idx);
    const bool pwm_active = (g_io_state.dop_pwm[io_idx] != 0U);
    const bool io0_analog_mode =
      (io_idx == 0) &&
      (g_io_state.ao_range == kAoRange4To20mA ||
       g_io_state.ao_range == kAoRange0To20mA);
    if (!pwm_active && !io0_analog_mode) {
      switch (io_idx) {
        case 0:
          value = (ConnectorIO0_GetInputState() != 0);
          break;
        case 1:
          value = (ConnectorIO1_GetInputState() != 0);
          break;
        case 2:
          value = (ConnectorIO2_GetInputState() != 0);
          break;
        case 3:
          value = (ConnectorIO3_GetInputState() != 0);
          break;
        case 4:
          value = (ConnectorIO4_GetInputState() != 0);
          break;
        case 5:
          value = (ConnectorIO5_GetInputState() != 0);
          break;
        default:
          break;
      }
    }
  } else if (idx <= 8U) {
    switch (idx) {
      case 6U:
        value = (ConnectorDI6_GetState() != 0);
        break;
      case 7U:
        value = (ConnectorDI7_GetState() != 0);
        break;
      case 8U:
        value = (ConnectorDI8_GetState() != 0);
        break;
      default:
        break;
    }
    status = false;
  } else {
    const unsigned ai_idx = idx - 9U;
    if (g_io_state.ai_range[ai_idx] == kAiRange0To10V) {
      status = true;
      value = false;
    } else {
      switch (ai_idx) {
        case 0U:
          value = (ConnectorA9_GetState() != 0);
          break;
        case 1U:
          value = (ConnectorA10_GetState() != 0);
          break;
        case 2U:
          value = (ConnectorA11_GetState() != 0);
          break;
        case 3U:
          value = (ConnectorA12_GetState() != 0);
          break;
        default:
          break;
      }
    }
  }

  g_dip_value_mirror[idx] = value ? (CipBool)1 : (CipBool)0;
  g_dip_status_mirror[idx] = status ? (CipBool)1 : (CipBool)0;
}

static void AnalogInputRefreshInstanceAttributes(const CipInstanceNum instance_number) {
  if (instance_number < 1U || instance_number > 4U) {
    return;
  }
  const unsigned idx = (unsigned)(instance_number - 1U);
  const bool analog_ok = (g_io_state.ai_range[idx] == kAiRange0To10V);
  g_aip_status_mirror[idx] = analog_ok ? (CipBool)0 : (CipBool)1;
  EipUint16 raw = 0U;
  switch (idx) {
    case 0U:
      raw = ConnectorA9_GetAnalogRaw();
      break;
    case 1U:
      raw = ConnectorA10_GetAnalogRaw();
      break;
    case 2U:
      raw = ConnectorA11_GetAnalogRaw();
      break;
    case 3U:
      raw = ConnectorA12_GetAnalogRaw();
      break;
    default:
      break;
  }
  g_aip_value_mirror[idx] =
    analog_ok ? (CipInt)(int16_t)(raw & 0xFFFFU) : (CipInt)0;
}

static void DiscreteOutputRefreshInstanceAttributes(const CipInstanceNum instance_number) {
  if (instance_number < 1U || instance_number > 6U) {
    return;
  }
  const unsigned idx = (unsigned)(instance_number - 1U);
  g_dop_instance_value_mirror[idx] =
    (((g_io_state.dop_value_bits >> idx) & 1U) != 0U) ? (CipBool)1 : (CipBool)0;
  if (idx == 0U) {
    g_dop_instance_status_mirror[idx] =
      ((g_io_state.ao_range != kAoRangeDisabled) || IoOutputInFault(0))
      ? (CipBool)1
      : (CipBool)0;
  } else {
    g_dop_instance_status_mirror[idx] =
      IoOutputInFault((int)idx) ? (CipBool)1 : (CipBool)0;
  }
}

static int32_t EncoderVelocityScaledForEip(void) {
  const int32_t raw = Encoder_GetVelocity();
  uint32_t res = g_io_state.encoder_velocity_resolution;
  if (res == 0U) {
    res = 100U;
  }
  /*
   * Position Sensor attr 26 / config assembly: "Velocity Resolution" minimizes velocity
   * dither (default 100). Same physical rate at 100 maps raw counts/s 1:1; larger values
   * scale reported velocity down (ClearLink object reference Rev. 1.15, Position Sensor).
   */
  return (int32_t)(((int64_t)raw * 100LL) / (int64_t)res);
}

static void RefreshPositionSensorMirrors(void) {
  g_ps_position = Encoder_GetPosition();
  g_ps_velocity = EncoderVelocityScaledForEip();
  g_ps_velocity_format = 0x1F04U;
  g_ps_direction_toggle =
    ((g_io_state.encoder_configuration & kEncoderCfgBitSwapDirection) != 0U)
    ? (CipBool)1
    : (CipBool)0;
  g_ps_index_active_level =
    ((g_io_state.encoder_configuration & kEncoderCfgBitIndexInverted) != 0U)
    ? (CipBool)1
    : (CipBool)0;
  g_ps_alarm_flag = Encoder_GetAlarmFlag() != 0 ? (CipBool)1 : (CipBool)0;
  g_ps_alarms = (g_ps_alarm_flag != (CipBool)0) ? (CipWord)1 : (CipWord)0;
  g_ps_supported_alarms = (CipWord)1;
  g_ps_index_location = Encoder_GetIndexPosition();
  g_ps_add_ack = g_io_state.add_to_position_ack ? (CipBool)1 : (CipBool)0;
  g_ps_add_to_position =
    (CipDint)ReadLe32(g_assembly_data_output_stepdir,
                      kIoOutEncoderAddPositionOffset);
}

static void RefreshMotorInputInstance(const CipInstanceNum instance_number) {
  if (instance_number < 1U || instance_number > kMotorAxisInstanceCount) {
    return;
  }
  const unsigned idx = (unsigned)(instance_number - 1U);
  MotorAxisExplicitState *const axis = &g_motor_axis[idx];

  uint32_t extra_status = 0U;
  if (axis->out_add_position_ack) {
    extra_status |= (1UL << 18);
  }
  float torque = -9999.0F;
  uint32_t motor_status = 0U;
  uint32_t motor_shutdowns = 0U;
  ClearLinkMotor_PollStepDirFeedback(idx,
                                     extra_status,
                                     &axis->in_commanded_position,
                                     &axis->in_commanded_velocity,
                                     &axis->in_target_position,
                                     &axis->in_target_velocity,
                                     &torque,
                                     &motor_status,
                                     &motor_shutdowns);
  axis->in_measured_torque = (CipReal)torque;
  axis->in_motor_status = (CipDword)motor_status;
  axis->in_motor_shutdowns = (CipDword)motor_shutdowns;
}

static void RefreshMConnectorInstance(const CipInstanceNum instance_number) {
  if (instance_number < 1U || instance_number > kMotorAxisInstanceCount) {
    return;
  }
  const unsigned idx = (unsigned)(instance_number - 1U);
  MotorAxisExplicitState *const axis = &g_motor_axis[idx];

  float hlfb = -9999.0F;
  uint16_t mc_st = 0U;
  ClearLinkMotor_PollMConnectorFeedback(idx, &hlfb, &mc_st);
  if (axis->mc_enable_trigger_pulses != 0U) {
    mc_st = (uint16_t)(mc_st | (1U << 2));  /* Trigger Pulses Active */
    mc_st = (uint16_t)(mc_st | (1U << 3));  /* Trigger Pulses ACK */
  } else {
    mc_st = (uint16_t)(mc_st & (uint16_t)(~(1U << 2)));
    mc_st = (uint16_t)(mc_st & (uint16_t)(~(1U << 3)));
  }
  if ((axis->mc_output_register & (1U << 3)) != 0U) {
    mc_st = (uint16_t)(mc_st | (1U << 4));  /* Disable Pulse ACK */
  } else {
    mc_st = (uint16_t)(mc_st & (uint16_t)(~(1U << 4)));
  }
  axis->mc_hlfb_input_duty = (CipReal)hlfb;
  axis->mc_status_register = (CipWord)mc_st;
}

static void PopulateSharedIoInputSegment(EipUint8 *input_buffer) {
  memset(input_buffer, 0, kIoInSharedSize);

  /* DIP value/state bits for IO-0..IO-5, DI-6..DI-8, A-9..A-12 */
  for (int idx = 0; idx < 6; ++idx) {
    bool status = IoOutputActiveForDipStatus(idx);
    bool value = false;
    bool pwm_active = (g_io_state.dop_pwm[idx] != 0U);
    bool io0_analog_mode = (idx == 0) &&
      (g_io_state.ao_range == kAoRange4To20mA || g_io_state.ao_range == kAoRange0To20mA);

    if (!pwm_active && !io0_analog_mode) {
      switch (idx) {
        case 0: value = (ConnectorIO0_GetInputState() != 0); break;
        case 1: value = (ConnectorIO1_GetInputState() != 0); break;
        case 2: value = (ConnectorIO2_GetInputState() != 0); break;
        case 3: value = (ConnectorIO3_GetInputState() != 0); break;
        case 4: value = (ConnectorIO4_GetInputState() != 0); break;
        case 5: value = (ConnectorIO5_GetInputState() != 0); break;
        default: break;
      }
    }

    SetBit(input_buffer, kIoInDipValueOffset, (EipUint8)idx, value);
    SetBit(input_buffer, kIoInDipStatusOffset, (EipUint8)idx, status);
  }

  SetBit(input_buffer, kIoInDipValueOffset, 6U, ConnectorDI6_GetState() != 0);
  SetBit(input_buffer, kIoInDipValueOffset, 7U, ConnectorDI7_GetState() != 0);
  SetBit(input_buffer, kIoInDipValueOffset, 8U, ConnectorDI8_GetState() != 0);

  if (g_io_state.ai_range[0] == kAiRange0To10V) {
    SetBit(input_buffer, kIoInDipStatusOffset, 9U, true);
  } else {
    SetBit(input_buffer, kIoInDipValueOffset, 9U, ConnectorA9_GetState() != 0);
  }
  if (g_io_state.ai_range[1] == kAiRange0To10V) {
    SetBit(input_buffer, kIoInDipStatusOffset, 10U, true);
  } else {
    SetBit(input_buffer, kIoInDipValueOffset, 10U, ConnectorA10_GetState() != 0);
  }
  if (g_io_state.ai_range[2] == kAiRange0To10V) {
    SetBit(input_buffer, kIoInDipStatusOffset, 11U, true);
  } else {
    SetBit(input_buffer, kIoInDipValueOffset, 11U, ConnectorA11_GetState() != 0);
  }
  if (g_io_state.ai_range[3] == kAiRange0To10V) {
    SetBit(input_buffer, kIoInDipStatusOffset, 12U, true);
  } else {
    SetBit(input_buffer, kIoInDipValueOffset, 12U, ConnectorA12_GetState() != 0);
  }

  /* AIP values INT[4], only valid for range=2 (0-10V) */
  WriteLe16(input_buffer, kIoInAipValueOffset + 0U, (EipUint16)((g_io_state.ai_range[0] == kAiRange0To10V) ? ConnectorA9_GetAnalogRaw() : 0));
  WriteLe16(input_buffer, kIoInAipValueOffset + 2U, (EipUint16)((g_io_state.ai_range[1] == kAiRange0To10V) ? ConnectorA10_GetAnalogRaw() : 0));
  WriteLe16(input_buffer, kIoInAipValueOffset + 4U, (EipUint16)((g_io_state.ai_range[2] == kAiRange0To10V) ? ConnectorA11_GetAnalogRaw() : 0));
  WriteLe16(input_buffer, kIoInAipValueOffset + 6U, (EipUint16)((g_io_state.ai_range[3] == kAiRange0To10V) ? ConnectorA12_GetAnalogRaw() : 0));

  /* AIOP status bits: A-9..A-12, IO-0 */
  SetBit(input_buffer, kIoInAiopStatusOffset, 0U, g_io_state.ai_range[0] != kAiRange0To10V);
  SetBit(input_buffer, kIoInAiopStatusOffset, 1U, g_io_state.ai_range[1] != kAiRange0To10V);
  SetBit(input_buffer, kIoInAiopStatusOffset, 2U, g_io_state.ai_range[2] != kAiRange0To10V);
  SetBit(input_buffer, kIoInAiopStatusOffset, 3U, g_io_state.ai_range[3] != kAiRange0To10V);
  SetBit(input_buffer, kIoInAiopStatusOffset, 4U, g_io_state.ao_range == kAoRangeDisabled);

  /* DOP status bits: overload/fault and mode conflicts. */
  SetBit(input_buffer, kIoInDopStatusOffset, 0U,
         (g_io_state.ao_range != kAoRangeDisabled) || IoOutputInFault(0));
  SetBit(input_buffer, kIoInDopStatusOffset, 1U, IoOutputInFault(1));
  SetBit(input_buffer, kIoInDopStatusOffset, 2U, IoOutputInFault(2));
  SetBit(input_buffer, kIoInDopStatusOffset, 3U, IoOutputInFault(3));
  SetBit(input_buffer, kIoInDopStatusOffset, 4U, IoOutputInFault(4));
  SetBit(input_buffer, kIoInDopStatusOffset, 5U, IoOutputInFault(5));

  if (g_io_state.ccio_enabled) {
    WriteLe64(input_buffer, kIoInCcioInputValueOffset, Ccio_GetInputBits());
    WriteLe64(input_buffer, kIoInCcioStatusOffset, Ccio_GetStatusBits());
    input_buffer[kIoInCcioBoardCountOffset] = Ccio_GetBoardCount();
  } else {
    WriteLe64(input_buffer, kIoInCcioInputValueOffset, 0ULL);
    WriteLe64(input_buffer, kIoInCcioStatusOffset, 0ULL);
    input_buffer[kIoInCcioBoardCountOffset] = 0U;
  }
  /* Debug heartbeat for implicit input visibility in PLC (reserved bytes 33..35). */
  input_buffer[kIoInReservedOffset + 0U] =
    (EipUint8)(g_implicit_input_debug_counter & 0xFFU);
  input_buffer[kIoInReservedOffset + 1U] =
    (EipUint8)((g_implicit_input_debug_counter >> 8) & 0xFFU);
  input_buffer[kIoInReservedOffset + 2U] =
    (EipUint8)((g_implicit_input_debug_counter >> 16) & 0xFFU);
  g_implicit_input_debug_counter++;

  if (g_io_state.encoder_enabled) {
    WriteLeDint(input_buffer, kIoInEncoderPositionOffset,
                Encoder_GetPosition());
    WriteLeDint(input_buffer, kIoInEncoderVelocityOffset,
                EncoderVelocityScaledForEip());
    WriteLeDint(input_buffer, kIoInEncoderIndexPositionOffset,
                Encoder_GetIndexPosition());
    SetByteBit(input_buffer, kIoInEncoderStatusOffset, 0U,
               Encoder_GetAlarmFlag() != 0);
    SetByteBit(input_buffer, kIoInEncoderStatusOffset, 1U,
               g_io_state.add_to_position_ack);
  } else {
    WriteLeDint(input_buffer, kIoInEncoderPositionOffset, 0);
    WriteLeDint(input_buffer, kIoInEncoderVelocityOffset, 0);
    WriteLeDint(input_buffer, kIoInEncoderIndexPositionOffset, 0);
    SetByteBit(input_buffer, kIoInEncoderStatusOffset, 0U, false);
    SetByteBit(input_buffer, kIoInEncoderStatusOffset, 1U, false);
  }
}

EipStatus ApplicationInitialization(void) {
  CipRunIdleHeaderSetO2T(true);
  CipRunIdleHeaderSetT2O(false);

  ConnectorIO0_Initialize();
  ConnectorIO1_Initialize();
  ConnectorIO2_Initialize();
  ConnectorIO3_Initialize();
  ConnectorIO4_Initialize();
  ConnectorIO5_Initialize();
  ConnectorDI6_Initialize();
  ConnectorDI7_Initialize();
  ConnectorDI8_Initialize();
  ConnectorA9_Initialize();
  ConnectorA10_Initialize();
  ConnectorA11_Initialize();
  ConnectorA12_Initialize();
  Ccio_Initialize();
  Encoder_Initialize();
  MotorAxisExplicitDefaultsInit();
  ClearLinkMotor_Init();
  /* Default ClearLink parity personality; M-connector assembly path overrides on first load. */
  if (BoardMotorMode_Request(1)) {
    s_applied_clearcore_motor_family = 1;
  }

  memset(g_assembly_data_input_stepdir, 0, sizeof(g_assembly_data_input_stepdir));
  memset(g_assembly_data_input_mconnector, 0, sizeof(g_assembly_data_input_mconnector));
  memset(g_assembly_data_output_stepdir, 0, sizeof(g_assembly_data_output_stepdir));
  memset(g_assembly_data_output_mconnector, 0, sizeof(g_assembly_data_output_mconnector));
  memset(g_assembly_data_config_stepdir, 0, sizeof(g_assembly_data_config_stepdir));
  memset(g_assembly_data_config_mconnector, 0, sizeof(g_assembly_data_config_mconnector));

  g_assembly_data_config_stepdir[kIoCfgAiRangeOffset + 0U] = kAiRangeDisabled;
  g_assembly_data_config_stepdir[kIoCfgAiRangeOffset + 1U] = kAiRangeDisabled;
  g_assembly_data_config_stepdir[kIoCfgAiRangeOffset + 2U] = kAiRangeDisabled;
  g_assembly_data_config_stepdir[kIoCfgAiRangeOffset + 3U] = kAiRangeDisabled;
  g_assembly_data_config_stepdir[kIoCfgAoRangeOffset] = kAoRangeDisabled;
  g_assembly_data_config_stepdir[kIoCfgAipFilterOffset + 0U] = 10U;
  g_assembly_data_config_stepdir[kIoCfgAipFilterOffset + 1U] = 10U;
  g_assembly_data_config_stepdir[kIoCfgAipFilterOffset + 2U] = 10U;
  g_assembly_data_config_stepdir[kIoCfgAipFilterOffset + 3U] = 10U;
  WriteLe16(g_assembly_data_config_stepdir, kIoCfgDopPwmFreqOffset, 0U);
  for (int board = 0; board < 8; ++board) {
    g_assembly_data_config_stepdir[kIoCfgCcioFilterOffset + (size_t)board] = 10U;
  }
  WriteLe32(g_assembly_data_config_stepdir, kIoCfgEncoderVelocityResolutionOffset, 100U);
  g_assembly_data_config_stepdir[kIoCfgEncoderConfigurationOffset] = 0U;
  for (int dip = 0; dip < 13; ++dip) {
    size_t entry_offset = kIoCfgDipFilterOffset + (size_t)(dip * 4);
    WriteLe16(g_assembly_data_config_stepdir, entry_offset, 1000U);
    WriteLe16(g_assembly_data_config_stepdir, entry_offset + 2U, 1000U);
  }
  memcpy(g_assembly_data_config_mconnector, g_assembly_data_config_stepdir, 80U);
  /* Default disabled connector numbers (-1) in RAM assembly mirrors (0 would mean IO-0). */
  for (unsigned ax = 0U; ax < kMotorAxisInstanceCount; ++ax) {
    const size_t sd_off =
        kIoCfgStepDirMotor0Offset + (size_t)ax * kIoCfgStepDirMotorBlockSize;
    WriteLe32(g_assembly_data_config_stepdir, sd_off + 0U,
              (EipUint32)kMotorCfgConfigRegisterPdfDefault);
    WriteLeDint(g_assembly_data_config_stepdir, sd_off + 4U, 1);
    WriteLeDint(g_assembly_data_config_stepdir, sd_off + 8U, 1);
    WriteLeDint(g_assembly_data_config_stepdir, sd_off + 12U,
                (int32_t)kMotorCfgMaxDecelerationPdfDefault);
    WriteLeDint(g_assembly_data_config_stepdir, sd_off + 16U, 0);
    WriteLeDint(g_assembly_data_config_stepdir, sd_off + 20U, 0);
    const size_t sd_tail = sd_off + 24U;
    for (unsigned k = 0U; k < 7U; ++k) {
      g_assembly_data_config_stepdir[sd_tail + k] = 0xFFU;
    }
    g_assembly_data_config_stepdir[sd_tail + 7U] = 0U;
    const size_t mc_off =
        kIoCfgMcMotor0Offset + (size_t)ax * kIoCfgMcMotorBlockSize;
    g_assembly_data_config_mconnector[mc_off + 0U] = 0xFFU;
    g_assembly_data_config_mconnector[mc_off + 1U] = 0xFFU;
    g_assembly_data_config_mconnector[mc_off + 2U] = 0xFFU;
    g_assembly_data_config_mconnector[mc_off + 3U] = 30U;
  }

  LoadIoConfigurationFromBuffer(g_assembly_data_config_stepdir,
                                sizeof(g_assembly_data_config_stepdir));
  LoadIoConfigurationFromBuffer(g_assembly_data_config_mconnector,
                                sizeof(g_assembly_data_config_mconnector));
  LoadIoOutputFromBuffer(g_assembly_data_output_stepdir,
                         kClearLinkImplicitAssemblyStepDir);
  LoadIoOutputFromBuffer(g_assembly_data_output_mconnector,
                         kClearLinkImplicitAssemblyMConnector);

  if (InitializeExplicitConfigObjects() != kEipStatusOk) {
    OPENER_TRACE_ERR("ApplicationInitialization: Failed to initialize explicit config objects\n");
    return kEipStatusError;
  }

  /* -- Create EIP assembly objects -- */
  OPENER_TRACE_INFO("ApplicationInitialization: Creating assembly objects...\n");

  CreateAssemblyObject(kAssemblyInputStepDirInstance, g_assembly_data_input_stepdir,
                       sizeof(g_assembly_data_input_stepdir));
  OPENER_TRACE_INFO("ApplicationInitialization: Created input assembly %d (%d bytes)\n",
                    kAssemblyInputStepDirInstance, (int)sizeof(g_assembly_data_input_stepdir));

  CreateAssemblyObject(kAssemblyInputMConnectorInstance, g_assembly_data_input_mconnector,
                       sizeof(g_assembly_data_input_mconnector));
  OPENER_TRACE_INFO("ApplicationInitialization: Created input assembly %d (%d bytes)\n",
                    kAssemblyInputMConnectorInstance, (int)sizeof(g_assembly_data_input_mconnector));

  CreateAssemblyObject(kAssemblyOutputStepDirInstance, g_assembly_data_output_stepdir,
                       sizeof(g_assembly_data_output_stepdir));
  OPENER_TRACE_INFO("ApplicationInitialization: Created output assembly %d (%d bytes)\n",
                    kAssemblyOutputStepDirInstance, (int)sizeof(g_assembly_data_output_stepdir));

  CreateAssemblyObject(kAssemblyOutputMConnectorInstance, g_assembly_data_output_mconnector,
                       sizeof(g_assembly_data_output_mconnector));
  OPENER_TRACE_INFO("ApplicationInitialization: Created output assembly %d (%d bytes)\n",
                    kAssemblyOutputMConnectorInstance, (int)sizeof(g_assembly_data_output_mconnector));

  CreateAssemblyObject(kAssemblyConfigStepDirInstance, g_assembly_data_config_stepdir,
                       sizeof(g_assembly_data_config_stepdir));
  OPENER_TRACE_INFO("ApplicationInitialization: Created config assembly %d (%d bytes)\n",
                    kAssemblyConfigStepDirInstance, (int)sizeof(g_assembly_data_config_stepdir));

  CreateAssemblyObject(kAssemblyConfigMConnectorInstance, g_assembly_data_config_mconnector,
                       sizeof(g_assembly_data_config_mconnector));
  OPENER_TRACE_INFO("ApplicationInitialization: Created config assembly %d (%d bytes)\n",
                    kAssemblyConfigMConnectorInstance, (int)sizeof(g_assembly_data_config_mconnector));

  /* -- Configure the exclusive-owner connection point -- */
  ConfigureExclusiveOwnerConnectionPoint(0, kAssemblyOutputStepDirInstance,
                                         kAssemblyInputStepDirInstance,
                                         kAssemblyConfigStepDirInstance);
  ConfigureExclusiveOwnerConnectionPoint(1, kAssemblyOutputMConnectorInstance,
                                         kAssemblyInputMConnectorInstance,
                                         kAssemblyConfigMConnectorInstance);
  OPENER_TRACE_INFO("ApplicationInitialization: Exclusive owner connection points configured\n");

  {
    CipClass *p_assembly_class = GetCipClass(0x04U);
    OPENER_ASSERT(NULL != p_assembly_class);
    InsertGetSetCallback(p_assembly_class, ExplicitObjectPostSetCallback, kPostSetFunc);
  }

  {
    CipClass *p_tcpip_class = GetCipClass(kCipTcpIpInterfaceClassCode);
    OPENER_ASSERT(NULL != p_tcpip_class);
    InsertGetSetCallback(p_tcpip_class, NvTcpipSetCallback, kNvDataFunc);

    CipClass *p_qos_class = GetCipClass(kCipQoSClassCode);
    OPENER_ASSERT(NULL != p_qos_class);
    InsertGetSetCallback(p_qos_class, NvQosSetCallback, kNvDataFunc);
  }

#if defined(OPENER_ETHLINK_CNTRS_ENABLE) && 0 != OPENER_ETHLINK_CNTRS_ENABLE
  {
    CipClass *p_eth_link_class = GetCipClass(kCipEthernetLinkClassCode);
    InsertGetSetCallback(p_eth_link_class,
                         EthLnkPreGetCallback,
                         kPreGetFunc);
    InsertGetSetCallback(p_eth_link_class,
                         EthLnkPostGetCallback,
                         kPostGetFunc);
    for (int idx = 0; idx < OPENER_ETHLINK_INSTANCE_CNT; ++idx)
    {
      CipAttributeStruct *p_eth_link_attr;
      CipInstance *p_eth_link_inst =
        GetCipInstance(p_eth_link_class, idx + 1);
      OPENER_ASSERT(p_eth_link_inst);

      p_eth_link_attr = GetCipAttribute(p_eth_link_inst, 4);
      p_eth_link_attr->attribute_flags |= (kPreGetFunc | kPostGetFunc);
      p_eth_link_attr = GetCipAttribute(p_eth_link_inst, 5);
      p_eth_link_attr->attribute_flags |= (kPreGetFunc | kPostGetFunc);
    }
  }
#endif

  OPENER_TRACE_INFO("ApplicationInitialization: ClearLink Compatibility Firmware ready\n");
  CipIdentitySetStatusFlags(kConfigured);
  CipIdentityClearStatusFlags(kOwned |
                              kMinorRecoverableFault |
                              kMinorUncoverableFault |
                              kMajorRecoverableFault |
                              kMajorUnrecoverableFault);
  CipIdentitySetExtendedDeviceStatus(kNoIoConnectionsEstablished);
  return kEipStatusOk;
}

/*******************************************************************************
 * HandleApplication -- called cyclically from the OpENer main loop.
 * Handles deferred reset execution after service responses are sent.
 ******************************************************************************/

void HandleApplication(void) {
  for (unsigned m = 0U; m < kMotorAxisInstanceCount; ++m) {
    RefreshMotorInputInstance((CipInstanceNum)(m + 1U));
    RefreshMConnectorInstance((CipInstanceNum)(m + 1U));
  }

  if(g_pending_reset != kPendingResetNone &&
     GetMillis() >= g_pending_reset_deadline_ms) {
    OPENER_TRACE_INFO("HandleApplication: Executing pending reset (%d)\n",
                      g_pending_reset);
    ClearCoreRebootDevice();
    return;
  }

}

void CheckIoConnectionEvent(unsigned int output_assembly_id,
                            unsigned int input_assembly_id,
                            IoConnectionEvent io_connection_event) {

  (void) output_assembly_id;
  (void) input_assembly_id;

  if (io_connection_event == kIoConnectionEventOpened) {
    g_eip_scanner_connected = 1;
    OPENER_TRACE_INFO("CheckIoConnectionEvent: Scanner connected\n");
    CipIdentitySetStatusFlags(kOwned);
    CipIdentityClearStatusFlags(kMinorRecoverableFault |
                                kMinorUncoverableFault |
                                kMajorRecoverableFault |
                                kMajorUnrecoverableFault);
    CipIdentitySetExtendedDeviceStatus(
      kAtLeastOneIoConnectionEstablishedAllInIdleMode);
  }
  else if (io_connection_event == kIoConnectionEventClosed) {
    g_eip_scanner_connected = 0;
    OPENER_TRACE_INFO("CheckIoConnectionEvent: Connection lost\n");
    CipIdentityClearStatusFlags(kOwned | kMinorRecoverableFault);
    CipIdentitySetExtendedDeviceStatus(kNoIoConnectionsEstablished);

    g_ascii_implicit_last_applied_config = UINT32_MAX;
    memset(g_assembly_data_output_stepdir, 0, sizeof(g_assembly_data_output_stepdir));
    memset(g_assembly_data_output_mconnector, 0, sizeof(g_assembly_data_output_mconnector));
    LoadIoOutputFromBuffer(g_assembly_data_output_stepdir,
                           kClearLinkImplicitAssemblyStepDir);
    LoadIoOutputFromBuffer(g_assembly_data_output_mconnector,
                           kClearLinkImplicitAssemblyMConnector);
  }
  else if (io_connection_event == kIoConnectionEventTimedOut) {
    g_eip_scanner_connected = 0;
    OPENER_TRACE_INFO("CheckIoConnectionEvent: Connection timed out\n");
    CipIdentityClearStatusFlags(kOwned);
    CipIdentitySetStatusFlags(kMinorRecoverableFault);
    CipIdentitySetExtendedDeviceStatus(kNoIoConnectionsEstablished);

    g_ascii_implicit_last_applied_config = UINT32_MAX;
    memset(g_assembly_data_output_stepdir, 0, sizeof(g_assembly_data_output_stepdir));
    memset(g_assembly_data_output_mconnector, 0, sizeof(g_assembly_data_output_mconnector));
    LoadIoOutputFromBuffer(g_assembly_data_output_stepdir,
                           kClearLinkImplicitAssemblyStepDir);
    LoadIoOutputFromBuffer(g_assembly_data_output_mconnector,
                           kClearLinkImplicitAssemblyMConnector);
  }
}

/*******************************************************************************
 * AfterAssemblyDataReceived -- process commands from the controller
 *
 * Output instances:
 *   112 (Step/Dir): parity layout command payload
 *   113 (M-Conn):   parity layout command payload
 *   150/151:        config payloads
 ******************************************************************************/
EipStatus AfterAssemblyDataReceived(CipInstance *instance) {
  EipStatus status = kEipStatusOk;

  switch (instance->instance_number) {
    case kAssemblyOutputStepDirInstance:
      LoadIoOutputFromBuffer(g_assembly_data_output_stepdir,
                             kClearLinkImplicitAssemblyStepDir);
      break;
    case kAssemblyOutputMConnectorInstance:
      LoadIoOutputFromBuffer(g_assembly_data_output_mconnector,
                             kClearLinkImplicitAssemblyMConnector);
      break;
    case kAssemblyConfigStepDirInstance:
      LoadIoConfigurationFromBuffer(g_assembly_data_config_stepdir,
                                    sizeof(g_assembly_data_config_stepdir));
      break;
    case kAssemblyConfigMConnectorInstance: {
      LoadIoConfigurationFromBuffer(g_assembly_data_config_mconnector,
                                    sizeof(g_assembly_data_config_mconnector));
      break;
    }
    default:
      OPENER_TRACE_INFO("Unknown assembly instance in AfterAssemblyDataReceived\n");
      break;
  }
  return status;
}

/*******************************************************************************
 * BeforeAssemblyDataSend -- populate status for the controller
 *
 * Input instances:
 *   100 (Step/Dir): parity layout status payload
 *   101 (M-Conn):   parity layout status payload
 ******************************************************************************/
EipBool8 BeforeAssemblyDataSend(CipInstance *pa_pstInstance) {
  if (pa_pstInstance->instance_number == kAssemblyInputStepDirInstance) {
    memset(g_assembly_data_input_stepdir, 0, sizeof(g_assembly_data_input_stepdir));
    PopulateSharedIoInputSegment(g_assembly_data_input_stepdir);
    PackStepDirMotorInputBlocks(g_assembly_data_input_stepdir);
    PopulateAsciiImplicitInputSegment(g_assembly_data_input_stepdir,
                                      kIoInStepDirAsciiSegmentOffset);
  } else if (pa_pstInstance->instance_number == kAssemblyInputMConnectorInstance) {
    memset(g_assembly_data_input_mconnector, 0, sizeof(g_assembly_data_input_mconnector));
    PopulateSharedIoInputSegment(g_assembly_data_input_mconnector);
    PackMConnectorMotorInputFields(g_assembly_data_input_mconnector);
    PopulateAsciiImplicitInputSegment(g_assembly_data_input_mconnector,
                                      kIoInMConnectorAsciiSegmentOffset);
  }
  return true;
}

EipStatus ResetDevice(void) {
  OPENER_TRACE_INFO(
    "ResetDevice: Scheduling reboot after reset response...\n");
  CloseAllConnections();
  CipQosUpdateUsedSetQosValues();
  g_pending_reset = kPendingResetWarm;
  g_pending_reset_deadline_ms = GetMillis() + 50UL;
  return kEipStatusOk;
}

EipStatus ResetDeviceToInitialConfiguration(void) {
  OPENER_TRACE_INFO(
    "ResetDeviceToInitialConfiguration: Scheduling factory reset after response...\n");
  g_tcpip.encapsulation_inactivity_timeout = 120;
  CipQosResetAttributesToDefaultValues();
  OPENER_TRACE_INFO(
    "ResetDeviceToInitialConfiguration: Clearing NVRAM...\n");
  ClearCoreClearNvram();
  g_pending_reset = kPendingResetFactoryDefaults;
  g_pending_reset_deadline_ms = GetMillis() + 50UL;
  return kEipStatusOk;
}

void*
CipCalloc(size_t number_of_elements,
          size_t size_of_element) {
  return calloc(number_of_elements, size_of_element);
}

void CipFree(void *data) {
  free(data);
}

void RunIdleChanged(EipUint32 run_idle_value) {
  OPENER_TRACE_INFO("Run/Idle handler triggered\n");
  if ((0x0001 & run_idle_value) == 1) {
    CipIdentitySetExtendedDeviceStatus(kAtLeastOneIoConnectionInRunMode);
  } else {
    CipIdentitySetExtendedDeviceStatus(
        kAtLeastOneIoConnectionEstablishedAllInIdleMode);
  }
  (void) run_idle_value;
}


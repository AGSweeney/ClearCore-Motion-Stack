---
title: libClearCore
component: libclearcore
level: library
reuse: high
platforms:
  - ClearCore-SAME53
topics:
  - MotorDriver
  - StepGenerator
  - Connector
  - EthernetMgr
  - SysManager
source_paths:
  - ProjectTemplate/libClearCore/inc/ClearCore.h
  - ProjectTemplate/libClearCore/inc/Connector.h
  - ProjectTemplate/libClearCore/src/MotorManager.cpp
status: verified
retrieval:
  questions:
    - How do I initialize ClearCore connectors and Ethernet?
    - Which ISR owns motor step generation?
    - What motor connector modes are supported?
  related:
    - libraries/coordinated-motion/README.md
    - libraries/clearcore-clearlink-bridge/README.md
    - platform/build/build-instructions.md
---

# libClearCore

Board support library for Teknic ClearCore (SAME53): connectors, motors, Ethernet, CCIO, and system timing. **Reuse: high** across firmware personalities.

Artifacts: [A-L01-if](../../artifacts/interfaces/clearcore_umbrella.h), [A-L01-pat](../../artifacts/patterns/motor-mode-step-dir.cpp).

## Purpose and reuse

Provides `ClearCore::*` connector globals and managers used by both EtherNet/IP and EtherCAT firmware.

## Public API surface

Umbrella `ClearCore.h` exports `ConnectorM0`–`M3`, `ConnectorIO*`, `EthernetMgr`, `MotorMgr`, `CcioMgr`, `SysMgr`. Motor modes in `Connector::ConnectorModes` include `CPM_MODE_STEP_AND_DIR` and A/B direct/PWM modes — no quadrature output mode.

## Initialization

`SysManager` / board startup constructs connectors; applications call `EthernetMgr.Setup()` after link detect.

## Runtime lifecycle

`SysManager::SysTickUpdate` / `FastUpdate` refresh managers; motors compute steps in the fast sample path when in step/dir mode.

## Threading and ownership

Bare-metal: FastUpdate ISR owns step generation; application main owns higher-level commands. Not FreeRTOS.

## Configuration

`MotorManager::MotorModeSet(MotorPair, ConnectorModes)` pairs M0/M1 and M2/M3. Step carrier clock via `MotorInputClocking`.

## Error handling

Motor alerts/status registers on `MotorDriver`; validate external inputs in application code, not assertions.

## Memory and resources

Static connector instances; TCC0/TCC1 used for step carrier / PWM.

## Timing and performance

Sample-rate step ISR; `StepsPerSampleMaxSet` tied to TCC period (`MotorManager.cpp`).

## Data formats / wire protocol

Not a network protocol library; motor step/dir electrical signaling only.

## Dependencies

Microchip SAME53 HAL/HRI under `libClearCore/hal` and `hri`.

## Integration points

Used by L05 bridge and P01/P02 firmware mains. Coordinated motion (L02) extends MotorDriver.

## Failure modes

| Symptom | Likely cause |\n|---------|--------------|\n| No steps | Mode not `CPM_MODE_STEP_AND_DIR` or carrier mux disabled |\n| Ethernet silent | `EthernetMgr.Setup` before link |

## Limits and constraints

Four motor connectors; step/dir is the motion-generation path. Quadrature is input-only (`EncoderInput` on DI6–DI8).

## Testing and validation

Exercised by firmware personalities and Enhanced ClearCore Library consumers.

## Portability

Tied to ClearCore pinmux (`HardwareMapping.h`); not portable to arbitrary MCUs without remap.

## Security

Local board HAL; no auth. Network security belongs to protocol stacks.

## Logging and diagnostics

USB serial commonly used by firmware mains; motor status registers for HLFB/alerts.

## Extension points

Enhanced ClearCore Library adds coordinated motion and unit converters in-tree.

## Source evidence

| Claim | Evidence | Level |\n|-------|----------|-------|\n| Step/Dir mode enum | `Connector.h` `CPM_MODE_STEP_AND_DIR` | E1 |\n| MotorModeSet enables step mux | `MotorManager.cpp` `MotorModeSet` | E1 |\n| Umbrella exports connectors | `ClearCore.h` | E1 |


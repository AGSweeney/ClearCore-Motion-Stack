# EtherCAT Personality Firmware

This document describes the EtherCAT personality implemented in
`ProjectTemplate/EtherCATSlaveFirmware` and the companion master test
application in `EtherCATMaster`.

## Conformance Status

This is a minimal, non-conformant EtherCAT implementation for research and
interoperability testing.

It is not an ETG-conformant EtherCAT slave and should not be represented as one.
The firmware emulates enough EtherCAT Slave Controller behavior to exchange raw
cyclic process data with selected masters, but it does not implement a complete
ESC, mailbox stack, CoE object dictionary, CiA-402 profile, distributed clocks,
or formal conformance-test behavior.

The implementation is best understood as a software EtherCAT-frame personality
running on a normal ClearCore Ethernet MAC. It can emulate register interfaces
and fixed PDO behavior, but it cannot provide the deterministic wire-time
timestamping, clock synchronization, or SYNC0/SYNC1 edge generation expected
from hardware ESC distributed-clock implementations.

## Scope

The firmware is a non-conformant ClearCore EtherCAT slave personality focused on
raw cyclic process data for ClearCore I/O and CCIO-8 expansion I/O.

- EtherCAT traffic is carried as raw Ethernet frames using EtherType `0x88A4`.
- The slave exposes a fixed process image:
  - RxPDO, master to slave: `40` bytes
  - TxPDO, slave to master: `64` bytes
- CiA-402, mailbox protocols, CoE object dictionary services, distributed clocks,
  and motion control are intentionally out of scope for this personality.
- ClearCore onboard I/O and CCIO are controlled directly by the firmware
  application loop, not through the existing EtherNet/IP OpENer application.

## Source Layout

- `ProjectTemplate/EtherCATSlaveFirmware/main.cpp`
  - ClearCore Ethernet bring-up
  - ClearCore onboard I/O mode/output handling
  - CCIO enable, direction, output, and readback handling
  - firmware-side timing instrumentation
  - periodic call into the EtherCAT transport module
- `ProjectTemplate/EtherCATSlaveFirmware/protocol/ethercat_slave/`
  - raw EtherCAT frame handling
  - minimal ESC register emulation
  - SII EEPROM image generation
  - SyncManager and FMMU-aware process image access
  - fixed PDO structs in `ethercat_pdo_layout.h`
- `ProjectTemplate/EtherCATSlaveFirmware/esi/ClearCore_EtherCAT_Slave.xml`
  - EtherCAT Slave Information XML for EtherCAT engineering tools
  - declares the same identity, SM windows, and fixed PDO layouts as firmware
- `ProjectTemplate/LwIP/LwIP/port/include/lwip_hooks.h`
  - lwIP unknown-EtherType hook integration
- `EtherCATMaster/`
  - SOEM-based Windows/Qt master utility
  - manual logical process-data fallback for masters that do not map PDO bytes
  - live ClearCore/CCIO/status/timing views

## Layering

The EtherCAT transport module is deliberately isolated from ClearCore hardware
control. It owns frame parsing, ESC emulation, and the process image only.

Allowed transport dependencies:

- lwIP networking headers and frame buffers
- fixed PDO structs from `ethercat_pdo_layout.h`

Forbidden transport dependencies:

- `MotorManager`, coordinated motion planners, or connector write APIs
- OpENer bridge/application files
- ClearLink EtherNet/IP state or object-class implementation

ClearCore connector access belongs in `main.cpp`, which reads the consumed
command image and populates the produced status image.

## EtherCAT Architecture

The firmware does not use a dedicated EtherCAT Slave Controller ASIC. It emulates
the minimum ESC behavior needed by the current SOEM master and similar scanners.
This emulation is pragmatic and incomplete; passing discovery or cyclic I/O with
one master does not imply EtherCAT conformance.

Implemented behavior:

- EtherType `0x88A4` receive path through lwIP unknown-protocol hook.
- EtherCAT datagram parsing for AP/FPR/Broadcast and logical read/write forms.
- Minimal AL state handling for `INIT`, `PRE-OP`, `SAFE-OP`, and `OP`.
- Minimal ESC register space with station address, AL control/status, DL status,
  EEPROM registers, SyncManager registers, and FMMU registers.
- SII EEPROM contents for identity, strings, SM2/SM3, RxPDO, and TxPDO declarations.
- SM2 output window at physical `0x1000`.
- SM3 input window at physical `0x1100`.
- FMMU-aware logical addressing when the master programs FMMUs.
- Logical-address fallback for the SOEM manual I/O path.
- Discovery-facing ESC information registers report supported FMMU count, supported
  SyncManager count, emulated RAM size, port descriptor, DL status, and EEPROM
  readiness for stricter scanners such as TwinCAT.

Process image ownership:

- Master writes copy into `EthercatPdoCommand`.
- Firmware application reads `EthercatPdoCommand`.
- Firmware application writes `EthercatPdoStatus`.
- Master reads `EthercatPdoStatus`.

LRW behavior:

- Combined LRW payloads contain output bytes followed by input bytes.
- Status/input data is written at the SM3/input offset after the SM2/output bytes.
- Status data is not written at payload byte `0` for LRW unless the input window
  itself begins there.

## Runtime Loop

The firmware loop:

1. Reads the latest command image.
2. Applies onboard `IO0..IO5` direction changes.
3. Applies `A9..A12` analog/digital mode changes.
4. Applies onboard output levels only for pins configured as outputs.
5. Applies CCIO enable/disable on `COM-1`.
6. Applies CCIO direction and output bits when CCIO is enabled.
7. Samples onboard and CCIO inputs.
8. Populates the TxPDO status image.
9. Services Ethernet and EtherCAT transport.
10. Updates firmware timing fields.
11. Delays approximately `1 ms`.

Onboard digital readback uses real-time input reads for live status, avoiding
ClearCore connector filter latency in the UI/status path. CCIO readback comes
from `CcioMgr.InputState()`, which is the CCIO manager's filtered input image.

## Onboard I/O Behavior

`IO0..IO5` are bidirectional digital points. Direction and output level are
separate fields:

- Direction bit `1`: `OUTPUT_DIGITAL`
- Direction bit `0`: `INPUT_DIGITAL`
- Output levels are applied only to pins whose applied direction is output.
- Input-mode pins ignore requested output levels and report live digital state.

`DI6..DI8` are always digital inputs.

`A9..A12` can be selected per channel as:

- `INPUT_ANALOG`
- `INPUT_DIGITAL`

When an `A` channel is in analog mode, its raw analog value is reported in packed
analog fields. When in digital mode, its live digital state is reported in the
digital status bank.

Supply voltage is reported in centivolts.

## CCIO Behavior

CCIO follows the same enable model used by the EtherNet/IP personality:

- Enable: `ConnectorCOM1.Mode(Connector::CCIO)` followed by
  `ConnectorCOM1.PortOpen()`.
- Disable: `ConnectorCOM1.PortClose()`.
- `PortOpen()` performs CCIO discovery and populates `CcioMgr.CcioCount()`.
- CCIO is disabled by default.

CCIO supports up to eight CCIO-8 boards:

- Board A: `CCIOA0..CCIOA7`
- Board B: `CCIOB0..CCIOB7`
- ...
- Board H: `CCIOH0..CCIOH7`

Bit indexing is linear:

- bit `0`: `CCIOA0`
- bit `7`: `CCIOA7`
- bit `8`: `CCIOB0`
- bit `63`: `CCIOH7`

Each CCIO pin has a requested direction and output level:

- Direction bit `1`: output
- Direction bit `0`: input
- Output bits are applied only for pins whose applied direction is output.
- Status includes CCIO input bits, overload bits, link-broken indication, board
  count, and applied direction echo.

## Timing Instrumentation

The firmware reports timing values in microseconds:

- `firmware_loop_period_us`: firmware cycle start-to-start period.
- `firmware_loop_runtime_us`: firmware loop work time before delay.
- `firmware_loop_jitter_us`: absolute difference from the nominal `1 ms` loop
  period.
- `firmware_transport_us`: time spent servicing Ethernet and EtherCAT transport
  during the firmware loop.

The Qt master additionally measures master-side values:

- cycle period
- loop runtime
- loop jitter
- process-data frame latency
- update consistency

## PDO Layout

All multi-byte fields are little-endian. The structs are packed with
`#pragma pack(push, 1)` in `ethercat_pdo_layout.h`.

### RxPDO: Master To Slave, `EthercatPdoCommand`, 40 Bytes

| Byte Offset | Size | Field | Description |
| --- | ---: | --- | --- |
| `0` | `2` | `control_word` | bits `0..5`: `IO0..IO5` output level request |
| `2` | `1` | `mode_of_operation` | reserved compatibility field |
| `3` | `1` | `reserved0` | bits `0..3`: `A9..A12` analog mode request (`1=analog`) |
| `4` | `4` | `target_position_counts` | reserved compatibility field |
| `8` | `4` | `target_velocity_counts_per_sec` | reserved compatibility field |
| `12` | `2` | `target_torque_tenths_percent` | reserved compatibility field |
| `14` | `2` | `reserved1` | bits `0..5`: `IO0..IO5` direction request; bit `15`: direction-valid |
| `16` | `4` | `sequence` | command sequence; echoed as `sequence_ack` |
| `20` | `2` | `ccio_control` | bit `0`: enable CCIO on `COM-1` |
| `22` | `2` | `ccio_reserved` | reserved, write `0` |
| `24` | `8` | `ccio_output_bits` | `CCIOA0..CCIOH7` output level request |
| `32` | `8` | `ccio_direction_bits` | `CCIOA0..CCIOH7` direction request (`1=output`) |

### TxPDO: Slave To Master, `EthercatPdoStatus`, 64 Bytes

| Byte Offset | Size | Field | Description |
| --- | ---: | --- | --- |
| `0` | `2` | `status_word` | bits `0..5`: `IO0..IO5` live digital state |
| `2` | `1` | `mode_of_operation_display` | bits `0..3`: applied `A9..A12` analog mode |
| `3` | `1` | `fault_code` | reserved, currently `0` |
| `4` | `4` | `actual_position_counts` | low 16 bits: `A9` raw; high 16 bits: `A10` raw |
| `8` | `4` | `actual_velocity_counts_per_sec` | low 16 bits: `A11` raw; high 16 bits: `A12` raw |
| `12` | `2` | `actual_torque_tenths_percent` | supply voltage in centivolts |
| `14` | `2` | `reserved0` | onboard extended digital state and direction echo |
| `16` | `4` | `sequence_ack` | latest consumed `sequence` |
| `20` | `2` | `ccio_status` | CCIO enable/link/overload status bits |
| `22` | `1` | `ccio_board_count` | discovered CCIO-8 board count |
| `23` | `1` | `ccio_reserved` | reserved |
| `24` | `8` | `ccio_input_bits` | `CCIOA0..CCIOH7` live input image |
| `32` | `8` | `ccio_status_bits` | CCIO overload bits; bit `63` also indicates link broken |
| `40` | `8` | `ccio_direction_bits` | applied `CCIOA0..CCIOH7` direction echo |
| `48` | `4` | `firmware_loop_period_us` | firmware loop period |
| `52` | `4` | `firmware_loop_runtime_us` | firmware loop runtime before delay |
| `56` | `4` | `firmware_loop_jitter_us` | absolute error from nominal `1 ms` period |
| `60` | `4` | `firmware_transport_us` | Ethernet/EtherCAT service time |

### `reserved0` Bit Map In TxPDO

| Bit | Meaning |
| ---: | --- |
| `0` | `DI6` digital state |
| `1` | `DI7` digital state |
| `2` | `DI8` digital state |
| `3` | `A9` digital state, valid when `A9` is digital |
| `4` | `A10` digital state, valid when `A10` is digital |
| `5` | `A11` digital state, valid when `A11` is digital |
| `6` | `A12` digital state, valid when `A12` is digital |
| `8..13` | applied `IO0..IO5` direction echo (`1=output`) |

### `ccio_status` Bit Map In TxPDO

| Bit | Meaning |
| ---: | --- |
| `0` | CCIO enabled |
| `1` | CCIO link broken |
| `2` | one or more CCIO overload bits active |

## Master Integration Notes

- Use `ProjectTemplate/EtherCATSlaveFirmware/esi/ClearCore_EtherCAT_Slave.xml`
  when an EtherCAT engineering tool requires an ESI file.
- Treat `status_word` as live `IO0..IO5` state only.
- Decode onboard `DI6..DI8`, `A9..A12` digital state, and `IO0..IO5`
  direction from `reserved0`.
- Write `IO0..IO5` direction to `reserved1[0..5]` and set
  `reserved1[15]`.
- Do not use `control_word[8..13]` for new masters; that range is accepted only
  as a legacy fallback.
- Decode `A9..A12` display mode from `mode_of_operation_display[0..3]`.
- Decode supply voltage as `actual_torque_tenths_percent / 100.0`.
- Decode CCIO bit fields starting at `CCIOA0` bit `0`.
- Require the full `40` byte RxPDO and `64` byte TxPDO. If the master maps a
  smaller process image, use the manual logical I/O path or fix PDO mapping.

## Compatibility Behavior

`IO0..IO5` direction source priority:

1. `command.reserved1[0..5]` when `command.reserved1[15]` is set.
2. Legacy fallback from `control_word[8..13]` when the direction-valid flag is
   not set.
3. Retain the current applied direction.

Firmware powers up with `IO0..IO5` in output mode for compatibility with older
test masters that only drive `control_word[0..5]`.

## Validated Runtime Behavior

The current implementation has been validated with the companion SOEM/Qt master
for:

- slave discovery
- transition to operational state
- cyclic process-data exchange
- `IO0..IO5` direction and output control
- `DI6..DI8` live digital readback
- `A9..A12` digital/analog mode selection and raw analog readback
- supply voltage reporting
- CCIO enable/discovery on `COM-1`
- one CCIO-8 board with live input readback and output direction controls
- firmware and master timing display

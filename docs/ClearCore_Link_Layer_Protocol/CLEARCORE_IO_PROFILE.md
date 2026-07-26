<!--
Copyright (c) 2026 Adam G. Sweeney
SPDX-License-Identifier: MIT

Purpose: ClearCore I/O process-data profile for the LEAP protocol suite.
-->

# ClearCore I/O Profile

This profile is the first LEAP process-data profile. It preserves the useful
fixed command and status images from the current experimental EtherCAT
personality while naming the behavior as a LEAP profile instead of a PDO map.

Profile ID: `0x00010001` (`REVIEW`)

Profile name: `ClearCoreFixedIoV1`

## 1. Profile Scope

The profile exposes:

- onboard bidirectional digital I/O `IO0..IO5`
- onboard digital inputs `DI6..DI8`
- onboard selectable analog/digital inputs `A9..A12`
- supply voltage reporting in centivolts
- optional CCIO-8 expansion I/O on `COM-1`
- application sequence acknowledgement
- firmware loop and transport timing diagnostics

The profile does not define servo motion, distributed clocks, mailbox traffic,
EtherCAT object dictionary behavior, or EtherNet/IP object behavior.

## 2. Endpoint Table

| Endpoint ID | Direction | Size | Name | Description |
| ---: | --- | ---: | --- | --- |
| `0x0001` | controller to device | `40` | `CommandImage` | command image consumed by firmware |
| `0x0002` | device to controller | `64` | `StatusImage` | status image produced by firmware |

LEAP `EXCHANGE_ENDPOINTS` writes endpoint `0x0001` and reads endpoint
`0x0002`. The response places status bytes after the 40 command bytes in the
combined exchange payload.

All multi-byte fields are little-endian. The packed layout MUST remain stable
for profile ID `0x00010001`.

## 3. Command Image

Endpoint: `CommandImage`

Size: `40` bytes

| Byte Offset | Size | Field | Description |
| ---: | ---: | --- | --- |
| `0` | `2` | `control_word` | bits `0..5`: `IO0..IO5` output level request |
| `2` | `1` | `mode_of_operation` | reserved compatibility field, write `0` |
| `3` | `1` | `analog_mode_request` | bits `0..3`: `A9..A12` analog mode request (`1=analog`) |
| `4` | `4` | `target_position_counts` | reserved compatibility field, write `0` |
| `8` | `4` | `target_velocity_counts_per_sec` | reserved compatibility field, write `0` |
| `12` | `2` | `target_torque_tenths_percent` | reserved compatibility field, write `0` |
| `14` | `2` | `io_direction_request` | bits `0..5`: `IO0..IO5` direction request; bit `15`: direction-valid |
| `16` | `4` | `sequence` | application command sequence echoed by status |
| `20` | `2` | `ccio_control` | bit `0`: enable CCIO on `COM-1` |
| `22` | `2` | `ccio_reserved` | reserved, write `0` |
| `24` | `8` | `ccio_output_bits` | `CCIOA0..CCIOH7` output level request |
| `32` | `8` | `ccio_direction_bits` | `CCIOA0..CCIOH7` direction request (`1=output`) |

### 3.1 `control_word`

| Bit | Meaning |
| ---: | --- |
| `0` | requested output level for `IO0` |
| `1` | requested output level for `IO1` |
| `2` | requested output level for `IO2` |
| `3` | requested output level for `IO3` |
| `4` | requested output level for `IO4` |
| `5` | requested output level for `IO5` |
| `6..15` | reserved for future profile versions, write `0` |

Output levels are applied only to pins whose applied direction is output.
Input-mode pins ignore requested output levels and report live input state.

### 3.2 `analog_mode_request`

| Bit | Meaning |
| ---: | --- |
| `0` | `A9` analog mode request |
| `1` | `A10` analog mode request |
| `2` | `A11` analog mode request |
| `3` | `A12` analog mode request |
| `4..7` | reserved, write `0` |

`1` selects analog input mode. `0` selects digital input mode.

### 3.3 `io_direction_request`

| Bit | Meaning |
| ---: | --- |
| `0` | `IO0` direction request (`1=output`, `0=input`) |
| `1` | `IO1` direction request (`1=output`, `0=input`) |
| `2` | `IO2` direction request (`1=output`, `0=input`) |
| `3` | `IO3` direction request (`1=output`, `0=input`) |
| `4` | `IO4` direction request (`1=output`, `0=input`) |
| `5` | `IO5` direction request (`1=output`, `0=input`) |
| `6..14` | reserved, write `0` |
| `15` | direction-valid flag |

When bit `15` is `1`, bits `0..5` are applied as the direction request. When
bit `15` is `0`, the device retains the current applied direction. LEAP should
not carry forward legacy fallback direction encoding from older EtherCAT test
masters.

### 3.4 CCIO Bit Numbering

CCIO bits are linear:

| Bit | Point |
| ---: | --- |
| `0` | `CCIOA0` |
| `7` | `CCIOA7` |
| `8` | `CCIOB0` |
| `15` | `CCIOB7` |
| `63` | `CCIOH7` |

CCIO is disabled by default. When `ccio_control[0]` is `1`, firmware opens
`COM-1` in CCIO mode and performs CCIO discovery. When the bit returns to `0`,
firmware closes the port and reports zero CCIO board count.

## 4. Status Image

Endpoint: `StatusImage`

Size: `64` bytes

| Byte Offset | Size | Field | Description |
| ---: | ---: | --- | --- |
| `0` | `2` | `status_word` | bits `0..5`: `IO0..IO5` live digital state |
| `2` | `1` | `mode_of_operation_display` | bits `0..3`: applied `A9..A12` analog mode |
| `3` | `1` | `fault_code` | profile fault code, currently `0` when clear |
| `4` | `4` | `actual_position_counts` | low 16 bits: `A9` raw; high 16 bits: `A10` raw |
| `8` | `4` | `actual_velocity_counts_per_sec` | low 16 bits: `A11` raw; high 16 bits: `A12` raw |
| `12` | `2` | `actual_torque_tenths_percent` | supply voltage in centivolts |
| `14` | `2` | `extended_status` | onboard extended digital state and direction echo |
| `16` | `4` | `sequence_ack` | latest consumed command `sequence` |
| `20` | `2` | `ccio_status` | CCIO enable/link/overload summary bits |
| `22` | `1` | `ccio_board_count` | discovered CCIO-8 board count |
| `23` | `1` | `ccio_reserved` | reserved |
| `24` | `8` | `ccio_input_bits` | `CCIOA0..CCIOH7` live input image |
| `32` | `8` | `ccio_status_bits` | CCIO overload bits; bit `63` also indicates link broken |
| `40` | `8` | `ccio_direction_bits` | applied `CCIOA0..CCIOH7` direction echo |
| `48` | `4` | `firmware_loop_period_us` | firmware cycle start-to-start period |
| `52` | `4` | `firmware_loop_runtime_us` | firmware loop runtime before delay |
| `56` | `4` | `firmware_loop_jitter_us` | absolute error from nominal loop period |
| `60` | `4` | `firmware_transport_us` | Ethernet/LEAP service time |

### 4.1 `status_word`

| Bit | Meaning |
| ---: | --- |
| `0` | live state for `IO0` |
| `1` | live state for `IO1` |
| `2` | live state for `IO2` |
| `3` | live state for `IO3` |
| `4` | live state for `IO4` |
| `5` | live state for `IO5` |
| `6..15` | reserved |

For output-mode pins, live state MAY reflect the commanded output readback. For
input-mode pins, live state MUST reflect the physical input state.

### 4.2 `extended_status`

| Bit | Meaning |
| ---: | --- |
| `0` | `DI6` digital state |
| `1` | `DI7` digital state |
| `2` | `DI8` digital state |
| `3` | `A9` digital state, valid when `A9` is digital |
| `4` | `A10` digital state, valid when `A10` is digital |
| `5` | `A11` digital state, valid when `A11` is digital |
| `6` | `A12` digital state, valid when `A12` is digital |
| `7` | reserved |
| `8..13` | applied `IO0..IO5` direction echo (`1=output`) |
| `14..15` | reserved |

### 4.3 `ccio_status`

| Bit | Meaning |
| ---: | --- |
| `0` | CCIO enabled |
| `1` | CCIO link broken |
| `2` | one or more CCIO overload bits active |
| `3..15` | reserved |

## 5. Timing Semantics

Timing values are unsigned microsecond counters sampled by firmware:

- `firmware_loop_period_us`: cycle start-to-start period.
- `firmware_loop_runtime_us`: work time before the firmware loop delay.
- `firmware_loop_jitter_us`: absolute difference from the profile nominal loop
  period.
- `firmware_transport_us`: time spent servicing Ethernet and LEAP transport.

The profile nominal loop period is `1000 us` unless overridden by a future
profile configuration object.

## 6. Safe Output Behavior

On power-up:

- `IO0..IO5` default to input mode.
- `A9..A12` default to digital input mode.
- CCIO is disabled and `COM-1` is closed.
- command and status images are zeroed before first valid exchange.

Outputs may be applied only when LEAP state is `OP`, the active controller owns
the lease, and a complete 40-byte `CommandImage` has passed validation.

On owner lease timeout, process-data timeout, malformed cyclic frame, or explicit
transition to `SAFE`, firmware MUST stop applying stale output changes and apply
the configured safe state. Initial safe state for this profile is:

- retain connector modes unless profile configuration says otherwise
- drive onboard outputs low when they are configured as outputs
- close `COM-1` and disable CCIO unless profile configuration permits hold-last
- report timeout/fault state through LEAP-DIAG counters and `fault_code`

## 7. Fault Codes

`fault_code` is profile-local. Initial assignments:

| Code | Meaning |
| ---: | --- |
| `0x00` | no profile fault |
| `0x01` | owner lease expired |
| `0x02` | process-data watchdog expired |
| `0x03` | command endpoint length invalid |
| `0x04` | unsupported direction or mode request |
| `0x05` | CCIO discovery failed |
| `0x06` | CCIO link broken |
| `0x07` | CCIO overload active |
| `0x08` | application loop overrun |

Fault code behavior needs review before implementation. Some conditions may be
warnings rather than state-machine faults.

## 8. LEAP Exchange Example

For the fixed ClearCore profile:

- `write_endpoint_id = 0x0001`
- `read_endpoint_id = 0x0002`
- `write_length = 40`
- `read_length = 64`
- `write_data` is the command image
- `read_reservation` is 64 zero bytes in the request
- response writes the status image starting at byte offset `16 + 40`

This preserves the important output-before-input packing rule from the current
LRW implementation while making it explicit in LEAP.

## 9. Extension Policy

The profile ID is immutable. Compatible extensions should use:

- LEAP-DIR objects for optional configuration
- LEAP-DIAG objects for optional counters
- new endpoints with new endpoint IDs
- TLVs in management or directory replies

Incompatible changes to the 40-byte command image or 64-byte status image require
a new profile ID. Controllers must select the profile they understand before
requesting `OP`.

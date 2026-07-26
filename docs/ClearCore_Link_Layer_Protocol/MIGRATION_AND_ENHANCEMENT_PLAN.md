<!--
Copyright (c) 2026 Adam G. Sweeney
SPDX-License-Identifier: MIT

Purpose: Extraction and enhancement plan from the EtherCAT personality to LEAP.
-->

# Migration And Enhancement Plan

This plan describes how to extract the project-owned parts of the current
EtherCAT-derived personality into LEAP without breaking the existing
experimental firmware or master utility.

## 1. Migration Principles

- Keep the existing EtherCAT personality intact until LEAP has its own firmware
  and master validation path.
- Reuse concepts, not EtherCAT names or conformance surfaces.
- Preserve the current ClearCore I/O behavior that has already been validated.
- Make ownership, watchdog, fault handling, and discovery explicit in LEAP.
- Avoid changes to external referenced library projects unless explicitly
  requested.
- Keep transport code independent from ClearCore connector hardware access.

## 2. Existing Concept Extraction

| Current EtherCAT Personality Concept | LEAP Replacement |
| --- | --- |
| EtherType `0x88A4` | LEAP assigned or lab EtherType |
| EtherCAT datagram header | LEAP 32-byte frame header |
| AP/FPR/Broadcast register access | LEAP-DIR object reads and writes |
| LRD/LWR/LRW logical process access | LEAP-PD endpoint read/write/exchange |
| Working counter | explicit status response and accepted byte range |
| AL states `INIT/PRE-OP/SAFE-OP/OP` | LEAP `INIT/CONFIGURED/SAFE/OP/FAULT` |
| ESC register map | LEAP-DIR live object directory |
| SII EEPROM image | LEAP-DIR identity/profile/endpoint descriptors |
| ESI XML | generated or exported LEAP profile manifest |
| SyncManager windows | endpoint descriptors |
| FMMU logical mapping | fixed endpoint IDs and endpoint offsets |
| PDO command/status structs | ClearCore I/O profile endpoints |
| LRW output/input packing | LEAP-PD `EXCHANGE_ENDPOINTS` layout |

## 3. Candidate Source Boundaries

Current files with reusable behavior:

- `ProjectTemplate/EtherCATSlaveFirmware/protocol/ethercat_slave/ethercat_pdo_layout.h`
  - preserve field order as the initial `ClearCoreFixedIoV1` profile schema
- `ProjectTemplate/EtherCATSlaveFirmware/main.cpp`
  - preserve application-side I/O application and status population behavior
- `ProjectTemplate/EtherCATSlaveFirmware/protocol/ethercat_slave/ethercat_slave.cpp`
  - reuse validation lessons, process image ownership, counters, and LRW packing
  - replace EtherCAT datagram parsing, ESC registers, SII, SM, and FMMU logic
- `ProjectTemplate/LwIP/LwIP/port/ethercat_unknown_eth_hook.c`
  - generalize the weak unknown-EtherType hook pattern for LEAP
- `EtherCATMaster/src/EthercatMasterThread.cpp`
  - reuse cyclic timing, UI snapshot concepts, and manual exchange flow
  - replace SOEM calls with a LEAP raw Ethernet controller path

## 4. Proposed Implementation Layout

The implementation should live beside, not inside, the current EtherCAT
personality until validated.

```text
ProjectTemplate/LeapFirmware/
  main.cpp
  protocol/leap/
    leap.h
    leap.cpp
    leap_frame.h
    leap_services.h
    leap_directory.h
    clearcore_io_profile.h
  lwip_hooks.h
  esi-or-profile-export/
    ClearCoreFixedIoV1.leap.json

LeapMaster/
  src/
    LeapTransport.*
    LeapControllerThread.*
    ClearCoreIoProfile.*
```

The JSON profile export is optional but useful for future tooling. It should be
generated from the same schema constants used by firmware once implementation
starts.

## 5. Firmware Extraction Steps

1. Create `ProjectTemplate/LeapFirmware` from the working ClearCore Ethernet
   bring-up pattern.
2. Add a new `protocol/leap` module with no EtherCAT naming.
3. Define packed LEAP header structs and helper functions:
   - little-endian read/write helpers
   - header length validation
   - CRC-16 header validation
   - CRC-32C payload validation
   - sequence duplicate detection
4. Move the current 40-byte command and 64-byte status images into
   `clearcore_io_profile.h`.
5. Replace ESC process RAM with endpoint buffers:
   - endpoint `0x0001`: command image
   - endpoint `0x0002`: status image
6. Implement LEAP-DISC:
   - `HELLO`
   - `HELLO_REPLY`
   - `IDENTIFY`
   - `IDENTIFY_REPLY`
7. Implement LEAP-MGMT:
   - session open/close
   - owner lease
   - heartbeat
   - state transitions
   - fault reset
8. Implement LEAP-DIR read-only objects:
   - identity
   - protocol version
   - service list
   - active profile
   - endpoint descriptors
9. Implement LEAP-PD:
   - `EXCHANGE_ENDPOINTS`
   - status response
   - command length validation
   - process watchdog
10. Implement LEAP-DIAG counters and timing.
11. Port the ClearCore application loop from the EtherCAT personality, keeping
    hardware access outside the LEAP transport.
12. Add safe-output behavior for owner lease and process-data timeout.

## 6. Master Extraction Steps

1. Create a LEAP master utility beside the existing `EtherCATMaster`.
2. Keep the useful Qt snapshot fields for I/O, CCIO, timing, and errors.
3. Replace SOEM discovery with LEAP-DISC broadcast and reply parsing.
4. Replace SOEM state management with LEAP-MGMT session and state requests.
5. Replace mapped PDO/manual logical I/O with LEAP-PD `EXCHANGE_ENDPOINTS`.
6. Add a profile parser for LEAP-DIR endpoint descriptors.
7. Display owner lease, protocol counters, and timeout/fault state.
8. Keep raw adapter selection and administrator/Npcap requirements documented.

## 7. Compatibility Decisions

### Keep

- fixed 40-byte command image and 64-byte status image for the first profile
- output-before-input combined exchange payload behavior
- CCIO enable/disable on `COM-1`
- `IO0..IO5`, `DI6..DI8`, `A9..A12`, supply, CCIO, and timing semantics
- transport/application separation
- firmware loop timing diagnostics

### Drop

- EtherCAT frame type and datagram command set
- ESC register emulation
- SII EEPROM categories
- ESI XML as the canonical profile source
- SyncManager and FMMU programming
- AL status register names
- working counter as a success signal
- legacy `control_word[8..13]` direction fallback for new LEAP controllers

### Add

- protocol magic/version header
- payload integrity check
- explicit error payloads
- live directory objects
- owner sessions and leases
- explicit watchdog-to-safe behavior
- profile IDs and endpoint IDs
- protocol counters for rejected frames and state failures
- future authentication extension point

## 8. Review Items Before Implementation

- Confirm the permanent protocol name and abbreviation.
- Decide whether the first firmware project should be a template or a concrete
  active target.
- Choose production EtherType strategy.
- Confirm whether lab default `0x88B5` is acceptable for local experimental use.
- Choose CRC algorithms and available firmware implementation.
- Decide whether safe output on timeout should drive outputs low or retain last
  connector mode with output updates disabled.
- Decide whether CCIO should close immediately on `SAFE`, only on `FAULT`, or be
  configurable.
- Define how profile manifests are exported for master tooling.
- Decide whether LEAP should support multi-device line topology later or remain
  switched Ethernet star only.

## 9. Validation Plan

### Unit-Level Validation

- header parser accepts valid frames and rejects malformed lengths
- CRC failures are rejected without state changes
- unsupported service and message IDs return deterministic errors
- TLV parser skips unknown valid TLVs and rejects invalid lengths
- owner lease state machine rejects non-owner output writes
- process watchdog transitions from `OP` to `SAFE`
- `EXCHANGE_ENDPOINTS` writes status at offset `16 + write_length`

### Firmware Bench Validation

- discovery returns identity and profile data
- session open grants owner lease
- state path reaches `OP`
- cyclic exchange updates `sequence_ack`
- `IO0..IO5` direction and output control works
- `DI6..DI8` live readback works
- `A9..A12` analog/digital selection works
- supply voltage reports centivolts
- CCIO enable/discovery works on `COM-1`
- CCIO input, output, direction, link, and overload status work
- unplugged master or stopped cyclic exchange causes safe transition

### Master Validation

- adapter selection and raw transmit/receive work on Windows
- discovery finds one device on an isolated adapter
- directory parser builds the endpoint table
- cyclic exchange meets the target loop period
- UI reports firmware and master timing
- diagnostics counters increment on expected faults
- non-owner write attempts are rejected and visible

## 10. Future Enhancements

- assigned production EtherType
- profile manifest generator and validator
- authenticated owner sessions
- VLAN priority and traffic class guidance
- optional time synchronization service
- multi-device addressing and grouping
- retained profile configuration object
- firmware-side event ring buffer
- conformance test harness with golden frame vectors
- Wireshark dissector for LEAP
- bridge mode from LEAP process data to EtherNet/IP assemblies
- optional motion-control profile after the I/O profile is stable

## 11. Documentation Updates After Implementation

When implementation starts, update this folder with:

- exact EtherType and version constants
- generated frame examples in hexadecimal
- JSON profile manifest schema
- controller state diagrams
- failure-mode timing measurements
- master utility operating instructions
- known interoperability limits

<!--
Copyright (c) 2026 Adam G. Sweeney
SPDX-License-Identifier: MIT

Purpose: Overview for the Layer2 Ethernet Automation Protocol suite.
-->

# LEAP Protocol Suite

LEAP, the Layer2 Ethernet Automation Protocol, is a project-owned raw Ethernet
protocol suite extracted from the experimental EtherCAT personality and extended
into a standalone automation protocol. It keeps the useful link-layer ideas from
the current firmware, especially deterministic cyclic exchange, explicit state
transitions, compact process images, diagnostics, and output/input exchange
semantics, while removing EtherCAT conformance claims and dependencies on
EtherCAT-specific ESC, SII, ESI, SyncManager, FMMU, and working-counter behavior.

This folder is a design and review package. It is intentionally separate from
`ProjectTemplate/EtherCATSlaveFirmware` until the protocol is approved and an
implementation can be created without destabilizing the existing test firmware.

## Source Basis

The current extraction source is:

- `ProjectTemplate/EtherCATSlaveFirmware/protocol/ethercat_slave/`
- `ProjectTemplate/EtherCATSlaveFirmware/main.cpp`
- `ProjectTemplate/EtherCATSlaveFirmware/esi/ClearCore_EtherCAT_Slave.xml`
- `EtherCATMaster/`
- `docs/EtherCAT_PERSONALITY.md`

The key protocol lessons retained from that implementation are:

- Raw Ethernet frames can provide a simple deterministic I/O path on ClearCore.
- A fixed master-to-device command image and device-to-master status image are
  easier to validate than ad hoc per-field messages.
- Combined write/read exchange must define where output bytes and input bytes
  live in one payload. The current EtherCAT LRW behavior uses output bytes first
  and input bytes at the input-window offset.
- Discovery metadata should be available from the device, but it should describe
  LEAP concepts directly rather than emulating EtherCAT EEPROM contents.
- Runtime state changes must be explicit and bounded so stale masters cannot
  leave outputs active indefinitely.

## Document Map

- `LEAP_SPECIFICATION.md`: normative draft of the link-layer frame format,
  services, state machine, reliability model, diagnostics, and extension rules.
- `CLEARCORE_IO_PROFILE.md`: the first device profile, preserving the current
  ClearCore onboard I/O, CCIO, supply voltage, and timing process images.
- `MIGRATION_AND_ENHANCEMENT_PLAN.md`: extraction plan from the existing
  EtherCAT personality into LEAP firmware and master components.

## Design Status

Status: draft for engineering review.

The protocol is not yet implemented. EtherType assignment, security model,
conformance tests, and firmware/master cutover strategy must be reviewed before
LEAP is used outside isolated lab networks.

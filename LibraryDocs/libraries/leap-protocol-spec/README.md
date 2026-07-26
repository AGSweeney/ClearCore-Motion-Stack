---
title: LEAP protocol spec
component: leap-protocol-spec
level: library
reuse: medium
platforms:
  - design-only
topics:
  - LEAP
  - link layer
  - endpoints
  - discovery
source_paths:
  - docs/ClearCore_Link_Layer_Protocol/README.md
  - docs/ClearCore_Link_Layer_Protocol/LEAP_SPECIFICATION.md
  - docs/ClearCore_Link_Layer_Protocol/CLEARCORE_IO_PROFILE.md
status: verified
retrieval:
  questions:
    - What does LEAP stand for?
    - How does LEAP differ from the EtherCAT personality?
    - What is the ClearCoreFixedIoV1 profile?
  related:
    - libraries/ethercat-slave-transport/README.md
    - docs/ClearCore_Link_Layer_Protocol/MIGRATION_AND_ENHANCEMENT_PLAN.md
---

# LEAP protocol spec

**LEAP** = Layer2 Ethernet Automation Protocol. Draft design package extracted from the EtherCAT personality. **Reuse: medium** (spec-only; not implemented in firmware yet).

Artifact: [A-L07-data](../../artifacts/data/leap-frame-header.md).

## Purpose and reuse

Define project-owned raw Ethernet services: MGMT, DISC, DIR, PD, DIAG — without EtherCAT ESC/SII/WKC.

## Public API surface

Not applicable (no code). Spec services `LEAP-MGMT`, `LEAP-DISC`, `LEAP-DIR`, `LEAP-PD`, `LEAP-DIAG`.

## Initialization

Future firmware: session open + profile select before OP.

## Runtime lifecycle

Documented state machine BOOT→INIT→CONFIGURED→SAFE→OP/FAULT.

## Threading and ownership

Spec requires single owner lease for outputs.

## Configuration

Directory objects + `ClearCoreFixedIoV1` profile endpoints.

## Error handling

`LeapErrorPayload` status codes in specification.

## Memory and resources

Implementation TBD; fragmentation optional and non-cyclic.

## Timing and performance

Cyclic PD exchange; watchdog/lease timeouts required.

## Data formats / wire protocol

32-byte LEAP header; magic ASCII `LEAP`; EXCHANGE_ENDPOINTS output-then-input packing.

## Dependencies

Conceptual dependency on L06 lessons; no runtime link.

## Integration points

Migration plan proposes `LeapFirmware` / `LeapMaster` (not present).

## Failure modes

Lease expiry must force SAFE outputs (spec).

## Limits and constraints

Draft; EtherType assignment REVIEW; lab-only until implemented.

## Testing and validation

Spec review only today.

## Portability

Protocol is hardware-agnostic; first profile is ClearCore I/O specific.

## Security

Isolated machine network assumptions; auth is future work.

## Logging and diagnostics

LEAP-DIAG counters defined in spec.

## Extension points

Vendor service IDs 0x8000+; new profile IDs for incompatible layouts.

## Source evidence

| Claim | Evidence | Level |\n|-------|----------|-------|\n| LEAP expansion | `docs/.../README.md` | E1 |\n| Header magic LEAP | `LEAP_SPECIFICATION.md` frame table | E1 |\n| 40/64 profile | `CLEARCORE_IO_PROFILE.md` | E1 |


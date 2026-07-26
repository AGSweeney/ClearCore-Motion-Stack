---
title: ClearCore ClearLink bridge
component: clearcore-clearlink-bridge
level: library
reuse: high
platforms:
  - ClearCore-SAME53
topics:
  - ClearLink
  - assemblies
  - CCIO
  - board mode
  - motors
source_paths:
  - ProjectTemplate/ClearLinkCompatibilityFirmware/OpENer/source/src/ports/ClearCore/clearcore_clearlink_bridge.h
  - ProjectTemplate/ClearLinkCompatibilityFirmware/OpENer/source/src/ports/ClearCore/clearcore_clearlink_bridge.cpp
status: verified
retrieval:
  questions:
    - How does CIP I/O map onto ClearCore connectors?
    - How is board motor mode (Step/Dir vs M-connector) requested?
    - Is the bridge safe to overwrite from upstream OpENer?
  related:
    - libraries/libclearcore/README.md
    - libraries/opener-clearcore-port/README.md
    - docs/ASSEMBLY_LAYOUT.md
---

# ClearCore ClearLink bridge

Project-owned C API bridging ClearLink-style CIP assemblies to libClearCore hardware. **Reuse: high** for EIP adapter personalities. **Never overwrite from DX200/upstream OpENer sync.**

Artifacts: [A-L05-if](../../artifacts/interfaces/clearcore_clearlink_bridge.h), [A-L05-pat](../../artifacts/patterns/board-motor-mode.cpp).

## Purpose and reuse

Maps IO, analog, CCIO, encoder, and motor Step/Dir objects to ClearCore connectors.

## Public API surface

`ConnectorIO*_…`, `Ccio_*`, `Encoder_*`, `ClearLinkMotor_*`, `BoardMotorMode_Request`, voltage/ASCII helpers in the bridge header.

## Initialization

Called from OpENer application object init / first assembly apply.

## Runtime lifecycle

Polled/applied on CIP cyclic path with OpENer.

## Threading and ownership

Same context as `opener_cyclic` / assembly handlers.

## Configuration

Board mode via class 0x69 attribute semantics (see `docs/ASSEMBLY_LAYOUT.md`); CCIO enable on COM-1.

## Error handling

Fault/overload bits surfaced to assemblies; validate host inputs.

## Memory and resources

Static bridge state for four Step/Dir axes.

## Timing and performance

Must complete within CIP RPI budget.

## Data formats / wire protocol

Canonical ClearLink layouts: `ExternalReferances/clearlink_ethernet-ip_object_reference.pdf` overrides informal docs.

## Dependencies

L01 libClearCore.

## Integration points

P01 application `.c` assemblies; MotionBench object map (P04).

## Failure modes

| Symptom | Likely cause |\n|---------|--------------|\n| Motors wrong mode | BoardMotorMode_Request polarity vs host bit |\n| CCIO dead | COM-1 not opened / enable bit clear |

## Limits and constraints

ClearLink parity target; not a full ClearPath-IP clone.

## Testing and validation

MotionBench + PLC scanners.

## Portability

ClearCore-only.

## Security

Trust cell network; no session auth in bridge.

## Logging and diagnostics

Optional USB; status bits in assemblies.

## Extension points

Future NVRAM board mode (see AGENTS.md optional enhancements).

## Source evidence

| Claim | Evidence | Level |\n|-------|----------|-------|\n| Project-owned header notice | `clearcore_clearlink_bridge.h` L1–9 | E1 |\n| BoardMotorMode_Request | bridge `.h` / `.cpp` | E1 |\n| CCIO APIs | `Ccio_SetEnabled` in header | E1 |


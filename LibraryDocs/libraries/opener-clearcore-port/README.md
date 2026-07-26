---
title: OpENer ClearCore port
component: opener-clearcore-port
level: library
reuse: medium
platforms:
  - ClearCore-SAME53
topics:
  - OpENer
  - EtherNet/IP
  - opener_init
  - CIP
source_paths:
  - ProjectTemplate/ClearLinkCompatibilityFirmware/OpENer/source/src/ports/ClearCore/opener.h
  - ProjectTemplate/ClearLinkCompatibilityFirmware/OpENer/source/src/ports/ClearCore/opener.c
status: verified
retrieval:
  questions:
    - How do I start the OpENer adapter on ClearCore?
    - What does opener_cyclic do?
    - Where is network handling for CIP sockets?
  related:
    - libraries/clearcore-clearlink-bridge/README.md
    - project/subsystems/clearlink-compatibility-firmware/README.md
---

# OpENer ClearCore port

ClearCore port entrypoints for the OpENer EtherNet/IP adapter stack. **Reuse: medium** (coupled to OpENer tree layout).

Artifact: [A-L04-if](../../artifacts/interfaces/opener_port.h).

## Purpose and reuse

Exposes `opener_init`, `opener_cyclic`, `opener_shutdown`, `opener_get_status` for firmware mains.

## Public API surface

See `opener.h`. Network sockets in `networkhandler.c` / `networkconfig.*`.

## Initialization

`opener_init(struct netif *)` after Ethernet setup.

## Runtime lifecycle

Periodic `opener_cyclic()` from main; services explicit + implicit CIP.

## Threading and ownership

Main-loop owned; not multi-threaded.

## Configuration

`opener_user_conf.h`, `devicedata.h` identity.

## Error handling

`opener_get_status` / `g_end_stack` checked by main.

## Memory and resources

OpENer connection/session pools per user conf.

## Timing and performance

Cyclic call period typically ~1 ms with Ethernet refresh.

## Data formats / wire protocol

EtherNet/IP CIP encapsulation (canonical details in OpENer + ClearLink PDF).

## Dependencies

Upstream OpENer CIP core; LwIP sockets; L05 for I/O mapping.

## Integration points

P01 main; application objects under `clearlink_compatibility_firmware/`.

## Failure modes

| Symptom | Likely cause |\n|---------|--------------|\n| Init FAIL | netif null / stack end flag |\n| No scanner connect | identity/EDS mismatch |

## Limits and constraints

Adapter-only; not a scanner.

## Testing and validation

MotionBench CIP client (P04); PLC Forward Open testing.

## Portability

Port is ClearCore/LwIP specific.

## Security

Industrial cell network assumptions; no CIP security profile claimed.

## Logging and diagnostics

USB prints from main; OpENer traces if enabled.

## Extension points

Application CIP objects; do not overwrite project-owned bridge files when refreshing OpENer.

## Source evidence

| Claim | Evidence | Level |\n|-------|----------|-------|\n| opener_init API | `opener.h` | E1 |\n| Called from main | `ClearLinkCompatibilityFirmware/main.cpp` | E1 |\n| Port path | `ports/ClearCore/opener.c` | E1 |


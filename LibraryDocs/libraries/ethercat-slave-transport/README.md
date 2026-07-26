---
title: EtherCAT slave transport
component: ethercat-slave-transport
level: library
reuse: high
platforms:
  - ClearCore-SAME53
topics:
  - EtherCAT
  - PDO
  - LRW
  - 0x88A4
source_paths:
  - ProjectTemplate/EtherCATSlaveFirmware/protocol/ethercat_slave/ethercat_slave.h
  - ProjectTemplate/EtherCATSlaveFirmware/protocol/ethercat_slave/ethercat_slave.cpp
  - ProjectTemplate/EtherCATSlaveFirmware/protocol/ethercat_slave/ethercat_pdo_layout.h
status: verified
retrieval:
  questions:
    - What are the RxPDO and TxPDO sizes?
    - How does LRW pack outputs and inputs?
    - How is the transport separated from ClearCore I/O?
  related:
    - libraries/leap-protocol-spec/README.md
    - project/subsystems/ethercat-slave-firmware/README.md
    - docs/EtherCAT_PERSONALITY.md
---

# EtherCAT slave transport

Minimal non-conformant EtherCAT-frame transport with fixed PDO images. **Reuse: high** as an isolated transport module. Canonical behavior: `docs/EtherCAT_PERSONALITY.md`.

Artifacts: [A-L06-if](../../artifacts/interfaces/ethercat_pdo_layout.h), [A-L06-pat](../../artifacts/patterns/lrw-output-then-input.cpp).

## Purpose and reuse

Parse/respond to EtherType 0x88A4 frames; own command/status process images.

## Public API surface

`EthercatSlaveInit`, `EthercatSlaveCyclic`, `EthercatSlaveHandleFrame`, `CommandImage`, `StatusImage`, `Stats`; C hook for LwIP.

## Initialization

`EthercatSlaveInit(netif)` after Ethernet up.

## Runtime lifecycle

RX via LwIP hook; `EthercatSlaveCyclic` syncs status image; app fills StatusImage / reads CommandImage.

## Threading and ownership

Hook may run in RX path; keep handlers short.

## Configuration

Fixed SM2/SM3 windows and SII image generated in transport `.cpp`.

## Error handling

rx_drop/tx_drop stats; malformed frames rejected.

## Memory and resources

ESC register emulation buffer + 512 B process image.

## Timing and performance

Software ESC — not hardware DC/timestamp capable.

## Data formats / wire protocol

`EthercatPdoCommand` 40 B, `EthercatPdoStatus` 64 B. LRW: outputs first, inputs after output length.

## Dependencies

L03 LwIP hook; no OpENer.

## Integration points

P02 main applies I/O; P03 master encodes/decodes matching layout.

## Failure modes

| Symptom | Likely cause |\n|---------|--------------|\n| Status stuck at byte0 | LRW input written at wrong offset |\n| TwinCAT scan fail | Incomplete ESC/SII emulation |

## Limits and constraints

Not ETG-conformant; no mailbox/CoE/CiA-402.

## Testing and validation

Validated with companion SOEM/Qt master (see personality doc).

## Portability

ClearCore + LwIP hook pattern.

## Security

Lab/isolated networks.

## Logging and diagnostics

`EthercatSlaveStats` counters.

## Extension points

LEAP (L07) extracts these ideas into a non-EtherCAT suite.

## Source evidence

| Claim | Evidence | Level |\n|-------|----------|-------|\n| PDO sizes 40/64 | `ethercat_pdo_layout.h` static_assert | E1 |\n| Public API | `ethercat_slave.h` | E1 |\n| LRW output-then-input | `ethercat_slave.cpp` LRW branch | E1 |


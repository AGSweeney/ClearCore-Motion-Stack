---
title: LwIP ClearCore port
component: lwip-clearcore-port
level: library
reuse: high
platforms:
  - ClearCore-SAME53
topics:
  - LwIP
  - ethernetif
  - unknown EtherType
  - GMAC
source_paths:
  - ProjectTemplate/LwIP/LwIP/port/include/lwip_hooks.h
  - ProjectTemplate/LwIP/LwIP/port/ethercat_unknown_eth_hook.c
  - ProjectTemplate/LwIP/LwIP/port/include/lwipopts.h
status: verified
retrieval:
  questions:
    - How does ClearCore receive non-IP Ethernet frames?
    - Where is the unknown EtherType hook defined?
    - What owns the weak EtherCAT hook stub?
  related:
    - libraries/ethercat-slave-transport/README.md
    - libraries/libclearcore/README.md
---

# LwIP ClearCore port

Project LwIP port and hooks for ClearCore Ethernet. **Reuse: high** for any firmware using `EthernetMgr` + custom EtherTypes.

Artifacts: [A-L03-if](../../artifacts/interfaces/lwip_hooks.h), [A-L03-pat](../../artifacts/patterns/unknown-ethertype-hook.c).

## Purpose and reuse

Binds LwIP to ClearCore MAC and provides `LWIP_HOOK_UNKNOWN_ETH_PROTOCOL` integration.

## Public API surface

Standard LwIP APIs; port hook macros in `lwip_hooks.h`; weak `EthercatSlave_LwipUnknownEthProtocolHook` stub.

## Initialization

Via `EthernetMgr.Setup()` which brings up netif / ethernetif.

## Runtime lifecycle

Packet RX in driver/IRQ path; stack processed cooperatively with `EthernetMgr.Refresh`.

## Threading and ownership

No RTOS; careful with ISR vs main when touching pbufs.

## Configuration

`lwipopts.h` memory pools and features.

## Error handling

LwIP `err_t`; weak hook returns `ERR_VAL` when unused.

## Memory and resources

Configured pbuf/mempool sizes in `lwipopts.h`.

## Timing and performance

Refresh cadence set by firmware main loops (~1 ms typical).

## Data formats / wire protocol

Ethernet frames; unknown EtherTypes diverted to hook.

## Dependencies

Upstream LwIP under `LwIP/LwIP/src` (third-party).

## Integration points

L06 EtherCAT personality overrides the weak hook; OpENer uses TCP/UDP on same netif.

## Failure modes

| Symptom | Likely cause |\n|---------|--------------|\n| EtherCAT frames dropped | Hook not overridden / wrong lwip_hooks.h |\n| TCP fails | opts too small or link down |

## Limits and constraints

Single netif ClearCore MAC.

## Testing and validation

Both firmware personalities bring up Ethernet.

## Portability

Port files are ClearCore-specific.

## Security

No TLS in this port by default.

## Logging and diagnostics

Optional LwIP stats; firmware USB logs.

## Extension points

Additional EtherTypes via personality `lwip_hooks.h` overrides.

## Source evidence

| Claim | Evidence | Level |\n|-------|----------|-------|\n| Hook macro | `lwip_hooks.h` `LWIP_HOOK_UNKNOWN_ETH_PROTOCOL` | E1 |\n| Weak stub | `ethercat_unknown_eth_hook.c` | E1 |\n| Port options | `lwipopts.h` | E1 |


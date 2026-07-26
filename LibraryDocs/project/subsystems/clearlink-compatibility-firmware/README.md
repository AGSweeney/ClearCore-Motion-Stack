---
title: ClearLinkCompatibilityFirmware
component: clearlink-compatibility-firmware
level: project
topics:
  - EtherNet/IP
  - ClearLink
  - OpENer
  - firmware
source_paths:
  - ProjectTemplate/ClearLinkCompatibilityFirmware/main.cpp
  - ProjectTemplate/ClearLinkCompatibilityFirmware/OpENer/source/src/ports/ClearCore/clearlink_compatibility_firmware/clearlink_compatibility_firmware.c
status: verified
---

# ClearLinkCompatibilityFirmware

Production-style EtherNet/IP adapter personality targeting ClearLink assembly/object parity on ClearCore.

Couples to L04 OpENer port, L05 bridge, L01 libClearCore, L03 LwIP.

Artifact: [A-P01-pat](../../../artifacts/patterns/clearlink-firmware-main.cpp).

Canonical assembly authority: ClearLink object reference PDF (see `docs/ASSEMBLY_LAYOUT.md`).

## Source evidence

| Claim | Evidence | Level |
|-------|----------|-------|
| main calls opener_init | `main.cpp` | E1 |
| App objects path | `clearlink_compatibility_firmware.c` | E1 |

---
title: EtherCATSlaveFirmware
component: ethercat-slave-firmware
level: project
topics:
  - EtherCAT
  - CCIO
  - PDO
  - firmware
source_paths:
  - ProjectTemplate/EtherCATSlaveFirmware/main.cpp
  - ProjectTemplate/EtherCATSlaveFirmware/lwip_hooks.h
  - docs/EtherCAT_PERSONALITY.md
status: verified
---

# EtherCATSlaveFirmware

Non-conformant EtherCAT I/O personality: onboard IO/DI/A, CCIO-8, supply, timing diagnostics.

Transport: L06. Hardware apply: `main.cpp` only (layering rule).

Artifact: [A-P02-pat](../../../artifacts/patterns/ethercat-firmware-main.cpp).

## Source evidence

| Claim | Evidence | Level |
|-------|----------|-------|
| Personality documented | `docs/EtherCAT_PERSONALITY.md` | E1 |
| Hook override | `EtherCATSlaveFirmware/lwip_hooks.h` | E1 |

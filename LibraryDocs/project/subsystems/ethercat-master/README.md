---
title: EtherCATMaster
component: ethercat-master
level: project
topics:
  - SOEM
  - Qt
  - Npcap
  - master
source_paths:
  - EtherCATMaster/src/EthercatMasterThread.cpp
  - EtherCATMaster/build.ps1
  - EtherCATMaster/CMakeLists.txt
status: verified
---

# EtherCATMaster

Windows Qt + SOEM master utility for the ClearCore EtherCAT personality. Build via `build.ps1`; requires Npcap and Administrator for raw Ethernet.

Artifact: [A-P03-pat](../../../artifacts/patterns/ethercat-master-manual-io.cpp).

## Source evidence

| Claim | Evidence | Level |
|-------|----------|-------|
| SOEM cyclic thread | `EthercatMasterThread.cpp` | E1 |
| Standardized build | `build.ps1` | E1 |

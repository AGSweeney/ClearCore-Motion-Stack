---
title: Component inventory
level: project
status: verified
inventory_version: 1
repo_root: .
last_updated: 2026-07-26
---

# Component Inventory

## Summary

| Metric | Count |
|--------|------:|
| Libraries | 7 |
| Project subsystems | 4 |
| Platform modules | 2 |
| Artifacts required | 18 |
| Verified (E1/E2) | 13 |
| Inferred/draft | 0 |

## Inventory table

| ID | Name | Level | Folder | Source paths | Reuse | Owner task | Socket/storage | Artifact IDs | Doc status | Evidence |
|----|------|-------|--------|--------------|-------|------------|----------------|--------------|------------|----------|
| L01 | libClearCore | library | libraries/libclearcore | ProjectTemplate/libClearCore/inc/ClearCore.h, ProjectTemplate/libClearCore/inc/Connector.h, ProjectTemplate/libClearCore/src/MotorManager.cpp, ProjectTemplate/libClearCore/src/SysManager.cpp | high | FastUpdate ISR / SysTick | EthernetMgr, MotorMgr GPIO | A-L01-if, A-L01-pat | verified | E1 |
| L02 | CoordinatedMotion | library | libraries/coordinated-motion | ProjectTemplate/libClearCore/inc/CoordinatedMotionController.h, ProjectTemplate/libClearCore/src/CoordinatedMotionController.cpp, ProjectTemplate/libClearCore/inc/ArcInterpolator.h | high | FastUpdate ISR (step consume) | MotorDriver M0/M1 | A-L02-if, A-L02-pat | verified | E1 |
| L03 | LwIP ClearCore port | library | libraries/lwip-clearcore-port | ProjectTemplate/LwIP/LwIP/port/include/lwip_hooks.h, ProjectTemplate/LwIP/LwIP/port/ethercat_unknown_eth_hook.c, ProjectTemplate/LwIP/LwIP/port/include/lwipopts.h, ProjectTemplate/LwIP/LwIP/port/sys_arch.c | high | Ethernet RX / cooperative | GMAC / netif | A-L03-if, A-L03-pat | verified | E1 |
| L04 | OpENer ClearCore port | library | libraries/opener-clearcore-port | ProjectTemplate/ClearLinkCompatibilityFirmware/OpENer/source/src/ports/ClearCore/opener.h, ProjectTemplate/ClearLinkCompatibilityFirmware/OpENer/source/src/ports/ClearCore/opener.c, ProjectTemplate/ClearLinkCompatibilityFirmware/OpENer/source/src/ports/ClearCore/networkhandler.c | medium | main loop opener_cyclic | TCP/UDP CIP sockets | A-L04-if | verified | E1 |
| L05 | ClearCore ClearLink bridge | library | libraries/clearcore-clearlink-bridge | ProjectTemplate/ClearLinkCompatibilityFirmware/OpENer/source/src/ports/ClearCore/clearcore_clearlink_bridge.h, ProjectTemplate/ClearLinkCompatibilityFirmware/OpENer/source/src/ports/ClearCore/clearcore_clearlink_bridge.cpp | high | OpENer assembly callbacks | ClearCore connectors / CCIO COM-1 | A-L05-if, A-L05-pat | verified | E1 |
| L06 | EtherCAT slave transport | library | libraries/ethercat-slave-transport | ProjectTemplate/EtherCATSlaveFirmware/protocol/ethercat_slave/ethercat_slave.h, ProjectTemplate/EtherCATSlaveFirmware/protocol/ethercat_slave/ethercat_slave.cpp, ProjectTemplate/EtherCATSlaveFirmware/protocol/ethercat_slave/ethercat_pdo_layout.h | high | LwIP unknown-eth hook + main cyclic | EtherType 0x88A4 frames | A-L06-if, A-L06-pat | verified | E1 |
| L07 | LEAP protocol spec | library | libraries/leap-protocol-spec | docs/ClearCore_Link_Layer_Protocol/README.md, docs/ClearCore_Link_Layer_Protocol/LEAP_SPECIFICATION.md, docs/ClearCore_Link_Layer_Protocol/CLEARCORE_IO_PROFILE.md | medium | n/a | n/a (docs-only) | A-L07-data | verified | E1 |
| P01 | ClearLinkCompatibilityFirmware | project | project/subsystems/clearlink-compatibility-firmware | ProjectTemplate/ClearLinkCompatibilityFirmware/main.cpp, ProjectTemplate/ClearLinkCompatibilityFirmware/OpENer/source/src/ports/ClearCore/clearlink_compatibility_firmware/clearlink_compatibility_firmware.c, ProjectTemplate/ClearLinkCompatibilityFirmware/OpENer/source/src/ports/ClearCore/devicedata.h | app-only | main loop | EtherNet/IP assemblies | A-P01-pat | verified | E1 |
| P02 | EtherCATSlaveFirmware | project | project/subsystems/ethercat-slave-firmware | ProjectTemplate/EtherCATSlaveFirmware/main.cpp, ProjectTemplate/EtherCATSlaveFirmware/lwip_hooks.h, ProjectTemplate/EtherCATSlaveFirmware/esi/ClearCore_EtherCAT_Slave.xml | app-only | main ~1 ms loop | ClearCore I/O + CCIO | A-P02-pat | verified | E1 |
| P03 | EtherCATMaster | project | project/subsystems/ethercat-master | EtherCATMaster/src/EthercatMasterThread.cpp, EtherCATMaster/src/EthercatTypes.h, EtherCATMaster/CMakeLists.txt, EtherCATMaster/build.ps1 | app-only | EthercatMasterThread | Npcap / SOEM | A-P03-pat | verified | E1 |
| P04 | MotionBench | project | project/subsystems/motionbench | MotionBench/src/protocol/EtherNetIpClient.h, MotionBench/src/device/ClearLinkObjectMap.cpp, MotionBench/CMakeLists.txt | app-only | DeviceIoWorker / UI thread | CIP explicit messaging | A-P04-pat | verified | E1 |
| PL01 | Build / toolchain | platform | platform/build | ProjectTemplate/libClearCore/ClearCore.cppproj, EtherCATMaster/build.ps1, MotionBench/CMakeLists.txt | n/a | — | — | A-PL01-bld | verified | E1 |
| PL02 | Flash / deploy | platform | platform/build | ProjectTemplate/Tools/flash_clearcore.cmd, ProjectTemplate/Tools/README.md | n/a | — | USB bossac / UF2 | A-PL02-bld | verified | E1 |

### Artifact ID map

| ID | File |
|----|------|
| A-L01-if | interfaces/clearcore_umbrella.h |
| A-L01-pat | patterns/motor-mode-step-dir.cpp |
| A-L02-if | interfaces/coordinated_motion_controller.hpp |
| A-L02-pat | patterns/coordinated-fast-path.cpp |
| A-L03-if | interfaces/lwip_hooks.h |
| A-L03-pat | patterns/unknown-ethertype-hook.c |
| A-L04-if | interfaces/opener_port.h |
| A-L05-if | interfaces/clearcore_clearlink_bridge.h |
| A-L05-pat | patterns/board-motor-mode.cpp |
| A-L06-if | interfaces/ethercat_pdo_layout.h |
| A-L06-pat | patterns/lrw-output-then-input.cpp |
| A-L07-data | data/leap-frame-header.md |
| A-P01-pat | patterns/clearlink-firmware-main.cpp |
| A-P02-pat | patterns/ethercat-firmware-main.cpp |
| A-P03-pat | patterns/ethercat-master-manual-io.cpp |
| A-P04-pat | patterns/motionbench-object-map.cpp |
| A-PL01-bld | build/ethercat-master-build.ps1 |
| A-PL02-bld | build/flash_clearcore.cmd |

## Excluded (grouped under parent)

| Symbol/file | Parent ID | Reason |
|-------------|-----------|--------|
| OpENer cip/* upstream | L04 | Third-party CIP core; document via port surface |
| LwIP src/core/* upstream | L03 | Upstream stack; port owns ClearCore integration |
| Device_Startup/* | P02 | Board startup boilerplate |
| ExternalReferances/** | — | Reference baseline only |

## Coupling register

| From ID | To ID | Coupling type | Notes |
|---------|-------|---------------|-------|
| P01 | L04 | calls API | `opener_init` / `opener_cyclic` from main |
| P01 | L05 | calls API | Assemblies invoke ClearLink bridge |
| P01 | L01 | calls API | via bridge and EthernetMgr |
| P01 | L03 | include-only | LwIP netif from EthernetMgr |
| P02 | L06 | calls API | EtherCAT transport cyclic + hook |
| P02 | L01 | calls API | I/O and CCIO in main.cpp |
| P02 | L03 | calls API | unknown EtherType hook |
| P03 | L06 | config blob | PDO sizes/offsets match L06 layout (host-side) |
| P04 | P01 | calls API | CIP client targets ClearLink-compat adapter |
| L05 | L01 | calls API | Bridge wraps libClearCore connectors |
| L06 | L03 | calls API | Frame RX via LwIP hook |
| L02 | L01 | calls API | Uses MotorDriver / StepGenerator |
| L07 | L06 | include-only | Spec extracted from EtherCAT personality |

## Retrieval keywords

| ID | keywords |
|----|----------|
| L01 | ClearCore, MotorDriver, StepGenerator, Connector, EthernetMgr, SysManager |
| L02 | coordinated motion, arc, linear interpolator, MoveArc, queue |
| L03 | LwIP, ethernetif, unknown EtherType, lwip_hooks, GMAC |
| L04 | OpENer, opener_init, opener_cyclic, EtherNet/IP adapter |
| L05 | ClearLink bridge, CCIO, BoardMotorMode, assemblies, HLFB |
| L06 | EtherCAT, 0x88A4, PDO, LRW, EthercatPdoCommand, EthercatPdoStatus |
| L07 | LEAP, Layer2 Ethernet Automation Protocol, discovery, endpoints |
| P01 | ClearLinkCompatibilityFirmware, main loop, OpENer bring-up |
| P02 | EtherCATSlaveFirmware, CCIO, IO0-IO5, personality |
| P03 | EtherCATMaster, SOEM, Npcap, Qt master |
| P04 | MotionBench, CIP client, ClearLink object map, board mode |
| PL01 | Microchip Studio, CMake, Qt, build.ps1 |
| PL02 | flash_clearcore, bossac, UF2 |

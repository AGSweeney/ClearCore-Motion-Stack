# LibraryDocs INDEX

| Path | ID | Level | Component | Purpose | Topics | Status |
|------|----|-------|-----------|---------|--------|--------|
| libraries/libclearcore/README.md | L01 | library | libclearcore | ClearCore BSP/HAL and motor connectors | motors, connectors, EthernetMgr, SysManager | verified |
| libraries/coordinated-motion/README.md | L02 | library | coordinated-motion | Dual-axis coordinated arc/linear motion | arc, interpolator, queue, FastUpdate | verified |
| libraries/lwip-clearcore-port/README.md | L03 | library | lwip-clearcore-port | ClearCore LwIP port and EtherType hook | LwIP, GMAC, unknown EtherType | verified |
| libraries/opener-clearcore-port/README.md | L04 | library | opener-clearcore-port | OpENer adapter port entrypoints | OpENer, CIP, opener_cyclic | verified |
| libraries/clearcore-clearlink-bridge/README.md | L05 | library | clearcore-clearlink-bridge | CIP assembly to ClearCore hardware bridge | ClearLink, CCIO, board mode | verified |
| libraries/ethercat-slave-transport/README.md | L06 | library | ethercat-slave-transport | Non-conformant EtherCAT frame transport | EtherCAT, PDO, LRW, 0x88A4 | verified |
| libraries/leap-protocol-spec/README.md | L07 | library | leap-protocol-spec | Draft LEAP link-layer protocol suite | LEAP, endpoints, discovery | verified |
| project/subsystems/clearlink-compatibility-firmware/README.md | P01 | project | clearlink-compatibility-firmware | EtherNet/IP ClearLink-compat adapter firmware | OpENer, main, assemblies | verified |
| project/subsystems/ethercat-slave-firmware/README.md | P02 | project | ethercat-slave-firmware | EtherCAT I/O personality firmware | CCIO, PDO map, main loop | verified |
| project/subsystems/ethercat-master/README.md | P03 | project | ethercat-master | Windows SOEM/Qt EtherCAT master | SOEM, Npcap, Qt | verified |
| project/subsystems/motionbench/README.md | P04 | project | motionbench | Host CIP bench for ClearLink objects | CIP, board mode, Qt | verified |
| platform/build/build-instructions.md | PL01 | platform | build | Toolchain and build entrypoints | Microchip Studio, CMake, Qt | verified |
| platform/build/flash-deploy.md | PL02 | platform | flash-deploy | ClearCore flash via bossac/UF2 | flash, bossac, UF2 | verified |
| project/architecture/system-overview.md | — | project | architecture | Startup order and data flows | architecture, personalities | verified |
| project/recipes/flash-clearlink-firmware.md | — | project | recipes | Flash ClearLink-compat firmware | flash, recipe | verified |

# Artifacts registry

| ID | File | Component | Usefulness | Description |
|----|------|-----------|------------|-------------|
| A-L01-if | [interfaces/clearcore_umbrella.h](interfaces/clearcore_umbrella.h) | L01 | U1–U6 | ClearCore umbrella exports |
| A-L01-pat | [patterns/motor-mode-step-dir.cpp](patterns/motor-mode-step-dir.cpp) | L01 | U2 | MotorModeSet step/dir mux |
| A-L02-if | [interfaces/coordinated_motion_controller.hpp](interfaces/coordinated_motion_controller.hpp) | L02 | U1–U6 | CoordinatedMotion public API |
| A-L02-pat | [patterns/coordinated-fast-path.cpp](patterns/coordinated-fast-path.cpp) | L02 | U2 | Coordinated step path notes |
| A-L03-if | [interfaces/lwip_hooks.h](interfaces/lwip_hooks.h) | L03 | U1–U6 | Unknown EtherType hook macro |
| A-L03-pat | [patterns/unknown-ethertype-hook.c](patterns/unknown-ethertype-hook.c) | L03 | U2 | Weak hook stub |
| A-L04-if | [interfaces/opener_port.h](interfaces/opener_port.h) | L04 | U1–U6 | opener_init/cyclic API |
| A-L05-if | [interfaces/clearcore_clearlink_bridge.h](interfaces/clearcore_clearlink_bridge.h) | L05 | U1–U6 | Bridge C API excerpt |
| A-L05-pat | [patterns/board-motor-mode.cpp](patterns/board-motor-mode.cpp) | L05 | U2 | BoardMotorMode_Request |
| A-L06-if | [interfaces/ethercat_pdo_layout.h](interfaces/ethercat_pdo_layout.h) | L06 | U1–U6 | Fixed PDO structs |
| A-L06-pat | [patterns/lrw-output-then-input.cpp](patterns/lrw-output-then-input.cpp) | L06 | U2 | LRW packing rule |
| A-L07-data | [data/leap-frame-header.md](data/leap-frame-header.md) | L07 | U1–U6 | LEAP header fields |
| A-P01-pat | [patterns/clearlink-firmware-main.cpp](patterns/clearlink-firmware-main.cpp) | P01 | U2 | OpENer bring-up |
| A-P02-pat | [patterns/ethercat-firmware-main.cpp](patterns/ethercat-firmware-main.cpp) | P02 | U2 | EtherCAT main includes |
| A-P03-pat | [patterns/ethercat-master-manual-io.cpp](patterns/ethercat-master-manual-io.cpp) | P03 | U2 | Manual LWR/LRD fallback |
| A-P04-pat | [patterns/motionbench-object-map.cpp](patterns/motionbench-object-map.cpp) | P04 | U2 | board.mode object map |
| A-PL01-bld | [build/ethercat-master-build.ps1](build/ethercat-master-build.ps1) | PL01 | U1–U6 | build.ps1 excerpt |
| A-PL02-bld | [build/flash_clearcore.cmd](build/flash_clearcore.cmd) | PL02 | U1–U6 | Flash entrypoint |

Bench folder reserved for future E2 logs (`artifacts/bench/`).

# Platform

## Detected stack

| Layer | Inference | Evidence |
|-------|-----------|----------|
| Language | C/C++ firmware; Qt C++/QML hosts | `.cppproj`, `CMakeLists.txt` |
| MCU | ClearCore SAME53 | libClearCore Device_Startup / DFP usage |
| Build | Microchip Studio + CMake/VS2022 | ProjectTemplate + EtherCATMaster/MotionBench |
| Networking | LwIP + OpENer + raw EtherType | port hooks, opener, ethercat_slave |
| Host capture | Npcap (EtherCATMaster) | `build.ps1` copies wpcap |

| Doc | ID |
|-----|----|
| [build/build-instructions.md](build/build-instructions.md) | PL01 |
| [build/flash-deploy.md](build/flash-deploy.md) | PL02 |

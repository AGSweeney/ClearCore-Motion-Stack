---
title: Build instructions
component: build
level: platform
topics:
  - Microchip Studio
  - CMake
  - Qt
  - toolchain
source_paths:
  - EtherCATMaster/build.ps1
  - MotionBench/CMakeLists.txt
  - ProjectTemplate/libClearCore/ClearCore.cppproj
status: verified
---

# Build instructions

## Firmware (ClearCore)

- Open `ProjectTemplate/*/*.atsln` / `.cppproj` in Microchip Studio.
- Link against `libClearCore` and `LwIP` projects.
- Toolchain: ARM GCC as configured by the `.cppproj`.

## EtherCATMaster

```powershell
cd EtherCATMaster
.\build.ps1 -Configuration Release
```

Discovers VS 2022, CMake, and `windeployqt`; copies Npcap DLLs to `out/bin`.

## MotionBench

Use `MotionBench/CMakeLists.txt` / `build_vs.cmd` with Qt 6 + MSVC.

Artifact: [A-PL01-bld](../../artifacts/build/ethercat-master-build.ps1).

## Source evidence

| Claim | Evidence | Level |
|-------|----------|-------|
| Host build script | `EtherCATMaster/build.ps1` | E1 |
| libClearCore project | `ClearCore.cppproj` | E1 |

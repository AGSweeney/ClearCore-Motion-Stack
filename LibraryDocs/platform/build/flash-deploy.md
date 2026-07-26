---
title: Flash and deploy
component: flash-deploy
level: platform
topics:
  - flash
  - bossac
  - UF2
  - ClearCore
source_paths:
  - ProjectTemplate/Tools/flash_clearcore.cmd
  - ProjectTemplate/Tools/README.md
status: verified
---

# Flash and deploy

Firmware flashing entrypoint: `ProjectTemplate/Tools/flash_clearcore.cmd` (bossac-based; may forward to EnhancedClearCoreLibrary tools).

UF2 builder tooling also lives under `ProjectTemplate/Tools/uf2-builder/`.

Host apps: copy/deploy via `windeployqt` from respective `build.ps1` / CMake install rules. EtherCATMaster needs Administrator + Npcap.

Artifact: [A-PL02-bld](../../artifacts/build/flash_clearcore.cmd).

## Source evidence

| Claim | Evidence | Level |
|-------|----------|-------|
| Flash script exists | `ProjectTemplate/Tools/flash_clearcore.cmd` | E1 |
| Tools README | `ProjectTemplate/Tools/README.md` | E1 |

---
title: Flash ClearLink compatibility firmware
component: recipes
level: project
topics:
  - flash
  - ClearLink
  - bossac
status: verified
---

# Flash ClearLink compatibility firmware

1. Build `ProjectTemplate/ClearLinkCompatibilityFirmware` in Microchip Studio (Release).
2. Put ClearCore in bootloader / ensure USB programming port is available.
3. Run `ProjectTemplate/Tools/flash_clearcore.cmd` with the produced binary (or use the `.cppproj` post-build flash target).
4. Confirm USB banner: `ClearLink Compatibility Firmware -- ClearCore`.
5. Verify Ethernet link and OpENer init SUCCESS on serial.

See [../../platform/build/flash-deploy.md](../../platform/build/flash-deploy.md).

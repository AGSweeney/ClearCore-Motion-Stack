# Project Template

Microchip Studio–oriented build layout for ClearCore firmware that pairs **libClearCore**, **LwIP**, and an **OpENer**-based EtherNet/IP adapter port. This folder is the working template used by active development in ClearCore Motion Stack.

Copyright (c) 2026 Adam G. Sweeney `<agsweeney@gmail.com>`  
SPDX-License-Identifier: MIT

## Testing status and production use

This tree reflects the **current state of in-progress testing** for the firmware and related integration code. It is **not fully validated** and **will contain defects**; behavior can change as issues are found and fixed.

The ClearLink compatibility firmware **can command motion and I/O** (on-board I/O and **CCIO** expansion). Treat any connected mechanics, tooling, and personnel accordingly.

**Do not deploy this firmware in a production environment.** Use it only in controlled lab, bench, or development setups where unexpected motion, I/O, or network behavior is acceptable and mitigated.

## Contents

| Path | Role |
|------|------|
| [`ClearLinkCompatibilityFirmware/`](ClearLinkCompatibilityFirmware/) | Executable firmware: OpENer adapter with ClearLink-style I/O and object/assembly parity goals |
| [`libClearCore/`](libClearCore/) | Board and peripheral library (linked by the firmware solution) |
| [`LwIP/`](LwIP/) | TCP/IP stack used by libClearCore Ethernet and OpENer |
| [`Tools/`](Tools/) | Windows flash scripts and related utilities (see [`Tools/README.md`](Tools/README.md)) |

Open the firmware solution in Microchip Studio:

- [`ClearLinkCompatibilityFirmware/ClearLinkCompatibilityFirmware.atsln`](ClearLinkCompatibilityFirmware/ClearLinkCompatibilityFirmware.atsln)

---

## ClearLink compatibility firmware (current state)

**ClearLink compatibility firmware** is the template’s primary application. It targets a Teknic ClearCore running as an **EtherNet/IP adapter** whose produced assemblies, shared I/O segments, and much of the visible object model are intended to stay aligned with **ClearLink**-style integrations so existing scanners, EDS usage, and tooling can be exercised without treating this repo as an official Teknic product.

### What is implemented today

- **Bring-up and runtime loop** are intentionally minimal: wait for Ethernet link, initialize the MAC/LwIP path, call `opener_init()`, then run a tight loop with `EthernetMgr.Refresh()` and `opener_cyclic()` (about 1 ms cadence) plus light status output on USB and link-transition logging. See [`ClearLinkCompatibilityFirmware/main.cpp`](ClearLinkCompatibilityFirmware/main.cpp).
- **OpENer** is carried under the firmware project (`ClearLinkCompatibilityFirmware/OpENer/`) with a **ClearCore port** and an application subtree under `OpENer/source/src/ports/ClearCore/clearlink_compatibility_firmware/`.
- **ClearLink I/O parity** into libClearCore is implemented in the **project-owned** bridge [`clearcore_clearlink_bridge.cpp`](ClearLinkCompatibilityFirmware/OpENer/source/src/ports/ClearCore/clearcore_clearlink_bridge.cpp) / [`.h`](ClearLinkCompatibilityFirmware/OpENer/source/src/ports/ClearCore/clearcore_clearlink_bridge.h). These files are **not** generic OpENer upstream content; they encode how assembly data maps to connectors, motors, and related signals on ClearCore hardware.
- **Assembly and object layout** used for parity work are summarized in repository docs; the canonical reference when anything disagrees is the ClearLink EtherNet/IP object PDF in tree:

  - Summary: [`../docs/ASSEMBLY_LAYOUT.md`](../docs/ASSEMBLY_LAYOUT.md)  
  - Authority: [`../ExternalReferances/clearlink_ethernet-ip_object_reference.pdf`](../ExternalReferances/clearlink_ethernet-ip_object_reference.pdf)

### What is still evolving

- Object models, assembly behavior, explicit messaging paths, and adapter edge cases continue to be refined as interoperability testing finds gaps versus the PDF and real scanners.
- Higher-level **coordinated motion** and trajectory features live at the motion-stack / library research layer; this firmware template stays focused on **reliable adapter behavior** and **ClearLink-style I/O parity** unless a change is deliberately scoped otherwise.

### Porting and maintenance notes

When refreshing OpENer or baseline port files from other trees, treat the following as **intentional, review-before-overwrite** project surfaces (among others noted in repository `AGENTS.md`):

- `devicedata.h` and the ClearCore application tree under `clearlink_compatibility_firmware/`
- `clearcore_clearlink_bridge.{h,cpp}` (never treated as generic upstream)
- Firmware `main.cpp` for this template (kept narrow: Ethernet, OpENer init/cyclic, not unrelated peripheral bring-up)

`clearcore_wrapper.*` may follow a DX200-like baseline elsewhere in the stack; merge or refresh those deliberately rather than bulk-copying.

### Flashing

The firmware `.cppproj` is wired so the custom programming tool runs:

`$(MSBuildProjectDirectory)\..\Tools\flash_clearcore.cmd`

with the built `.bin` from the active configuration. See [`Tools/README.md`](Tools/README.md) and `flash_clearcore.cmd` for bossac-based USB flashing on Windows.

### Toolchain

The project expects Microchip Studio with device packs consistent with the `.cppproj` (for example **SAME53_DFP** and **CMSIS** paths as declared in the project file). Align pack versions with your studio installation or Teknic’s published ClearCore development guidance when builds fail on missing includes.

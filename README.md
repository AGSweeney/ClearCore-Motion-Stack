# ClearCore Motion Stack

Open EtherNet/IP motion firmware for ClearCore, focused on practical ClearLink migration and advanced coordinated motion.

## Overview

ClearCore Motion Stack extends Teknic ClearCore with a flexible, production-oriented EtherNet/IP motion platform.

The project is built around two operating modes:

- **ClearLink Compatibility Mode** - Enables migration from legacy ClearLink systems with minimal PLC and scanner-side changes.
- **Coordinated Motion Mode** - Adds multi-axis capabilities beyond ClearLink, including synchronized and trajectory-based behavior.

## Technical Basis

The compatibility layer in this project is based on the ClearLink EtherNet/IP object model documented in:

- ClearLink EtherNet/IP Setup and Object Data Reference, Rev. 1.15 (February 20, 2025)

The enhanced firmware foundation is based on the Enhanced ClearCore Library:

- <https://github.com/AGSweeney/EnhancedClearCoreLibrary>

## Why This Project

Teknic lists ClearLink as "not recommended for new designs" on its downloads page. At the time of writing, this lifecycle signal is not yet consistently reflected across all product-marketing pages, while ClearLink has been removed from primary navigation. This project provides a migration path for existing machines while creating an open motion foundation for new development.

Taken together, these signals are consistent with ClearPath-IP being positioned as the successor path for new EtherNet/IP motion designs.

- Reference: <https://teknic.com/downloads/>

## Successor Platform Compatibility and Integration Complexity

Based on Teknic's ClearPath-IP documentation, migration to the likely successor platform must account for integration complexity beyond simple command/status exchange.

### Key Observations from Teknic Documentation

- The **I/O HUB serves as the EtherNet/IP network interface** for ClearPath-IP systems.
- Teknic recommends controllers that support **implicit EtherNet/IP communication** and accept **EDS files**.
- For generic controllers, Teknic's integration flow includes:
  - Adding an I/O HUB device
  - Loading the correct model-specific EDS
  - Extracting/manually mapping assemblies where needed
  - Recreating Data Exchange / AOI-style logic in non-Logix environments
- Multiple I/O HUB variants use different assembly instance IDs and payload sizes, increasing controller mapping and validation effort.
- PLCs that do not support EDS or equivalent tooling can require substantial manual implementation work.

These factors can increase project risk, integration time, and maintenance cost across mixed PLC ecosystems. ClearCore Motion Stack is intended to reduce that burden through a clearer compatibility layer and open, portable motion architecture.

- Reference: Teknic "ClearPath-IP Software Reference" (Rev 1.1, Nov 20, 2025)

## Project Goals

### 1) Seamless ClearLink Migration

- Provide a practical replacement path for ClearLink-based systems.
- Minimize required changes to existing PLC programs and EtherNet/IP scanners.
- Mirror expected command, status, and motion behavior where feasible.
- Support established machine architectures and wiring approaches.

### 2) Open EtherNet/IP Motion Platform

- Build on OpENer for adapter functionality.
- Define clear, versioned CIP object models and assemblies.
- Enable deterministic cyclic I/O for motion control.
- Support explicit messaging for configuration and diagnostics.

### 3) Expanded Motion Capability

- Add coordinated multi-axis motion support.
- Provide buffered and blended motion execution.
- Enable higher-level motion constructs beyond point-to-point control.
- Support future extensions (for example: camming, gearing, path execution).

### 4) Hardware Abstraction

- Decouple motion logic from underlying hardware.
- Provide a hardware abstraction layer (HAL) for ClearCore and future targets.
- Enable portability and long-term maintainability.

### 5) Production-Ready Reliability

- Deterministic state machines for motion and fault handling.
- Robust watchdog and connection-loss behavior.
- Persistent configuration and safe startup states.
- Testable, verifiable behavior through automated validation tools.

### 6) Developer-Friendly Architecture

- Clean separation between motion logic, CIP interface, and hardware abstraction.
- Modular design for long-term feature growth.
- Tooling support for EDS generation, configuration, and validation.

## Non-Goals

- This project is not an official Teknic product.
- Exact parity with ClearLink in every edge case is not guaranteed.
- This project is not intended to replace full PLC or CNC control systems.

## Status

Early development. Core EtherNet/IP adapter and motion control components are functional on ClearCore hardware. Compatibility layers and coordinated motion features remain under active development.

## Contributing

Contributions, testing, and feedback are welcome. If you are migrating from ClearLink or deploying new systems on ClearCore, practical field feedback is especially valuable.

## Project Rules

1. **MIT license where applicable** - Original project code should use MIT licensing where appropriate.
2. **Attribution to OpENer where applicable** - OpENer-derived concepts or code should be clearly attributed.
3. **Copyright and contributor notices for original work** - Include:
   - Adam G. Sweeney `<agsweeney@gmail.com>`

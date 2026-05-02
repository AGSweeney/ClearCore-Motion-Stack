# ClearCore Motion Stack

Open EtherNet/IP motion firmware for ClearCore, featuring ClearLink-compatible operation and advanced coordinated motion.

## Overview

ClearCore Motion Stack extends the capabilities of the Teknic ClearCore to provide a flexible, production-ready motion control platform over EtherNet/IP.

This project has two primary modes of operation:

- **ClearLink Compatibility Mode** - Enables migration from legacy Teknic ClearLink motion controller systems with minimal changes to existing PLC or application logic.
- **Coordinated Motion Mode** - Provides enhanced multi-axis motion capabilities beyond ClearLink, including trajectory planning and synchronized motion.

## Project Goals

### 1. Seamless ClearLink Migration

- Provide a functional, practical replacement for ClearLink-based systems.
- Minimize required changes to existing PLC programs and EtherNet/IP scanners.
- Mirror expected command, status, and motion behaviors where feasible.
- Support existing machine architectures and wiring approaches.

### 2. Open EtherNet/IP Motion Platform

- Build on OpENer for adapter functionality.
- Define clear, versioned CIP object models and assemblies.
- Enable deterministic cyclic I/O for motion control.
- Support explicit messaging for configuration and diagnostics.

### 3. Expand Motion Capability

- Add coordinated multi-axis motion support.
- Provide buffered and blended motion execution.
- Enable higher-level motion constructs beyond point-to-point control.
- Support future extensions (camming, gearing, path execution).

### 4. Hardware Abstraction

- Decouple motion logic from underlying hardware.
- Provide a hardware abstraction layer (HAL) for ClearCore and future targets.
- Enable portability and long-term maintainability.

### 5. Production-Ready Reliability

- Deterministic state machines for motion and fault handling.
- Robust watchdog and connection-loss behavior.
- Persistent configuration and safe startup states.
- Testable and verifiable behavior through automated validation tools.

### 6. Developer-Friendly Architecture

- Clean separation between:
  - Motion control logic
  - EtherNet/IP interface (CIP)
  - Hardware abstraction
- Modular design to support feature growth.
- Tooling for EDS generation, configuration, and testing.

## Non-Goals

- This project is not an official product of Teknic.
- Exact behavioral parity with ClearLink in all edge cases is not guaranteed.
- This is not intended to replace full PLC or CNC control systems.

## Status

Early development. Core components (EtherNet/IP adapter and motion control) are functional on ClearCore hardware. Compatibility layer and coordinated motion features are under active development.

## Contributing

Contributions, testing, and feedback are welcome.
If you are migrating from ClearLink or building new systems on ClearCore, your input is especially valuable.

## Project Rules

1. **MIT license where applicable** - Project code should be released under the MIT License where appropriate.
2. **Attribution to OpENer where applicable** - Any use of OpENer-derived concepts or code should include proper attribution.
3. **Copyright and contributor notices for original work** - Original project work should include:
   - Adam G. Sweeney `<agsweeney@gmail.com>`

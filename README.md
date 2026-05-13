# ClearCore Motion Stack

Experimental coordinated motion and EtherNet/IP motion research platform for Teknic ClearCore hardware.

ClearCore Motion Stack explores advanced multi-axis motion control, industrial interoperability, and lightweight EtherNet/IP motion architectures using the Teknic ClearCore ecosystem.

The project focuses primarily on:
- Coordinated multi-axis motion research
- Motion planning and trajectory execution
- EtherNet/IP motion experimentation
- Industrial controller interoperability
- PLC and robotics integration
- Extensions to the [Enhanced ClearCore Library](https://github.com/AGSweeney/EnhancedClearCoreLibrary)

Additional interoperability modes are also being explored to simplify integration continuity for existing ClearLink-based machine architectures where appropriate.

---

# Project Status

This repository is currently in active research and development.

At this stage the repository primarily contains:
- Architecture planning
- Interface concepts
- Motion system design notes
- Integration research
- Development direction documentation

Firmware implementations, motion planners, and protocol layers are still evolving and subject to substantial change.

---

# Important Notice

ClearCore Motion Stack is an independent third-party project and is not affiliated with, endorsed by, or supported by Teknic Inc.

Teknic, ClearCore, ClearLink, ClearPath, and ClearPath-IP are trademarks or registered trademarks of Teknic Inc.

This repository exists for engineering research, interoperability experimentation, and industrial motion development.

---

# Project Goals

The primary goal of this project is to explore what is possible when extending the ClearCore ecosystem beyond traditional single-axis motion applications.

Areas of investigation include:
- Coordinated linear and circular interpolation
- Buffered trajectory execution
- Motion queueing systems
- Streaming motion architectures
- External motion synchronization
- EtherNet/IP motion transport
- PLC-integrated motion control
- Industrial robotics interoperability
- Lightweight industrial motion systems
- CNC-style motion experimentation

The project is intended as a research and experimentation platform rather than a commercial motion controller product.

---

# Coordinated Motion Research

A major focus of this repository is coordinated multi-axis motion functionality built around extensions to the [Enhanced ClearCore Library](https://github.com/AGSweeney/EnhancedClearCoreLibrary).

Current areas of investigation include:
- Multi-axis interpolation
- Linear path blending
- Circular interpolation
- Motion buffering
- Continuous trajectory execution
- External controller command streaming
- Real-time motion synchronization
- Deterministic EtherNet/IP motion behaviors

The long-term objective is to investigate practical industrial motion architectures that remain lightweight, flexible, and highly interoperable.

---

# Industrial Integration Focus

This project places significant emphasis on interoperability with industrial automation systems including:
- PLC platforms
- Industrial robots
- EtherNet/IP scanners
- Embedded machine controllers
- Supervisory software systems
- Custom HMI applications

Particular attention is being given to environments where:
- EDS support may be limited
- EtherNet/IP assembly sizes matter
- Lightweight integrations are preferred
- Existing production systems cannot be substantially redesigned
- Flexible motion transport mechanisms are needed

---

# Legacy Integration Compatibility

In addition to coordinated motion research, this project also explores interoperability modes designed to simplify integration continuity for certain existing ClearLink-based machine architectures.

These interoperability concepts are intended to assist with:
- Existing EtherNet/IP machine integrations
- Established PLC mappings
- Legacy machine support
- Experimental migration strategies
- Development and testing environments

This project is not intended to replicate, replace, or emulate official Teknic products.

---

# Relationship to Teknic Products

Teknic's newer EtherNet/IP platform, ClearPath-IP, offers several advantages including:
- Full servo communication and diagnostics
- Simplified EtherNet/IP setup
- IO-Hub-based architecture
- Expanded software tooling
- Rockwell AOIs
- Enhanced drive and fault information

For many new machine designs, ClearPath-IP is likely the preferred long-term EtherNet/IP platform.

Teknic continues to manufacture and support both ClearCore and ClearLink hardware platforms.

This repository should not be interpreted as an official successor platform or replacement product for any Teknic offering.

For Teknic's own product positioning and specifications in adjacent EtherNet/IP and IO-centric motion spaces, refer to their pages for [ClearPath-IP (EtherNet/IP servo)](https://teknic.com/products/clearpath-brushless-dc-servo-motors/ethernet-ip-servo/), the [IO Motion Controller](https://teknic.com/products/io-motion-controller/), and the [EtherNet/IP IO Motion Controller](https://teknic.com/products/ethernet-ip-io-motion-controller/).

---

# Hardware Focus

Current project direction primarily targets:
- Teknic ClearCore
- ClearPath SD Series servos
- ClearPath MC Series servos

The project currently does not target ClearPath-IP, IO-Hub, or the related IO motion controller platforms as in-scope development targets for this repository. Use Teknic's official product pages (see the **Relationship to Teknic Products** section above) when evaluating those systems.

---

# Potential Use Cases

Areas being explored include:
- Experimental coordinated motion systems
- Robotics tooling and positioning
- PLC-integrated motion control
- Industrial automation research
- EtherNet/IP interoperability testing
- CNC-style motion experimentation
- Lightweight machine control architectures
- Development and educational platforms

---

# Current Development Areas

Planned development and research areas currently include:
- EtherNet/IP adapter behavior
- Motion abstraction layers
- Coordinated motion planning
- Buffered trajectory systems
- Lightweight communication architectures
- Industrial controller interoperability
- Configuration tooling
- Diagnostics and monitoring systems
- Motion visualization utilities

---

# Related Projects

## Enhanced ClearCore Library

Independent experimental extensions to the ClearCore ecosystem focused on advanced motion functionality and industrial control experimentation: [AGSweeney/EnhancedClearCoreLibrary](https://github.com/AGSweeney/EnhancedClearCoreLibrary) on GitHub.

## MotionBench Utility

Experimental desktop utility in this repository for EtherNet/IP motion testing, diagnostics, and configuration research. Start with [`MotionBench/README.md`](MotionBench/README.md).

## OpENer

EtherNet/IP adapter-related firmware work in this repository builds on the community [OpENer](https://github.com/EIPStackGroup/OpENer) project (open-source EtherNet/IP communication stack). Use that repository for upstream sources, licensing, and contribution workflow.

---

# Example Integration Projects

These sibling repositories illustrate applied industrial EtherNet/IP interoperability and motion-oriented integration patterns related to the research themes in this stack. They are maintained separately from ClearCore Motion Stack.

## Motoman EtherNet/IP ToolChanger

Industrial EtherNet/IP interoperability research involving:

- Yaskawa/Motoman robot integration
- EtherNet/IP motion coordination
- Lightweight industrial communication
- External motion device synchronization

Source: [AGSweeney/Motoman_EnIP_ToolChanger](https://github.com/AGSweeney/Motoman_EnIP_ToolChanger) on GitHub.

---

# Repository Visibility

This repository may periodically move between public and private visibility during active development.

Interfaces, documentation, and architecture are subject to substantial revision as development progresses.

---

# License

License information will be added as development progresses.

---

# Trademarks

Teknic, ClearCore, ClearLink, ClearPath, and ClearPath-IP are trademarks or registered trademarks of Teknic Inc.

All trademarks belong to their respective owners.
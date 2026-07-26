---
title: MotionBench
component: motionbench
level: project
topics:
  - CIP
  - ClearLink
  - Qt
  - bench
source_paths:
  - MotionBench/src/protocol/EtherNetIpClient.h
  - MotionBench/src/device/ClearLinkObjectMap.cpp
  - MotionBench/CMakeLists.txt
status: verified
---

# MotionBench

Host-side EtherNet/IP CIP client and UI for exercising ClearLink-compat objects (including board mode tooling).

Artifact: [A-P04-pat](../../../artifacts/patterns/motionbench-object-map.cpp).

## Source evidence

| Claim | Evidence | Level |
|-------|----------|-------|
| Object map board.mode | `ClearLinkObjectMap.cpp` | E1 |
| CIP client header | `EtherNetIpClient.h` | E1 |

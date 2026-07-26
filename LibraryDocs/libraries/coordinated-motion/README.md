---
title: CoordinatedMotion
component: coordinated-motion
level: library
reuse: high
platforms:
  - ClearCore-SAME53
topics:
  - arc
  - linear interpolation
  - motion queue
  - coordinated axes
source_paths:
  - ProjectTemplate/libClearCore/inc/CoordinatedMotionController.h
  - ProjectTemplate/libClearCore/src/CoordinatedMotionController.cpp
status: verified
retrieval:
  questions:
    - How do I initialize coordinated X/Y motors?
    - How are arc moves queued and consumed?
    - Which context consumes coordinated steps?
  related:
    - libraries/libclearcore/README.md
    - docs/ARC_INTERPOLATION_SPEC.md
---

# CoordinatedMotion

Dual-axis coordinated motion controller and interpolators inside libClearCore. **Reuse: high** for multi-axis research firmware (not wired into ClearLink EIP main today).

Artifacts: [A-L02-if](../../artifacts/interfaces/coordinated_motion_controller.hpp), [A-L02-pat](../../artifacts/patterns/coordinated-fast-path.cpp).

## Purpose and reuse

`CoordinatedMotionController` manages arc/linear queues for two `MotorDriver` axes.

## Public API surface

`Initialize(MotorDriver*, MotorDriver*)`, `MoveArc`, continuous/queued arc APIs; helpers `ArcInterpolator`, `LinearInterpolator`, `UnitConverter`.

## Initialization

Call `Initialize` with X/Y motor pointers before queueing moves.

## Runtime lifecycle

Command APIs from app context; step consumption during MotorDriver refresh / FastUpdate when coordinated mode is active.

## Threading and ownership

ISR-safe step consume path; do not block in FastUpdate.

## Configuration

Mechanical unit conversion via `UnitConverter` / motor `SetMechanicalParams` when used.

## Error handling

Boolean accept/reject on move APIs; check return values.

## Memory and resources

Internal motion queues; capacity documented in header members.

## Timing and performance

Must keep queue ahead of ISR consumption to avoid starvation.

## Data formats / wire protocol

Step counts in integer steps; angles in radians for arcs.

## Dependencies

L01 MotorDriver / StepGenerator.

## Integration points

Research firmware / Enhanced ClearCore Library demos; not used by P01 ClearLink main loop.

## Failure modes

| Symptom | Likely cause |\n|---------|--------------|\n| No coordinated motion | Motors not in step/dir or Initialize failed |\n| Stutter | Queue underrun |

## Limits and constraints

Two-axis focus; not a full CNC interpreter.

## Testing and validation

See `docs/ARC_INTERPOLATION_SPEC.md` for arc behavior notes.

## Portability

Requires ClearCore MotorDriver step path.

## Security

Not applicable (local motion).

## Logging and diagnostics

Status via controller query methods; app-level USB logging.

## Extension points

Queue depth, interpolator algorithms, more axes (future).

## Source evidence

| Claim | Evidence | Level |\n|-------|----------|-------|\n| Initialize takes two motors | `CoordinatedMotionController.h` `Initialize` | E1 |\n| MoveArc signature | same header `MoveArc` | E1 |\n| Lives in libClearCore | path under `ProjectTemplate/libClearCore` | E1 |


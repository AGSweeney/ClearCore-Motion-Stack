# Arc Interpolation Specification

This document defines the coordinated circular interpolation contract for the enhanced firmware.

It is intended to lock behavior for upcoming project delivery and prevent drift between PLC command generation, EtherNet/IP transport, and firmware planner execution.

Related documents:

- `docs/ASSEMBLY_LAYOUT.md` (parity baseline)
- `docs/ASSEMBLY_LAYOUT_ENHANCED.md` (enhanced runtime command transport)

## 1) Scope

In scope:

- Coordinated linear and circular trajectory segments over the enhanced queue interface.
- Deterministic arc geometry validation and execution.
- Junction transition behavior (including `JunctionDVmax` application).
- Runtime unit handling and conversion to internal counts.

Out of scope (initial release):

- Full NURBS/spline trajectory families.
- Dynamic look-ahead replan across already accepted segments.
- Multi-plane helical interpolation unless explicitly enabled in a future revision.

## 2) Canonical Internal Representation

- Internal planning and execution use **counts** as canonical units.
- Incoming engineering units (mm/in/deg) are converted to counts at command ingest time using configured scale factors.
- All geometric checks (radius consistency, tolerance checks, blending, jerk/accel constraints) operate in counts.

## 3) Segment Types

Segment type values (from enhanced assembly command payload):

- `1`: Linear coordinated move
- `2`: Circular CW move
- `3`: Circular CCW move
- `4`: Dwell

## 4) Arc Definition Modes

Initial supported modes:

- **Center-offset mode** (I/J style equivalent in selected plane)
- **Radius mode** (R style equivalent)
- **Coordinate mode** for arc targets:
  - **Absolute** target coordinates
  - **Incremental** target coordinates (relative to segment start)

The mode is encoded in segment flags/aux parameter fields as defined by assembly schema revision.

### 4.0 Coordinate Mode Semantics (Required)

- Arc commands must support both absolute and incremental target modes.
- Absolute mode: target fields are interpreted in the active unit space relative to the work coordinate frame.
- Incremental mode: target fields are interpreted as deltas from the current segment start point.
- Center-offset parameters remain start-relative in center-offset mode.
- Firmware must normalize both coordinate modes to a common internal start/end representation before geometric validation.

### 4.1 Center-Offset Mode

- Arc is defined by:
  - start point (current planner endpoint),
  - end point (segment target),
  - center offset relative to start point in active plane.
- Validation must confirm:
  - finite center values,
  - non-degenerate radius,
  - start/end distance to center match within tolerance.

### 4.2 Radius Mode

- Arc is defined by:
  - start point,
  - end point,
  - signed/flagged radius interpretation.
- Firmware resolves center from start/end/radius and direction (CW/CCW).
- Invalid geometric combinations are rejected deterministically.

### 4.3 Mode Encoding Requirements

- Segment flags must encode:
  - arc definition mode (center-offset vs radius)
  - coordinate mode (absolute vs incremental)
- Unknown or conflicting mode combinations must be rejected with explicit error.

## 5) Plane Policy

Initial release policy:

- **XY plane supported**
- Non-XY plane requests return explicit unsupported-plane error

Future extension may enable XZ/YZ (and helical behaviors) under a new interface revision.

## 6) Direction and Sweep Semantics

- Segment type determines direction:
  - `2` = CW
  - `3` = CCW
- Short-vs-long sweep selection must be deterministic and explicitly defined by flags.
- If sweep policy flag is absent, default policy is shortest valid sweep consistent with direction and mode.

## 7) Unit Contract

Unit mode is configured per motor through runtime configuration commands (no config assembly dependency), with enum:

- `0`: counts (raw)
- `1`: millimeters
- `2`: inches
- `3`: degrees

Scale conversion:

`counts = value_in_units * (counts_per_unit_numerator / counts_per_unit_denominator)`

If unit mode differs across participating motors in one coordinated segment, firmware rejects the segment unless explicit mixed-unit policy is enabled in future revision.

## 8) Transition Velocity and Junction Behavior

`JunctionDVmax` (steps/sec) is the per-axis delta-V cap used at segment boundaries.

- `0` disables `JunctionDVmax` cap and falls back to alternate junction policy.
- For non-zero value, junction velocity is additionally constrained by per-axis direction change limits.
- `JunctionDVmax` applies to:
  - line->line
  - line->arc
  - arc->line
  - arc->arc

Arc entry/exit tangency is enforced through planner continuity constraints prior to acceptance.

## 9) Queue and Execution Model

- Firmware accepts buffered segments through enhanced output assembly extension.
- Sequence/ack fields provide anti-stale command protection.
- Segment acceptance can be partial; rejected segments are reported with precise segment ID/error.
- Execution start can be explicit (`Start Buffered Execution`) after queueing.

## 10) Error Handling Contract

Invalid arc requests must be rejected with deterministic error codes, including at minimum:

- unsupported plane
- invalid radius/center geometry
- degenerate segment (zero-length with invalid arc params)
- impossible sweep for selected direction/mode
- invalid coordinate mode or unsupported mode combination
- mixed-unit incompatibility
- queue overflow
- planner not initialized/configured

On fault, firmware updates coordinated status fields:

- last error code
- fault bitfield
- fault source motor
- fault source segment

## 11) Determinism Requirements

- Identical command streams must produce identical planner decisions and status transitions.
- No hidden auto-corrections for invalid geometry; reject with explicit code instead.
- Timing-sensitive transitions (hold/resume/stop) must preserve segment integrity and state-machine consistency.

## 12) Minimum Validation Matrix

- CW/CCW arcs in XY plane under both center-offset and radius modes
- line->arc and arc->line transitions with `JunctionDVmax` = 0 and non-zero
- unit modes (counts, mm, in, deg) with conversion sanity checks
- boundary cases (very small arcs, large radius arcs, near-collinear endpoints)
- rejection behavior (invalid geometry, unsupported plane, queue overflow)

## 13) Versioning and Compatibility

- Behavior changes to arc semantics require interface revision bump.
- Existing parity assembly prefixes remain immutable.
- Enhanced semantics are versioned through runtime interface fields and command schema revision.

## 14) PLC-Style Pseudocode Example (Buffered CW Arc)

The following example shows a PLC-style sequence using the extended existing assemblies:

- Runtime configuration through instance `112` extension commands
- Status/ack monitoring through instance `100` extension status
- Queue one clockwise arc segment, then execute

```pascal
(*
  PSEUDOCODE - Structured Text style
  Based on docs/ASSEMBLY_LAYOUT_ENHANCED.md
*)

(* ---------- Constants ---------- *)
CMD_QUEUE_SEGMENTS         := 3;
CMD_START_BUFFERED_EXEC    := 4;
CMD_SET_MOTOR_UNIT_SCALE   := 11;
CMD_SET_JUNCTION_DVMAX     := 12;

SEG_LINEAR                 := 1;
SEG_ARC_CW                 := 2;
SEG_ARC_CCW                := 3;

UNIT_COUNTS                := 0;
UNIT_MM                    := 1;
UNIT_IN                    := 2;
UNIT_DEG                   := 3;

MOTOR_MASK_M0_M1           := 16#03;   (* bit0=M0, bit1=M1 *)

(* Example flag bits (project-defined in your schema) *)
FLAG_COORD_ABSOLUTE        := 16#0001; (* 0=incremental, 1=absolute *)
FLAG_ARC_MODE_CENTER       := 16#0002; (* center-offset mode *)
FLAG_PLANE_M0_M1           := 16#0010; (* XY-equivalent using M0/M1 *)

(* ---------- Tags (conceptual) ---------- *)
(* Out112Ext = extension fields in instance 112 (bytes 280+) *)
(* In100Ext  = extension fields in instance 100 (bytes 332+) *)

(* ---------- Step 1: Configure units/scaling via runtime command ---------- *)
Out112Ext.InterfaceRevision := 1;
Out112Ext.CommandCode       := CMD_SET_MOTOR_UNIT_SCALE;
Out112Ext.JobId             := 1001;
Out112Ext.SegmentBaseId     := 0;
Out112Ext.InputSequence     := Out112Ext.InputSequence + 1;

(* Slot0 -> M0 config *)
Out112Ext.Slot0.Byte0_MotorId         := 0;        (* M0 *)
Out112Ext.Slot0.Byte1_MotorFlags      := UNIT_MM;  (* UnitMode bits 0..2 *)
Out112Ext.Slot0.CountsPerUnitNum      := 20000;    (* example *)
Out112Ext.Slot0.CountsPerUnitDen      := 1;

(* Slot1 -> M1 config *)
Out112Ext.Slot1.Byte0_MotorId         := 1;        (* M1 *)
Out112Ext.Slot1.Byte1_MotorFlags      := UNIT_MM;
Out112Ext.Slot1.CountsPerUnitNum      := 20000;
Out112Ext.Slot1.CountsPerUnitDen      := 1;

(* Send/scan one cycle, then wait for ack echo *)
IF In100Ext.OutputSequenceEcho = Out112Ext.InputSequence THEN
    UnitsConfigured := TRUE;
END_IF;

(* ---------- Step 2: Set JunctionDVmax ---------- *)
IF UnitsConfigured THEN
    Out112Ext.CommandCode        := CMD_SET_JUNCTION_DVMAX;
    Out112Ext.InputSequence      := Out112Ext.InputSequence + 1;
    Out112Ext.Slot0.ConfigValue0 := 1500;  (* JunctionDVmax steps/sec *)
END_IF;

IF In100Ext.OutputSequenceEcho = Out112Ext.InputSequence THEN
    DVmaxConfigured := TRUE;
END_IF;

(* ---------- Step 3: Queue one CW arc segment ---------- *)
IF DVmaxConfigured THEN
    Out112Ext.CommandCode       := CMD_QUEUE_SEGMENTS;
    Out112Ext.JobId             := 2001;
    Out112Ext.SegmentBaseId     := 1;
    Out112Ext.InputSequence     := Out112Ext.InputSequence + 1;

    (* Slot0 = arc segment *)
    Out112Ext.Slot0.SegmentType := SEG_ARC_CW;
    Out112Ext.Slot0.MotorMask   := MOTOR_MASK_M0_M1;
    Out112Ext.Slot0.SegmentFlags :=
        FLAG_COORD_ABSOLUTE OR FLAG_ARC_MODE_CENTER OR FLAG_PLANE_M0_M1;

    (* Absolute target endpoint in mm (firmware converts using unit scale) *)
    Out112Ext.Slot0.TargetM0    := 50.0;   (* M0 endpoint *)
    Out112Ext.Slot0.TargetM1    := 25.0;   (* M1 endpoint *)
    Out112Ext.Slot0.TargetM2    := 0.0;
    Out112Ext.Slot0.TargetM3    := 0.0;

    Out112Ext.Slot0.Feed        := 120.0;  (* mm/s, interpreted by unit mode *)
    Out112Ext.Slot0.BlendRadius := 2.0;    (* mm *)

    (*
      AuxParameter encodes center-offset I/J equivalent for M0/M1.
      Example packing is project-defined; placeholder helper:
      PackArcCenterOffset(I_mm := 0.0, J_mm := -25.0)
    *)
    Out112Ext.Slot0.AuxParameter := PackArcCenterOffset(0.0, -25.0);
END_IF;

IF In100Ext.OutputSequenceEcho = Out112Ext.InputSequence THEN
    SegmentQueued := TRUE;
END_IF;

(* ---------- Step 4: Start buffered execution ---------- *)
IF SegmentQueued THEN
    Out112Ext.CommandCode   := CMD_START_BUFFERED_EXEC;
    Out112Ext.InputSequence := Out112Ext.InputSequence + 1;
END_IF;

(* ---------- Step 5: Monitor execution ---------- *)
IF In100Ext.ActiveCoordinatedJobId = 2001 THEN
    ArcInProgress := TRUE;
END_IF;

IF (In100Ext.LastCompletedJobId = 2001) AND (In100Ext.CoordinatedFaultBits = 0) THEN
    ArcDone := TRUE;
END_IF;

IF In100Ext.CoordinatedFaultBits <> 0 THEN
    ArcFault := TRUE;
    FaultCode := In100Ext.LastCoordinatedError;
    FaultMotor := In100Ext.FaultSourceMotor;
    FaultSegment := In100Ext.FaultSourceSegment;
END_IF;
```

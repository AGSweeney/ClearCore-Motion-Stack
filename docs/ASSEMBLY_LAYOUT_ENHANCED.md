# Enhanced Firmware Assembly Layout (Extended Existing Instances)

This document defines the enhanced EtherNet/IP assembly layout for coordinated multi-motor motion.

Design constraint:

- Do **not** add a second set of assembly instances for coordinated motion.
- Extend the existing ClearLink-compatible assembly instances so controllers such as Micro850 can use one adapter assembly set.

Baseline reference:

- `docs/ASSEMBLY_LAYOUT.md` (parity/base mapping)

## 1) Assembly Strategy

Enhanced firmware keeps the same Assembly Object instance IDs and extends payload length for the same instances:

- `100` (`0x64`) T2O Input (Step and Direction) - extended
- `112` (`0x70`) O2T Output (Step and Direction) - extended
- `150` (`0x96`) Configuration - extended

Optional mirror extension may be applied to M-Connector set if needed:

- `101` (`0x65`), `113` (`0x71`), `151` (`0x97`)

No new coordinated-motion assembly instance IDs are introduced.

## 2) Compatibility Model

Because assembly sizes change when extended fields are enabled, compatibility is handled by firmware variant selection:

- **Parity firmware variant**: exact ClearLink sizes and layout (baseline document).
- **Enhanced firmware variant**: same instance IDs with appended extension regions.

This preserves the single-assembly-set model required by controllers that cannot practically manage dual assembly sets on one adapter.

## 3) Extended Layout Principle

- Bytes `0..N` of each assembly remain byte-for-byte compatible with parity mapping.
- New coordinated-motion fields are appended after the parity payload end.
- Existing field offsets must never move.

Parity payload sizes (unchanged prefix):

- Instance `100`: first `332` bytes are parity-compatible
- Instance `112`: first `280` bytes are parity-compatible
- Instance `150`: first `232` bytes are parity-compatible

## 4) Instance 100 (0x64) Extended T2O Input

### 4.1 Size

- Base parity: `332`
- Enhanced extension: `+96`
- Enhanced total: `428` bytes

### 4.2 Extension Region (`332-427`)

#### Coordinated status header (`332-363`)

- `332-333`: Coordinated Interface Revision `UINT`
- `334-335`: Coordinated Capability Bits `UINT`
- `336-339`: Coordinated Status Bits `DWORD`
- `340-343`: Active Coordinated Job ID `UDINT`
- `344-347`: Active Segment ID `UDINT`
- `348-351`: Buffered Segment Count `UDINT`
- `352-355`: Queue Free Slots `UDINT`
- `356-359`: Last Completed Job ID `UDINT`
- `360-363`: Last Coordinated Error `UDINT`

#### Coordinated path state (`364-403`)

- `364-367`: Path Position M0 (counts) `DINT`
- `368-371`: Path Position M1 (counts) `DINT`
- `372-375`: Path Position M2 (counts) `DINT`
- `376-379`: Path Position M3 (counts) `DINT`
- `380-383`: Path Velocity Magnitude (counts/s) `UDINT`
- `384-387`: Feed Override Active (0.001 scale) `UDINT`
- `388-391`: Active Segment Progress (0.001 scale) `UDINT`
- `392-395`: Active Blend Radius (counts) `UDINT`
- `396-399`: Active Trajectory Time (ms) `UDINT`
- `400-403`: Remaining Trajectory Time (ms) `UDINT`

#### Coordinated diagnostics/handshake (`404-427`)

- `404-407`: Input Sequence Ack `UDINT`
- `408-411`: Output Sequence Echo `UDINT`
- `412-415`: Coordinated Warning Bits `DWORD`
- `416-419`: Coordinated Fault Bits `DWORD`
- `420-423`: Fault Source Motor `UDINT`
- `424-427`: Fault Source Segment `UDINT`

## 5) Instance 112 (0x70) Extended O2T Output

### 5.1 Size

- Base parity: `280`
- Enhanced extension: `+176`
- Enhanced total: `456` bytes

### 5.2 Extension Region (`280-455`)

#### Coordinated command header (`280-311`)

- `280-281`: Coordinated Interface Revision `UINT`
- `282-283`: Coordinated Command Code `UINT`
- `284-287`: Coordinated Command Options `DWORD`
- `288-291`: Coordinated Job ID `UDINT`
- `292-295`: Coordinated Segment Base ID `UDINT`
- `296-299`: Input Sequence `UDINT`
- `300-303`: Requested Feed Override (0.001 scale) `UDINT`
- `304-307`: Requested Buffer Depth `UDINT`
- `308-311`: Reserved

#### Coordinated segment queue payload (`312-439`)

Four fixed segment slots (`32` bytes each):

- Slot 0: `312-343`
- Slot 1: `344-375`
- Slot 2: `376-407`
- Slot 3: `408-439`

Per-slot layout:

- `+0`: Segment Type `USINT`
- `+1`: Motor Mask `USINT`
- `+2-3`: Segment Flags `UINT`
- `+4-7`: Target M0 (counts) `DINT`
- `+8-11`: Target M1 (counts) `DINT`
- `+12-15`: Target M2 (counts) `DINT`
- `+16-19`: Target M3 (counts) `DINT`
- `+20-23`: Feed (counts/s) `UDINT`
- `+24-27`: Blend Radius (counts) `UDINT`
- `+28-31`: Aux Parameter `UDINT`

Segment types:

- `0`: unused
- `1`: linear coordinated move
- `2`: circular CW
- `3`: circular CCW
- `4`: dwell

For circular segments, `Aux Parameter` carries arc interpretation mode and plane selection.

#### Reserved/future (`440-455`)

- `440-455`: Reserved for future trajectory metadata

## 6) Instance 150 (0x96) Extended Configuration

### 6.1 Size

- Base parity: `232`
- Enhanced extension: `+68`
- Enhanced total: `300` bytes

### 6.2 Extension Region (`232-299`)

#### Coordinated planner config (`232-275`)

- `232-233`: Coordinated Interface Revision `UINT`
- `234-235`: Coordinated Config Flags `UINT`
- `236-239`: Default Feed Override (0.001 scale) `UDINT`
- `240-243`: Max Feed (counts/s) `UDINT`
- `244-247`: Max Accel (counts/s^2) `UDINT`
- `248-251`: Max Decel (counts/s^2) `UDINT`
- `252-255`: Default Blend Radius (counts) `UDINT`
- `256-259`: Path Tolerance (counts) `UDINT`
- `260-263`: Queue Capacity (segments) `UDINT`
- `264-267`: Watchdog Timeout (ms) `UDINT`
- `268-271`: Planner Cycle Time (us) `UDINT`
- `272-275`: JunctionDVmax (steps/sec) `UDINT` (0 = disabled)

`JunctionDVmax` maps directly to the coordinated motion controller junction delta-V limiter (`JunctionDVmax` / `dVmax`) used by the Enhanced ClearCore Library.

#### Motor mapping/scaling (`276-299`)

Motor blocks (M0, M1, M2, M3), 6 bytes each:

- `276-281`: Motor M0 mapping block
- `282-287`: Motor M1 mapping block
- `288-293`: Motor M2 mapping block
- `294-299`: Motor M3 mapping block

Per-motor block:

- Motor Instance ID `USINT`
- Motor Flags `USINT`
- Counts-per-unit numerator `UINT`
- Counts-per-unit denominator `UINT`

## 7) Coordinated Command Codes (Instance 112 Extension)

- `0`: NOP
- `1`: Planner Reset
- `2`: Queue Clear
- `3`: Queue Segments
- `4`: Start Buffered Execution
- `5`: Feed Hold
- `6`: Resume
- `7`: Controlled Stop
- `8`: Emergency Stop
- `9`: Set Feed Override

## 8) Determinism and Error Semantics

- Unknown command codes must return deterministic error status in extension status fields.
- Coordinated faults must populate:
  - Last Coordinated Error
  - Coordinated Fault Bits
  - Fault Source Motor / Segment
- Extension faults must not change parity field meanings in the base prefix.

## 9) Implementation Notes

- Keep parity prefixes immutable and covered by regression tests.
- Gate coordinated behavior on interface revision/capability fields in extended regions.
- Document exact assembly byte sizes in EDS for the enhanced firmware variant.
- If both firmware variants are distributed, provide distinct EDS files per variant to avoid scanner size mismatch.

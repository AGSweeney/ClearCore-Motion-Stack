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
- `150` (`0x96`) Configuration - parity only (not used for enhanced settings)

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
- Instance `150`: full parity payload (`232`) remains unchanged

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

## 6) Runtime Configuration via Instance 112 (No Config Assembly Usage)

Enhanced coordinated settings are applied through command transactions in instance `112` extension fields.

Instance `150` remains parity-only and is not required for enhanced coordinated setup.

Enhanced runtime configuration is carried through the extended output assembly because some target PLCs (including Micro850) do not reliably support configuration-assembly workflows in this integration model.

### 6.1 Configuration Commands

Use `Coordinated Command Code` plus command payload in the extension area (`312-439`) as follows:

- `10`: Set Planner Global Config
- `11`: Set Motor Unit/Scaling Config
- `12`: Set JunctionDVmax
- `13`: Read Back Config Snapshot Request

### 6.2 Command Payload Mapping

When command code is `10` (Set Planner Global Config), Slot 0 (`312-343`) is interpreted as:

- `312-315`: Config Flags `DWORD`
- `316-319`: Max Feed (counts/s) `UDINT`
- `320-323`: Max Accel (counts/s^2) `UDINT`
- `324-327`: Max Decel (counts/s^2) `UDINT`
- `328-331`: Default Blend Radius (counts) `UDINT`
- `332-335`: Path Tolerance (counts) `UDINT`
- `336-339`: Queue Capacity (segments) `UDINT`
- `340-343`: Planner Cycle Time (us) `UDINT`

When command code is `11` (Set Motor Unit/Scaling Config), each slot defines one motor:

- Slot 0 (`312-343`): M0 config
- Slot 1 (`344-375`): M1 config
- Slot 2 (`376-407`): M2 config
- Slot 3 (`408-439`): M3 config

Per-slot motor config fields:

- `+0`: Motor ID (`USINT`, expected 0..3)
- `+1`: Motor Flags (`USINT`)
- `+2-3`: Reserved
- `+4-7`: Counts-per-unit numerator `UDINT`
- `+8-11`: Counts-per-unit denominator `UDINT`
- `+12-31`: Reserved

`Motor Flags` bit allocation (per motor):

- Bits `0-2`: Unit Mode
- Bit `3`: Reserved for future "rotary wrap enabled"
- Bits `4-7`: Reserved

Unit Mode enum:

- `0`: counts (raw)
- `1`: millimeters
- `2`: inches
- `3`: degrees
- `4-7`: reserved

When Unit Mode is not `counts`, command values are interpreted in engineering units and converted using:

`counts = value_in_units * (counts_per_unit_numerator / counts_per_unit_denominator)`

When command code is `12` (Set JunctionDVmax), Slot 0 field `312-315` is:

- `JunctionDVmax` (steps/sec) `UDINT` (`0 = disabled`)

`JunctionDVmax` maps directly to the coordinated motion controller junction delta-V limiter (`JunctionDVmax` / `dVmax`) used by the Enhanced ClearCore Library.

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
- `10`: Set Planner Global Config
- `11`: Set Motor Unit/Scaling Config
- `12`: Set JunctionDVmax
- `13`: Read Back Config Snapshot Request

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
- Keep configuration workflow runtime-only through instance `112`; avoid dependence on configuration assembly writes for enhanced features.
- If both firmware variants are distributed, provide distinct EDS files per variant to avoid scanner size mismatch.

# ClearLink Assembly Layout (Parity Map)

This document summarizes the EtherNet/IP assembly layout used for parity firmware in this project.

**Authority when anything disagrees:** `ExternalReferances/clearlink_ethernet-ip_object_reference.pdf` (*ClearLink EtherNet/IP Object Data Reference* / object reference) is the standard. If this markdown, the setup manual, or code diverges from that PDF on byte layout, class/instance/attribute definitions, or bit meanings, **reconcile toward the PDF** (then update this file and firmware to match).

Informative citation (revision naming may differ by publication):

- *ClearLink EtherNet/IP Setup and Object Data Reference*, Rev. 1.15 (February 20, 2025)

## Assembly Object Summary

Assembly Object Class: `0x04`

| Instance | Hex  | Direction | Name | Size |
|---|---|---|---|---|
| 100 | 0x64 | T2O (Input) | Assembly1 (Step and Direction Motors) | `SINT[332]` |
| 101 | 0x65 | T2O (Input) | Assembly2 (M-Connectors) | `SINT[228]` |
| 112 | 0x70 | O2T (Output) | Assembly1 (Step and Direction Motors) | `SINT[280]` |
| 113 | 0x71 | O2T (Output) | Assembly2 (M-Connectors) | `SINT[200]` |
| 150 | 0x96 | Config (Get/Set) | Configuration Assembly1 (Step and Direction Motors) | `SINT[232]` |
| 151 | 0x97 | Config (Get/Set) | Configuration Assembly2 (M-Connectors) | `SINT[120]` |

## Shared I/O Segment Layout (Used by Both T2O Inputs)

Byte ranges and fields:

- `0-1`: DIP Value `BOOL[13]`
- `2-3`: DIP Status `BOOL[13]`
- `4-11`: AIP Value `INT[4]`
- `12-13`: AIOP Status `BOOL[5]`
- `14-15`: DOP Status `BOOL[6]`
- `16-23`: CCIO Input Value `ULINT`
- `24-31`: CCIO Status `ULINT`
- `32`: CCIO Board Count `USINT`
- `33-35`: Reserved

AIOP status bit order:

- A-9, A-10, A-11, A-12, IO-0 (Analog Output)

## Instance 100 (0x64): Step and Direction Input (T2O)

Total size: `332` bytes

### Encoder Input

- `36-39`: Encoder Position `DINT`
- `40-43`: Encoder Velocity `DINT`
- `44-47`: Encoder Index Position `DINT`
- `48`: Encoder Status `BOOL[2]`
- `49-51`: Reserved

### Per-Motor Input Blocks (`0x65`, instances 1-4)

Each motor block is 32 bytes:

- Commanded Position `DINT`
- Commanded Velocity `DINT`
- Target Position `DINT`
- Target Velocity `DINT`
- Captured Position `DINT`
- Measured Torque `REAL`
- Status Register `DWORD`
- Shutdown Register `DWORD`

Offsets:

- Motor 0: `52-83`
- Motor 1: `84-115`
- Motor 2: `116-147`
- Motor 3: `148-179`

### Serial ASCII Input Segment

- `180-183`: Serial Status `DWORD`
- `184-187`: Output Char Count `UDINT`
- `188-191`: Input Char Count `UDINT`
- `192-195`: Output Sequence Ack `UDINT`
- `196-199`: Input Size `UDINT`
- `200-203`: Input Sequence `UDINT`
- `204-331`: Input Data `USINT[128]`

## Instance 112 (0x70): Step and Direction Output (O2T)

Total size: `280` bytes

### I/O + Encoder Output Segment

- `0-1`: AOP Value `INT`
- `2-3`: DOP Value `BOOL[6]`
- `4-9`: DOP PWM `USINT[6]`
- `10-11`: Reserved
- `12-19`: CCIO Output Value `ULINT`
- `20-23`: Encoder AddToPosition `DINT`

### Per-Motor Output Blocks (`0x66`, instances 1-4)

Each motor block is 28 bytes:

- Move Distance `DINT`
- Velocity Limit `UDINT`
- Acceleration Limit `UDINT`
- Deceleration Limit `UDINT`
- Jog Velocity `DINT`
- Add To Position `DINT`
- Output Register `DWORD`

Offsets:

- Motor 0: `24-51`
- Motor 1: `52-79`
- Motor 2: `80-107`
- Motor 3: `108-135`

### Serial ASCII Output Segment

- `136-139`: Serial Config `DWORD`
- `140-143`: Input Sequence Ack `UDINT`
- `144-147`: Output Size `UDINT`
- `148-151`: Output Sequence `UDINT`
- `152-279`: Output Data `USINT[128]`

## Instance 150 (0x96): Step and Direction Configuration Assembly

Total size: `232` bytes

### I/O Mode + Filter + Encoder Config

- `0-4`: AI0..AI3 Range, AO0 Range (`USINT`)
- `5-6`: DOP PWM Frequency / CCIO Enable bitfield
- `7`: Reserved
- `8-11`: AIP Filters
- `12-63`: DIP Filters `UINT[26]`
- `64-71`: CCIO Filters `USINT[8]`
- `72-75`: Encoder Velocity Resolution `UDINT`
- `76`: Reserved
- `77`: Encoder Configuration
- `78-79`: Reserved

DIP filter ordering:

- DIP0 Off_On, DIP0 On_Off, DIP1 Off_On, DIP1 On_Off, ...

### Per-Motor Configuration Blocks (`0x64`, instances 1-4)

Each motor block is **32 bytes** (PDF Configuration Assembly 150):

| Byte offset (within block) | Field | CIP type |
|---:|---|---|
| 0–3 | Config Register | `DWORD` |
| 4–7 | Follow Divisor | `DINT` |
| 8–11 | Follow Multiplier | `DINT` |
| 12–15 | Max Deceleration | `DINT` |
| 16–19 | Soft Limit 1 | `DINT` |
| 20–23 | Soft Limit 2 | `DINT` |
| 24 | Positive Limit connector | `SINT` (1 byte) |
| 25 | Negative Limit | `SINT` |
| 26 | Home Sensor | `SINT` |
| 27 | Brake | `SINT` |
| 28 | Stop Sensor | `SINT` |
| 29 | Position Capture Sensor | `SINT` |
| 30 | Follow Axis | `SINT` |
| 31 | Reserved padding | — |

Absolute byte ranges:

- Motor 0: `80-111`
- Motor 1: `112-143`
- Motor 2: `144-175`
- Motor 3: `176-207`

### Serial Config

- `208-211`: Serial Baud Rate `UDINT`
- `212-215`: Input Start Delimiter `DWORD`
- `216-219`: Input End Delimiter `DWORD`
- `220-223`: Output Start Delimiter `DWORD`
- `224-227`: Output End Delimiter `DWORD`
- `228-231`: Input Timeout `UDINT`

## Instance 101 (0x65): Motor Connector Input (T2O)

Total size: `228` bytes

### I/O + Encoder Input Segments

Identical layout to Instance 100 for bytes `0-51`.

### Motor Connector Input Data (`0x67`, instances 1-4)

Per motor:

- HLFB Duty `REAL`
- Status Reg `WORD`

Offsets:

- Motor 0 HLFB: `52-55`, Status: `68-69`
- Motor 1 HLFB: `56-59`, Status: `70-71`
- Motor 2 HLFB: `60-63`, Status: `72-73`
- Motor 3 HLFB: `64-67`, Status: `74-75`

### Serial ASCII Input Segment

- `76-79`: Serial Status `DWORD`
- `80-83`: Output Char Count `UDINT`
- `84-87`: Input Char Count `UDINT`
- `88-91`: Output Sequence Ack `UDINT`
- `92-95`: Input Size `UDINT`
- `96-99`: Input Sequence `UDINT`
- `100-227`: Input Data `USINT[128]`

## Instance 113 (0x71): Motor Connector Output (O2T)

Total size: `200` bytes

### I/O + Encoder Output Segment

- `0-1`: AOP Value `INT`
- `2-3`: DOP Value `BOOL[6]`
- `4-9`: DOP PWM `USINT[6]`
- `10-11`: Reserved
- `12-19`: CCIO Output Value `ULINT`
- `20-23`: Encoder AddToPosition `DINT`

### Per-Motor Connector Output Blocks (`0x67`, instances 1-4)

Each motor block is 8 bytes:

- Trig Pulses `UINT`
- A PWM `UINT`
- B PWM `UINT`
- Output Register `USINT` (+ padding)

Offsets:

- Motor 0: `24-31`
- Motor 1: `32-39`
- Motor 2: `40-47`
- Motor 3: `48-55`

### Serial ASCII Output Segment

- `56-59`: Serial Config `DWORD`
- `60-63`: Input Sequence Ack `UDINT`
- `64-67`: Output Size `UDINT`
- `68-71`: Output Sequence `UDINT`
- `72-199`: Output Data `USINT[128]`

## Instance 151 (0x97): Motor Connector Configuration Assembly

Total size: `120` bytes

### I/O Mode + Filter + Encoder Config

- `0-4`: AI0..AI3 Range, AO0 Range
- `5-6`: DOP PWM Frequency / CCIO Enable bitfield
- `7`: Reserved
- `8-11`: AIP Filters
- `12-63`: DIP Filters `UINT[26]`
- `64-71`: CCIO Filters `USINT[8]`
- `72-75`: Encoder Velocity Resolution `UDINT`
- `76`: Reserved
- `77`: Encoder Configuration
- `78-79`: Reserved

### Motor Connector Config (4 motors)

Bytes `80-95`: **16 bytes** (4 motors × **4 bytes**), per PDF Configuration Assembly 151. Each 4-byte slice:

| Offset in slice | Field | CIP type |
|---:|---|---|
| 0 | Enable Input Connector | `SINT` (1 byte) |
| 1 | A Input Connector | `SINT` |
| 2 | B Input Connector | `SINT` |
| 3 | Trigger Pulse Time | `USINT` |

Absolute ranges:

- Motor 0: `80-83`
- Motor 1: `84-87`
- Motor 2: `88-91`
- Motor 3: `92-95`

### Serial Config

- `96-99`: Serial Baud Rate `UDINT`
- `100-103`: Input Start Delimiter `DWORD`
- `104-107`: Input End Delimiter `DWORD`
- `108-111`: Output Start Delimiter `DWORD`
- `112-115`: Output End Delimiter `DWORD`
- `116-119`: Input Timeout `UDINT`

## Implementation Notes for Parity Firmware

- Assembly and object semantics: **PDF object reference first**; keep this document aligned with it after code changes.
- Keep instance IDs and payload sizes fixed for parity mode.
- Preserve ordering and packing of status bitfields and filter arrays exactly as specified.
- Validate both Step/Direction (`100/112/150`) and M-Connector (`101/113/151`) assembly families.
- **Class 0x65 motor status / shutdowns** packed into assemblies **100** / explicit **Get**: layout follows PDF **Table 24** (status DWORD) and **Table 25** (shutdown DWORD). `libClearCore` `AlertRegMotor` maps to Table 25 bits 0–3, 5, and 10; Table 25 **bit 4** (SW E-Stop) and **bit 6** (soft limit exceeded) are **host-latched** in parity firmware when the O2T output register (Table 28) asserts **SW E-Stop (bit 5)** or rejects a **Load Position** due to soft limits; **Clear Alerts (output bit 6)** clears those latch bits together with `MotorDriver::ClearAlerts`.
- **Class 0x64 config DWORD** (Table 21): bit **2** enable inversion, bit **3** HLFB active-level selector in this bridge implementation, bit **5** soft limit enable (with two distinct soft-limit positions). Default `0x0008` per configuration assembly table.
- **HLFB config bit behavior:** host-visible bit 3 is treated as HLFB active-high selection for parity with existing ClearLink tooling; it is inverted when applied to `MotorDriver::PolarityInvertSDHlfb(...)` because `PolarityInvertSDHlfb(true)` means active-low at the driver layer.
- **Class 0x69 board mode (instance 1, attr 2):** host semantics are `0 = Step/Direction`, `1 = non-Step/Direction (M-connector style)`. Bridge helper `BoardMotorMode_Request(nonzero)` uses the opposite polarity (`nonzero => Step/Direction`) internally.
- **Class 0x66 Add To Position (attr 7 / assembly 112 per-motor field):** non-zero command is edge-applied once per non-zero period (requires return to `0` to re-arm), and now updates the real motor commanded reference via `PositionRefSet(PositionRefCommanded() + delta)` instead of only modifying mirrored input fields.
- **Class 0x65 input block target velocity field:** for accepted positional moves, parity firmware mirrors the active velocity limit into `Target Velocity` so host UIs observing Table 24/assembly input data see a non-zero target-speed context during position commands.
- **Fault/status policy:** parity firmware maps only real `MotorDriver` status/alert content plus the explicit host-latched Table 25 bits documented above (SW E-Stop and soft-limit-exceeded). It does not synthesize additional motor-fault bits beyond those sources.

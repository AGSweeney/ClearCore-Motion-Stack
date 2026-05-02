# ClearLink Assembly Layout (Source-of-Truth)

This document defines the EtherNet/IP Assembly layout used for parity behavior in this project.

Source of truth:

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

Each motor block is 32 bytes and contains:

- Config Register `DWORD`
- Follow Divisor `DINT`
- Follow Multiplier `DINT`
- Max Deceleration `DINT`
- Soft Limit 1 `DINT`
- Soft Limit 2 `DINT`
- Positive Limit `SINT`
- Negative Limit `SINT`
- Home Sensor `SINT`
- Brake `SINT`
- Stop Sensor `SINT`
- Position Capture Sensor `SINT`
- Follow Axis `SINT`
- Reserved byte

Offsets:

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

Per motor (4 bytes each):

- Enable Input Connector `SINT`
- A Input Connector `SINT`
- B Input Connector `SINT`
- Trigger Pulse Time `USINT`

Offsets:

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

- Treat this document and the cited ClearLink reference revision as authoritative for assembly byte layout.
- Keep instance IDs and payload sizes fixed for parity mode.
- Preserve ordering and packing of status bitfields and filter arrays exactly as specified.
- Validate both Step/Direction (`100/112/150`) and M-Connector (`101/113/151`) assembly families.

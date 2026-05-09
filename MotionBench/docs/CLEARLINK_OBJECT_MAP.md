# ClearLink Object Map Inventory

This document freezes the canonical object map used by `MotionBench` for monitoring and commands.

## Source Basis

- Legacy monitor implementation: `ExternalReferances/ClearLinkMonitor-main/Form1.cs`
- Discovery implementation: `ExternalReferances/ClearLinkMonitor-main/Discover.cs`
- ClearLink object reference PDF: `ExternalReferances/clearlink_ethernet-ip_object_reference.pdf`

## Object Attributes

| Key | Class | Instance | Attribute | Type | RW | Notes |
| --- | --- | --- | --- | --- | --- | --- |
| `identity.serial_number` | `0x01` | `1` | `6` | `DINT` | R | Identity serial number |
| `identity.mac_address` | `0xF6` | `1` | `3` | `BYTE[]` | R | MAC address bytes |
| `board.mode` | `0x69` | `1` | `2` | `BOOL` | R | `false` step/dir, `true` M-connector |
| `board.supply_voltage` | `0x69` | `1` | `5` | `REAL` | R | Supply voltage |
| `assembly.discrete_input_bits` | `0x04` | `100` | `3` | `BYTE[]` | R | Legacy discrete input bitfield |
| `ccio.io_status_bits` | `0x68` | `1` | `2` | `BYTE[]` | R | 64 CCIO bits (8 boards x 8 bits) |
| `ccio.board_count` | `0x68` | `1` | `4` | `SINT` | R | CCIO board count |
| `ccio.enabled` | `0x68` | `1` | `6` | `BOOL` | R | CCIO enabled state |
| `motor.commanded_position` | `0x65` | `1..4` | `1` | `DINT` | R | Per selected motor instance |
| `motor.commanded_velocity` | `0x65` | `1..4` | `2` | `DINT` | R | Per selected motor instance |
| `motor.target_position` | `0x65` | `1..4` | `3` | `DINT` | R | Per selected motor instance |
| `motor.target_velocity` | `0x65` | `1..4` | `4` | `DINT` | R | Per selected motor instance |
| `motor.captured_position` | `0x65` | `1..4` | `5` | `DINT` | R | Per selected motor instance |
| `motor.measured_torque` | `0x65` | `1..4` | `6` | `REAL` | R | Per selected motor instance |
| `motor.status_reg` | `0x65` | `1..4` | `7` | `BYTE[]` | R | Status bits |
| `motor.alert_reg` | `0x65` | `1..4` | `8` | `BYTE[]` | R | Alert bits |
| `motor.move_distance` | `0x66` | `1..4` | `1` | `DINT` | RW | Command object |
| `motor.velocity` | `0x66` | `1..4` | `2` | `DINT` | RW | Command object |
| `motor.move_velocity` | `0x66` | `1..4` | `3` | `DINT` | RW | Command object |
| `motor.accel` | `0x66` | `1..4` | `4` | `DINT` | RW | Command object |
| `motor.decel` | `0x66` | `1..4` | `5` | `DINT` | RW | Command object |
| `motor.control_reg` | `0x66` | `1..4` | `6` | `DWORD` | RW | Control register bits |
| `motor.add_to_position` | `0x66` | `1..4` | `7` | `DINT` | RW | Add-to-position / ACK usage |
| `mconnector.output_reg` | `0x67` | `1..4` | `1` | `BYTE[]` | R | M-connector output bits |
| `mconnector.status_reg` | `0x67` | `1..4` | `2` | `BYTE[]` | R | M-connector status bits |
| `mconnector.measured_torque` | `0x67` | `1..4` | `6` | `REAL` | R | M-connector torque |
| `mconnector.pwm_a` | `0x67` | `1..4` | `8` | `INT` | R | PWM A |
| `mconnector.pwm_b` | `0x67` | `1..4` | `9` | `INT` | R | PWM B |
| `mconnector.trigger` | `0x67` | `1..4` | `10` | `SINT` | R | Trigger value |
| `digital_output.state` | `0x09` | `1..6` | `3` | `BOOL` | RW | Legacy monitor writes instances 1..6 |

## Safety Notes

- Write access is constrained in `DeviceService` to explicit, known attributes only.
- Unsupported keys are rejected; read-only keys cannot be written.
- Motor instance is constrained to `1..4`.
- Polling is constrained to `50..5000 ms` to avoid accidental overload.

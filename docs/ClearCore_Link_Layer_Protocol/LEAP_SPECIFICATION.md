<!--
Copyright (c) 2026 Adam G. Sweeney
SPDX-License-Identifier: MIT

Purpose: Draft specification for the Layer2 Ethernet Automation Protocol.
-->

# LEAP Protocol Specification

## 1. Scope

LEAP, the Layer2 Ethernet Automation Protocol, is a project-owned raw Ethernet
protocol suite for deterministic device I/O, device discovery, profile
description, diagnostics, and safe ownership over a private machine network.

LEAP is inspired by lessons learned while building the experimental EtherCAT
personality, but it is not EtherCAT and must not use EtherCAT conformance terms
for its own behavior. LEAP removes EtherCAT Slave Controller emulation,
SyncManager configuration, FMMU mapping, SII EEPROM categories, ESI XML, AL
status registers, and working-counter semantics. Equivalent LEAP features are
defined directly in this document.

## 2. Normative Language

The words `MUST`, `MUST NOT`, `SHALL`, `SHOULD`, and `MAY` are used for protocol
requirements. Draft items are marked `REVIEW`.

## 3. Transport

LEAP frames are raw Ethernet frames on an isolated industrial cell network.

- Physical medium: standard Ethernet supported by the target hardware.
- Minimum MTU: `1500` bytes.
- Byte order: little-endian for all multi-byte protocol fields.
- Addressing: Ethernet unicast for sessions, Ethernet broadcast for discovery.
- Production EtherType: `REVIEW`, requires a formally assigned EtherType.
- Lab EtherType: `0x88B5` MAY be used only on isolated local experimental
  networks when that value is acceptable under the deployment policy.

LEAP implementations MUST reject frames with malformed lengths, unsupported
major versions, failed header checks, failed payload checks, invalid state for
the requested service, or payload sizes that exceed the current endpoint limits.

## 4. Layering

LEAP is split into small services:

- `LEAP Frame`: common Ethernet payload header, sequencing, and checks.
- `LEAP-MGMT`: sessions, ownership, state control, watchdog, and fault reset.
- `LEAP-DISC`: discovery and identity bootstrap.
- `LEAP-DIR`: device directory, endpoint table, profile metadata, and config.
- `LEAP-PD`: cyclic process-data write, read, and exchange.
- `LEAP-DIAG`: counters, timing, fault, and event log access.

Firmware hardware control remains outside the transport. The transport owns
frame validation, sessions, endpoint buffers, state, and diagnostics. The
application owns ClearCore connector modes, output application, input sampling,
and status population.

## 5. Frame Format

The Ethernet payload begins with a fixed 32-byte LEAP header.

| Offset | Size | Field | Description |
| ---: | ---: | --- | --- |
| `0` | `4` | `magic` | ASCII bytes `LEAP` |
| `4` | `1` | `version_major` | incompatible protocol version |
| `5` | `1` | `version_minor` | compatible feature version |
| `6` | `1` | `header_length` | header bytes, initially `32` |
| `7` | `1` | `flags` | frame flags |
| `8` | `2` | `service_id` | service namespace |
| `10` | `2` | `message_type` | service-local message type |
| `12` | `4` | `session_id` | `0` for discovery and pre-session traffic |
| `16` | `4` | `sequence` | sender sequence number |
| `20` | `4` | `ack_sequence` | latest peer sequence observed, or `0` |
| `24` | `2` | `payload_length` | payload bytes after this header |
| `26` | `2` | `header_crc16` | CRC-16 over header with this field zeroed |
| `28` | `4` | `payload_crc32c` | CRC-32C over payload, or `0` when flag disables it |

### 5.1 Flags

| Bit | Name | Meaning |
| ---: | --- | --- |
| `0` | `ACK_REQUESTED` | receiver SHOULD send an explicit response |
| `1` | `RESPONSE` | frame is a response to a previous request |
| `2` | `ERROR` | response payload begins with `LeapErrorPayload` |
| `3` | `BROADCAST` | sender intentionally used broadcast destination |
| `4` | `NO_PAYLOAD_CRC` | payload CRC field is ignored; only allowed during discovery |
| `5` | `FRAGMENTED` | payload begins with `LeapFragmentHeader` |
| `6` | `TIME_VALID` | timestamp TLV, when present, is meaningful |
| `7` | `RESERVED` | send as `0`, ignore on receive |

## 6. Service Registry

| Service ID | Service | Purpose |
| ---: | --- | --- |
| `0x0001` | `LEAP-MGMT` | session, state, owner lease, watchdog |
| `0x0002` | `LEAP-DISC` | discovery and identity bootstrap |
| `0x0003` | `LEAP-DIR` | directory, endpoint table, profile metadata |
| `0x0010` | `LEAP-PD` | cyclic process-data exchange |
| `0x0020` | `LEAP-DIAG` | counters, timing, event and fault reporting |
| `0x8000..0xFFFE` | vendor/project extension | private extensions |
| `0xFFFF` | reserved | invalid |

Service IDs below `0x8000` are protocol-governed. New standard services require
an update to this specification.

## 7. Common Payload Types

### 7.1 TLV Encoding

Many metadata messages use TLV fields:

| Offset | Size | Field | Description |
| ---: | ---: | --- | --- |
| `0` | `2` | `type` | registry-defined TLV type |
| `2` | `2` | `length` | value bytes |
| `4` | `length` | `value` | field payload |

TLVs are padded to a 4-byte boundary. Receivers MUST skip unknown TLV types with
valid lengths. Receivers MUST reject TLVs whose declared length exceeds the
remaining payload.

### 7.2 Error Payload

All error responses begin with:

| Offset | Size | Field | Description |
| ---: | ---: | --- | --- |
| `0` | `2` | `status_code` | LEAP status code |
| `2` | `2` | `detail_code` | service-local detail |
| `4` | `4` | `rejected_sequence` | sequence number that failed |
| `8` | `4` | `affected_offset` | optional byte/object offset |
| `12` | `4` | `affected_length` | optional byte/object length |

Standard status codes:

| Code | Name | Meaning |
| ---: | --- | --- |
| `0x0000` | `OK` | request accepted |
| `0x0001` | `UNSUPPORTED_VERSION` | major version is not supported |
| `0x0002` | `BAD_LENGTH` | frame, header, TLV, or payload length invalid |
| `0x0003` | `BAD_CHECK` | header or payload integrity check failed |
| `0x0004` | `UNSUPPORTED_SERVICE` | service is unknown or disabled |
| `0x0005` | `UNSUPPORTED_MESSAGE` | message is unknown for the service |
| `0x0006` | `INVALID_STATE` | message is not valid in current device state |
| `0x0007` | `NOT_OWNER` | sender does not own the active control lease |
| `0x0008` | `LEASE_EXPIRED` | active lease expired before request arrived |
| `0x0009` | `BUSY` | device cannot complete request now |
| `0x000A` | `FAULTED` | device is faulted and requires reset or safe action |
| `0x000B` | `RANGE` | object, endpoint, offset, or length out of range |
| `0x000C` | `RATE_LIMITED` | sender exceeded configured service rate |

## 8. Discovery Service

Discovery uses `session_id = 0` and MAY disable payload CRC for a minimal
`HELLO` frame. Discovery responses SHOULD use payload CRC.

| Message Type | Name | Direction |
| ---: | --- | --- |
| `0x0001` | `HELLO` | controller broadcast to devices |
| `0x0002` | `HELLO_REPLY` | device unicast to controller |
| `0x0003` | `IDENTIFY` | controller request to one device |
| `0x0004` | `IDENTIFY_REPLY` | device identity response |

`HELLO_REPLY` and `IDENTIFY_REPLY` SHOULD include TLVs for:

- protocol version range
- device name
- vendor/project identity
- product code
- revision
- serial number
- primary MAC address
- supported services
- default profile ID
- current state
- active owner MAC, when any

Discovery MUST NOT change outputs, state, ownership, or retained configuration.

## 9. Management Service

LEAP-MGMT controls sessions and state. Only one controller may own the active
control lease for a device at a time.

| Message Type | Name | Purpose |
| ---: | --- | --- |
| `0x0001` | `OPEN_SESSION` | request a session and optional owner lease |
| `0x0002` | `OPEN_SESSION_REPLY` | assigns `session_id` and lease parameters |
| `0x0003` | `CLOSE_SESSION` | releases session and optional owner lease |
| `0x0004` | `HEARTBEAT` | maintains owner lease and reports sequence health |
| `0x0005` | `SET_STATE` | requests device state transition |
| `0x0006` | `STATE_REPLY` | reports accepted or current state |
| `0x0007` | `FAULT_RESET` | clears eligible protocol/application faults |
| `0x0008` | `OWNER_RELEASE` | explicitly places outputs in configured safe state |

### 9.1 Owner Lease

An owner lease is required before `LEAP-PD` can apply output data in `OP`.

- Lease duration is negotiated during `OPEN_SESSION`.
- Controller MUST refresh the lease with `HEARTBEAT` or accepted `LEAP-PD`
  traffic before the deadline.
- Device MUST transition to `SAFE` and apply configured safe outputs when the
  owner lease expires.
- A second controller MAY observe state and diagnostics, but MUST NOT write
  outputs until the first owner releases or times out.

### 9.2 State Machine

| State | Meaning | Output Behavior |
| --- | --- | --- |
| `BOOT` | firmware has not completed transport init | outputs held by application defaults |
| `INIT` | protocol ready, no active process profile | outputs held safe |
| `CONFIGURED` | directory and endpoint profile selected | outputs held safe |
| `SAFE` | owner may exchange data, outputs not applied or forced safe | safe outputs |
| `OP` | cyclic process data is active | owned outputs applied |
| `FAULT` | protocol or application fault active | fault policy outputs |

Required transitions:

- `BOOT -> INIT` after transport initialization.
- `INIT -> CONFIGURED` after a valid profile is selected or defaulted.
- `CONFIGURED -> SAFE` after owner lease is granted.
- `SAFE -> OP` only when endpoints validate and watchdog is armed.
- `OP -> SAFE` on owner release, heartbeat loss, low-level communication loss,
  or explicit safe request.
- Any state `-> FAULT` on unrecoverable protocol or application fault.
- `FAULT -> INIT` only through accepted `FAULT_RESET` and application approval.

Devices MUST reject unsupported transitions with `INVALID_STATE`.

## 10. Directory Service

LEAP-DIR replaces EtherCAT EEPROM and engineering XML with a live device
directory.

| Message Type | Name | Purpose |
| ---: | --- | --- |
| `0x0001` | `READ_DIRECTORY` | read identity, services, profiles, endpoints |
| `0x0002` | `READ_DIRECTORY_REPLY` | TLV list or fixed object response |
| `0x0003` | `READ_OBJECT` | read one object by ID and offset |
| `0x0004` | `READ_OBJECT_REPLY` | object bytes and metadata |
| `0x0005` | `WRITE_OBJECT` | write configurable object bytes |
| `0x0006` | `WRITE_OBJECT_REPLY` | accepted write range |
| `0x0007` | `SELECT_PROFILE` | choose a process-data profile |
| `0x0008` | `PROFILE_REPLY` | active profile and endpoint table |

### 10.1 Object ID Space

Object IDs are 32-bit values:

- high 16 bits: namespace
- low 16 bits: object number

Standard namespaces:

| Namespace | Meaning |
| ---: | --- |
| `0x0001` | identity and protocol information |
| `0x0002` | management configuration |
| `0x0003` | endpoint and profile metadata |
| `0x0004` | persistent device configuration |
| `0x0005` | diagnostics and counters |
| `0x8000..0xFFFE` | project/vendor extension |

### 10.2 Endpoint Descriptor

Each cyclic data window is described by an endpoint descriptor:

| Field | Size | Description |
| --- | ---: | --- |
| `endpoint_id` | `2` | stable endpoint identifier |
| `direction` | `1` | `1=controller_to_device`, `2=device_to_controller` |
| `flags` | `1` | fixed, optional, retained, safe-state flags |
| `profile_id` | `4` | profile that owns this endpoint |
| `byte_length` | `2` | endpoint payload size |
| `alignment` | `1` | required byte alignment |
| `reserved` | `1` | send `0` |
| `schema_object_id` | `4` | object describing field layout |

Endpoint descriptors are the LEAP replacement for SyncManager and FMMU setup.
The controller reads descriptors and sends endpoint IDs directly; the device does
not need a controller-programmed logical mapping step.

## 11. Process Data Service

LEAP-PD is the cyclic data path. It is valid only for the session owner after the
device reaches `SAFE`; output application is valid only in `OP`.

| Message Type | Name | Purpose |
| ---: | --- | --- |
| `0x0001` | `WRITE_ENDPOINT` | write one controller-to-device endpoint |
| `0x0002` | `READ_ENDPOINT` | request one device-to-controller endpoint |
| `0x0003` | `ENDPOINT_DATA` | response containing endpoint data |
| `0x0004` | `EXCHANGE_ENDPOINTS` | write command endpoint and read status endpoint |
| `0x0005` | `EXCHANGE_REPLY` | combined exchange response |

### 11.1 Endpoint Data Header

LEAP-PD payloads begin with:

| Offset | Size | Field | Description |
| ---: | ---: | --- | --- |
| `0` | `2` | `endpoint_id` | endpoint being accessed |
| `2` | `2` | `endpoint_offset` | byte offset within endpoint |
| `4` | `2` | `data_length` | bytes in this operation |
| `6` | `2` | `endpoint_flags` | service-specific flags |
| `8` | `4` | `process_sequence` | application-level sequence |
| `12` | `4` | `cycle_time_us` | requested or measured cycle time |
| `16` | `N` | `data` | endpoint data |

### 11.2 Combined Exchange

`EXCHANGE_ENDPOINTS` is the LEAP successor to the useful EtherCAT LRW pattern.
It writes a command endpoint and obtains a status endpoint in the same
transaction.

The request payload is:

| Offset | Size | Field | Description |
| ---: | ---: | --- | --- |
| `0` | `2` | `write_endpoint_id` | command endpoint |
| `2` | `2` | `read_endpoint_id` | status endpoint |
| `4` | `2` | `write_length` | command bytes following header |
| `6` | `2` | `read_length` | requested status bytes |
| `8` | `4` | `process_sequence` | command sequence |
| `12` | `4` | `cycle_time_us` | requested cycle period, or `0` |
| `16` | `write_length` | `write_data` | command image |
| `16 + write_length` | `read_length` | `read_reservation` | zero-filled by sender |

The response payload preserves the same layout and writes status bytes into the
`read_reservation` range. This makes the output/input offset explicit and keeps
the current successful LRW rule: output bytes come first, input bytes are written
after the output bytes, never at payload byte `0` unless the input range starts
there by definition.

### 11.3 Cyclic Reliability Rules

- `sequence` detects duplicate Ethernet frames.
- `process_sequence` detects repeated or skipped application cycles.
- A device MAY drop duplicate `LEAP-PD` frames after re-sending the previous
  response.
- Cyclic `LEAP-PD` traffic SHOULD NOT block waiting for retransmission; the next
  cycle is the recovery path.
- Configuration and management traffic SHOULD use explicit responses and retry
  with bounded backoff.

### 11.4 Working Counter Replacement

EtherCAT working counters are replaced by explicit status:

- frame-level success or error response
- accepted endpoint ID
- accepted byte range
- owner/session validation result
- latest `process_sequence` consumed
- profile-specific status fields

Controllers MUST treat missing responses, stale `process_sequence`, low-level
CRC failures, or `ERROR` responses as communication faults for watchdog purposes.

## 12. Diagnostics Service

LEAP-DIAG provides enough runtime evidence for tuning and fault isolation.

| Message Type | Name | Purpose |
| ---: | --- | --- |
| `0x0001` | `READ_COUNTERS` | read protocol counters |
| `0x0002` | `COUNTERS_REPLY` | counter payload |
| `0x0003` | `READ_TIMING` | read loop and transport timing |
| `0x0004` | `TIMING_REPLY` | timing payload |
| `0x0005` | `READ_EVENTS` | read event log entries |
| `0x0006` | `EVENTS_REPLY` | event log payload |
| `0x0007` | `TRACE_MARK` | insert a timestamped trace marker |

Minimum counters:

- received frames accepted
- received frames rejected
- transmitted frames accepted
- transmitted frames dropped
- CRC failures
- bad length failures
- unsupported service/message failures
- duplicate sequences
- lease expirations
- state transition rejects
- process cycles accepted
- process cycles missed

## 13. Watchdog And Safe Output Rules

Each device profile MUST define:

- power-up output defaults
- safe output values
- fault output values
- owner lease timeout
- process-data timeout
- whether inputs remain readable in `SAFE` and `FAULT`

For the ClearCore I/O profile, output levels MUST NOT be applied unless:

- the controller owns the lease
- the device is in `OP`
- the command endpoint length and profile ID match the selected profile
- the latest frame passed integrity checks
- the process watchdog is not expired

On timeout, the device MUST stop applying stale outputs and transition to `SAFE`
or `FAULT` according to the profile fault policy.

## 14. Fragmentation

Fragmentation is optional and SHOULD NOT be used for cyclic process data.

When `FRAGMENTED` is set, payload begins with:

| Offset | Size | Field | Description |
| ---: | ---: | --- | --- |
| `0` | `4` | `fragment_group_id` | reassembly key |
| `4` | `2` | `fragment_index` | zero-based fragment index |
| `6` | `2` | `fragment_count` | total fragments |
| `8` | `4` | `total_length` | complete unfragmented payload bytes |
| `12` | `4` | `total_crc32c` | CRC-32C over complete payload |

Receivers MUST bound memory use and MUST discard incomplete fragment groups when
their reassembly timer expires.

## 15. Security Model

LEAP draft security assumes an isolated machine network. This is not enough for
routed, shared, or hostile networks.

Initial requirements:

- devices ignore output writes from non-owner sessions
- owner lease limits stale control
- discovery has no side effects
- malformed frames are rejected without assertions or crashes
- diagnostics expose owner identity and timeout counters

Future security extensions SHOULD consider authenticated sessions, frame message
authentication codes, role-based write access, secure provisioning, and replay
protection across device restarts.

## 16. Versioning And Extension Rules

- Major version changes may break wire compatibility.
- Minor version changes must preserve the common header and existing service
  semantics.
- Unknown service IDs below `0x8000` are rejected.
- Unknown TLVs with valid lengths are skipped.
- Reserved bits are sent as `0` and ignored on receive.
- Profile IDs are immutable once published. New incompatible field layouts require
  a new profile ID.

## 17. Minimum Conformance Targets

A LEAP device implementation is minimally conformant when it supports:

- header validation
- discovery `HELLO` and `IDENTIFY`
- management `OPEN_SESSION`, `HEARTBEAT`, `SET_STATE`, `CLOSE_SESSION`
- directory read of identity, supported services, active profile, and endpoints
- process-data `EXCHANGE_ENDPOINTS`
- diagnostics counters
- owner lease timeout to safe outputs
- deterministic rejection of malformed frames

A LEAP controller implementation is minimally conformant when it supports:

- discovery
- session ownership
- state transition to `OP`
- cyclic exchange with sequence tracking
- timeout handling
- device directory parsing
- diagnostics readback on error

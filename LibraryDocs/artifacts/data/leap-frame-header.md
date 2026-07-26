---
title: LEAP frame header summary
component: leap-protocol-spec
level: library
reuse: medium
topics:
  - LEAP
  - frame header
  - EtherType
status: verified
---

<!-- EXCERPT — source: docs/ClearCore_Link_Layer_Protocol/LEAP_SPECIFICATION.md -->
<!-- EVIDENCE: E1 | symbol: LEAP frame header | lines: section 5 -->

# LEAP frame header (summary)

| Offset | Size | Field |
| ---: | ---: | --- |
| 0 | 4 | magic = ASCII `LEAP` |
| 4 | 1 | version_major |
| 5 | 1 | version_minor |
| 6 | 1 | header_length (32) |
| 7 | 1 | flags |
| 8 | 2 | service_id |
| 10 | 2 | message_type |
| 12 | 4 | session_id |
| 16 | 4 | sequence |
| 20 | 4 | ack_sequence |
| 24 | 2 | payload_length |
| 26 | 2 | header_crc16 |
| 28 | 4 | payload_crc32c |

Services: LEAP-MGMT `0x0001`, LEAP-DISC `0x0002`, LEAP-DIR `0x0003`, LEAP-PD `0x0010`, LEAP-DIAG `0x0020`.

## Source evidence

| Claim | Evidence | Level |
|-------|----------|-------|
| Header layout | `docs/ClearCore_Link_Layer_Protocol/LEAP_SPECIFICATION.md` section 5 | E1 |
| Magic ASCII LEAP | same | E1 |

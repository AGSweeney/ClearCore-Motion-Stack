# Validation and Hardening Checklist

## Protocol Validation

- Run `MotionBenchProtocolSmokeTest` after build to validate:
  - CIP path encoding for 8-bit and 16-bit segment forms.
  - Type encode/decode helpers used by `ClearLinkObjectMap`.
  - Presence of required canonical object keys.

## Runtime Validation

- Verify connect/disconnect behavior against a real ClearLink device.
- Verify poll rates at `250 ms` and `50 ms` do not destabilize the session.
- Verify monitor updates for:
  - Identity (`serial`, `MAC`, board mode, supply voltage)
  - CCIO state (`board_count`, `enabled`, 64-bit IO map)
  - Selected motor telemetry and status/alert bitfields
  - M-connector telemetry values

## Command Safety Validation

- Confirm writes are constrained to explicit map keys and valid instances.
- Confirm read-only attributes are rejected by write APIs.
- Confirm disconnected write attempts fail with clear error messages.
- Confirm motor instance selection is clamped to `1..4`.

## Deployment Validation

- Build in Debug and Release.
- Confirm `windeployqt` executes post-build.
- Run:

```powershell
powershell -ExecutionPolicy Bypass -File MotionBench/scripts/Validate-QtDeploy.ps1 -BinaryDirectory MotionBench/build
```

- Ensure required runtime DLLs/plugins are present beside the executable.

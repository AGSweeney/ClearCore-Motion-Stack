# MotionBench Architecture

`MotionBench` is a Qt Quick/QML Win32 application with a native C++ EtherNet/IP explicit messaging stack.

## Layers

- `src/protocol`
  - `EtherNetIpClient`: Encapsulation session management and explicit message transport.
  - `CipPath`: Encodes CIP logical paths for class/instance/attribute access.
- `src/discovery`
  - `ListIdentityService`: UDP ListIdentity discovery helper.
- `src/device`
  - `ClearLinkObjectMap`: Canonical object/attribute map and typed encode/decode helpers.
  - `DeviceService`: Application-facing service for connect/disconnect, polling, reads, and writes.
- `src/ui/viewmodels`
  - `AppViewModel`: Root view model exposed to QML.
- `qml`
  - Main UI including discovery, connection controls, motor/CCIO monitoring, and commands.

## Data Flow

1. QML invokes actions on `AppViewModel` and `DeviceService`.
2. `DeviceService` maps user intent to object keys from `ClearLinkObjectMap`.
3. `DeviceService` calls `EtherNetIpClient` for `Get_Attribute_Single` / `Set_Attribute_Single`.
4. Responses are decoded and merged into `monitorData`.
5. QML updates automatically via property notifications.

## Polling Strategy

- Poll timer defaults to `250 ms` and is configurable.
- Polling fetches identity, CCIO, selected motor telemetry, and M-connector telemetry.
- Reads happen through a constrained object map to keep behavior deterministic and auditable.

## Deployment Strategy

- Build includes `cmake/DeployQt.cmake`.
- `setup_qt_deploy(MotionBenchApp)` wires a post-build deployment step.
- Uses `windeployqt` (`WinQTDeploy`) to place required runtime DLLs/plugins beside the executable.

# MotionBench

MotionBench is a Qt Quick/QML Win32 application for monitoring and commanding ClearLink over EtherNet/IP explicit messaging, without `EEIP.dll`.

## Build Requirements

- CMake 3.21+
- Qt 6.5+ (`Core`, `Quick`, `Qml`, `Network`)
- MSVC toolchain on Windows

## Configure and Build

```powershell
cmake -S MotionBench -B MotionBench/build -G "Visual Studio 17 2022" -A x64
cmake --build MotionBench/build --config Release
```

## Runtime Deployment (WinQTDeploy / windeployqt)

- Build wiring is in `MotionBench/cmake/DeployQt.cmake`.
- Post-build step runs `windeployqt` to stage required Qt DLLs/plugins/QML runtime beside the application.
- To override auto-discovery, pass:

```powershell
cmake -S MotionBench -B MotionBench/build -DWINQTDEPLOY_EXECUTABLE="C:/Qt/6.8.0/msvc2022_64/bin/windeployqt.exe"
```

## Notes

- Canonical object map is documented in `MotionBench/docs/CLEARLINK_OBJECT_MAP.md`.
- High-level architecture is documented in `MotionBench/docs/ARCHITECTURE.md`.
- Validation and hardening checklist is in `MotionBench/docs/VALIDATION.md`.

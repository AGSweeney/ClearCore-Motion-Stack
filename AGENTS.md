## Learned User Preferences

- User wants firmware templates scaffolded from `ExternalReferances/Motoman_EnIP_ToolChanger/DX200_ToolChanger`.
- User expects firmware projects to remain flashable via the DX200-style custom programming tool flow.
- User expects firmware template `.cppproj` flashing commands to match the DX200 `$(MSBuildProjectDirectory)\..\Tools\flash_clearcore.cmd` pattern.
- User does not want external referenced library projects changed unless explicitly requested.

## Learned Workspace Facts

- Active firmware target is `ProjectTemplate/DX200_ToolChanger`.
- Shared external dependencies are `ExternalReferances/EnhancedClearCoreLibrary/libClearCore`, `ExternalReferances/EnhancedClearCoreLibrary/LwIP`, and `ExternalReferances/OpENer-Enhanced`.
- DX200 template `main.cpp` is intended to stay minimal: Ethernet bring-up, DHCP/static fallback, `opener_init()`, and periodic `opener_cyclic()`.
- DX200 template has `diablo16`, `toolchanger_hmi`, and `genie` components removed from source and build manifests.
- Template flash entrypoint is `ProjectTemplate/Tools/flash_clearcore.cmd`, which forwards to `ExternalReferances/EnhancedClearCoreLibrary/Tools/flash_clearcore.cmd`.

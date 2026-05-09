<#
Copyright (c) 2026 Adam G. Sweeney
SPDX-License-Identifier: MIT

Contributors: 2026 Adam G. Sweeney <agsweeney@gmail.com> - MotionBench / ClearCore Motion Stack

File: Validate-QtDeploy.ps1
Purpose: Verify windeployqt output contains required Qt DLLs and platform plugin.
#>
param(
    [Parameter(Mandatory = $true)]
    [string]$BinaryDirectory,
    [string]$ExecutableName = "MotionBenchApp.exe"
)

$exePath = Join-Path $BinaryDirectory $ExecutableName
if (-not (Test-Path $exePath)) {
    Write-Error "Executable not found: $exePath"
    exit 1
}

$requiredDlls = @(
    "Qt6Core.dll",
    "Qt6Gui.dll",
    "Qt6Qml.dll",
    "Qt6Network.dll",
    "Qt6Quick.dll",
    "Qt6QuickControls2.dll"
)

$missing = @()
foreach ($dll in $requiredDlls) {
    $dllPath = Join-Path $BinaryDirectory $dll
    if (-not (Test-Path $dllPath)) {
        $missing += $dll
    }
}

$platformsPlugin = Join-Path $BinaryDirectory "platforms\qwindows.dll"
if (-not (Test-Path $platformsPlugin)) {
    $missing += "platforms/qwindows.dll"
}

if ($missing.Count -gt 0) {
    Write-Error "Qt deployment incomplete. Missing: $($missing -join ', ')"
    exit 1
}

Write-Host "Qt deployment validation passed for $exePath"

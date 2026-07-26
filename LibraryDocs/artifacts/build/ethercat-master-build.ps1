// EXCERPT — source: EtherCATMaster/build.ps1
// EVIDENCE: E1 | symbol: build.ps1 | lines: 1-40
[CmdletBinding()]
param(
    [ValidateSet("Debug", "Release", "RelWithDebInfo", "MinSizeRel")]
    [string]$Configuration = "Release",

    [string]$BuildDir = "build",
    [string]$InstallDir = "out",
    [string]$QtRoot = "",

    [switch]$Clean,
    [switch]$SkipDeploy,
    [switch]$NoFetchSoem
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

function Write-Section {
    param([string]$Title)
    Write-Host "`n=== $Title ===" -ForegroundColor Cyan
}

function Get-VsInstallPath {
    $vsWhere = Join-Path ${env:ProgramFiles(x86)} "Microsoft Visual Studio\Installer\vswhere.exe"
    if (-not (Test-Path $vsWhere)) {
        throw "vswhere.exe was not found. Install Visual Studio 2022."
    }

    $path = & $vsWhere -latest -products * -requires Microsoft.Component.MSBuild -property installationPath
    if (-not $path) {
        throw "Visual Studio installation not found via vswhere."
    }
    return $path.Trim()
}

function Get-CMakePath {
    param([string]$VsInstallPath)

    $vsCmake = Join-Path $VsInstallPath "Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe"
    if (Test-Path $vsCmake) {

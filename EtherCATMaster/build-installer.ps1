[CmdletBinding()]
param(
    [switch]$SkipAppBuild,
    [ValidateSet("Debug", "Release", "RelWithDebInfo", "MinSizeRel")]
    [string]$Configuration = "Release",
    [string]$InnoSetupCompiler = ""
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

function Get-InnoSetupCompiler {
    param([string]$ExplicitPath)

    if ($ExplicitPath) {
        if (Test-Path $ExplicitPath) {
            return $ExplicitPath
        }
        throw "Inno Setup compiler not found at: $ExplicitPath"
    }

    $pathCandidate = Get-Command ISCC.exe -ErrorAction SilentlyContinue
    if ($pathCandidate) {
        return $pathCandidate.Source
    }

    $candidates = @(
        "${env:ProgramFiles(x86)}\Inno Setup 6\ISCC.exe",
        "$env:ProgramFiles\Inno Setup 6\ISCC.exe"
    )
    foreach ($candidate in $candidates) {
        if (Test-Path $candidate) {
            return $candidate
        }
    }

    throw "ISCC.exe was not found. Install Inno Setup 6 or pass -InnoSetupCompiler."
}

$projectRoot = Split-Path -Parent $MyInvocation.MyCommand.Path
$installerScript = Join-Path $projectRoot "installer\EtherCATMaster.iss"
$payloadExe = Join-Path $projectRoot "out\bin\EtherCATMasterQt.exe"

if (-not $SkipAppBuild) {
    & (Join-Path $projectRoot "build.ps1") -Configuration $Configuration -Clean
}

if (-not (Test-Path $payloadExe)) {
    throw "Installer payload is missing. Build the app first: $payloadExe"
}

$iscc = Get-InnoSetupCompiler -ExplicitPath $InnoSetupCompiler
Write-Host "Inno Setup: $iscc"
& $iscc $installerScript

$installerPath = Join-Path $projectRoot "installer\output\ClearCore_EtherCAT_Master_Setup.exe"
if (-not (Test-Path $installerPath)) {
    throw "Installer build completed without expected output: $installerPath"
}

Write-Host "Installer: $installerPath"

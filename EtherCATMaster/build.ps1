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
        return $vsCmake
    }

    $cmakeCmd = Get-Command cmake -ErrorAction SilentlyContinue
    if ($cmakeCmd) {
        return $cmakeCmd.Source
    }

    throw "cmake.exe was not found in Visual Studio or PATH."
}

function Resolve-QtRootCandidates {
    param([string]$ExplicitQtRoot)

    $roots = New-Object System.Collections.Generic.List[string]

    if ($ExplicitQtRoot) {
        $roots.Add($ExplicitQtRoot)
    }

    foreach ($candidate in @($env:QT_ROOT, $env:QTDIR, "C:\Qt", "D:\Qt", "$env:USERPROFILE\Qt", "C:\Program Files\Qt", "C:\Program Files (x86)\Qt")) {
        if ($candidate) {
            $roots.Add($candidate)
        }
    }

    $existing = $roots | Where-Object { Test-Path $_ } | Select-Object -Unique
    return $existing
}

function Get-WinDeployQtPath {
    param([string[]]$QtRoots)

    $fromPath = Get-Command windeployqt -ErrorAction SilentlyContinue
    if ($fromPath) {
        return $fromPath.Source
    }

    $allCandidates = New-Object System.Collections.Generic.List[string]
    foreach ($qtRoot in $QtRoots) {
        $hits = Get-ChildItem -Path $qtRoot -Filter windeployqt.exe -Recurse -ErrorAction SilentlyContinue
        foreach ($hit in $hits) {
            $allCandidates.Add($hit.FullName)
        }
    }

    if ($allCandidates.Count -eq 0) {
        throw "windeployqt.exe not found. Provide -QtRoot or set QT_ROOT/QTDIR."
    }

    $preferred = $allCandidates |
    ForEach-Object {
        $version = [version]"0.0.0"
        if ($_ -match "[\\/](\d+\.\d+\.\d+)[\\/]") {
            try {
                $version = [version]$Matches[1]
            }
            catch {
                $version = [version]"0.0.0"
            }
        }

        $kitRank = 3
        if ($_ -match "msvc2022_64") { $kitRank = 0 }
        elseif ($_ -match "msvc.*_64") { $kitRank = 1 }
        elseif ($_ -match "mingw") { $kitRank = 2 }

        [PSCustomObject]@{
            Path    = $_
            KitRank = $kitRank
            Version = $version
        }
    } |
    Sort-Object KitRank, @{ Expression = "Version"; Descending = $true }, @{ Expression = "Path"; Descending = $true } |
    Select-Object -First 1

    return $preferred.Path
}

function Get-QtPrefixFromDeployTool {
    param([string]$WinDeployQtPath)

    $binDir = Split-Path -Parent $WinDeployQtPath
    return Split-Path -Parent $binDir
}

function Copy-NpcapRuntime {
    param([string]$DestinationDir)

    $requiredDlls = @("wpcap.dll", "Packet.dll")
    $candidateRoots = @(
        "C:\Windows\System32\Npcap",
        "C:\Windows\System32",
        "C:\Windows\SysWOW64\Npcap",
        "C:\Windows\SysWOW64"
    )

    foreach ($dllName in $requiredDlls) {
        $sourcePath = $null
        foreach ($root in $candidateRoots) {
            $candidate = Join-Path $root $dllName
            if (Test-Path $candidate) {
                $sourcePath = $candidate
                break
            }
        }

        if (-not $sourcePath) {
            Write-Warning "$dllName was not found in expected Npcap locations. App may fail to start."
            continue
        }

        $destinationPath = Join-Path $DestinationDir $dllName
        Copy-Item -Path $sourcePath -Destination $destinationPath -Force
        Write-Host "Copied Npcap runtime: $dllName"
    }
}

$repoRoot = Split-Path -Parent $MyInvocation.MyCommand.Path
$buildPath = Join-Path $repoRoot $BuildDir
$installPath = Join-Path $repoRoot $InstallDir

if ($Clean) {
    Write-Section "Clean"
    if (Test-Path $buildPath) {
        Remove-Item -Recurse -Force $buildPath
    }
    if (Test-Path $installPath) {
        Remove-Item -Recurse -Force $installPath
    }
}

Write-Section "Tool Discovery"
$vsInstallPath = Get-VsInstallPath
$cmakePath = Get-CMakePath -VsInstallPath $vsInstallPath
$env:VCINSTALLDIR = Join-Path $vsInstallPath "VC\"
$qtRoots = Resolve-QtRootCandidates -ExplicitQtRoot $QtRoot
$winDeployQt = Get-WinDeployQtPath -QtRoots $qtRoots
$qtPrefix = Get-QtPrefixFromDeployTool -WinDeployQtPath $winDeployQt

Write-Host "Visual Studio: $vsInstallPath"
Write-Host "CMake:         $cmakePath"
Write-Host "Qt Prefix:     $qtPrefix"
Write-Host "WinDeployQt:   $winDeployQt"

Write-Section "Configure"
$fetchSoem = if ($NoFetchSoem) { "OFF" } else { "ON" }
$configureArgs = @(
    "-S", $repoRoot,
    "-B", $buildPath,
    "-G", "Visual Studio 17 2022",
    "-A", "x64",
    "-DCMAKE_PREFIX_PATH=$qtPrefix",
    "-DCMAKE_INSTALL_PREFIX=$installPath",
    "-DEC_MASTER_FETCH_SOEM=$fetchSoem"
)
& $cmakePath @configureArgs

Write-Section "Build + Install"
& $cmakePath --build $buildPath --config $Configuration --target install

$exePath = Join-Path $installPath "bin\EtherCATMasterQt.exe"
if (-not (Test-Path $exePath)) {
    throw "Built executable not found at $exePath"
}

if (-not $SkipDeploy) {
    Write-Section "Deploy Qt Runtime"
    $deployArgs = @("--no-translations", "--compiler-runtime")
    if ($Configuration -eq "Debug") {
        $deployArgs += "--debug"
    }
    else {
        $deployArgs += "--release"
    }
    $deployArgs += $exePath
    & $winDeployQt @deployArgs
}
else {
    Write-Section "Deploy Skipped"
}

Write-Section "Deploy Npcap Runtime"
Copy-NpcapRuntime -DestinationDir (Split-Path -Parent $exePath)

Write-Section "Complete"
Write-Host "Executable: $exePath"

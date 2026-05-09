@echo off
REM ---------------------------------------------------------------------------
REM Copyright (c) 2026 Adam G. Sweeney
REM SPDX-License-Identifier: MIT
REM
REM Contributors:
REM   2026 Adam G. Sweeney <agsweeney@gmail.com> - MotionBench original implementation
REM
REM File: build_vs.cmd
REM Purpose: Developer helper to configure and build MotionBench with Visual Studio.
REM ---------------------------------------------------------------------------
setlocal EnableExtensions EnableDelayedExpansion

set "SCRIPT_DIR=%~dp0"
set "SOURCE_DIR=%SCRIPT_DIR:~0,-1%"
set "BUILD_DIR=%SOURCE_DIR%\build-vs"
set "GENERATOR=Visual Studio 17 2022"
set "ARCH=x64"
set "CONFIG=Release"

if not "%~1"=="" set "CONFIG=%~1"

if /I not "%CONFIG%"=="Debug" if /I not "%CONFIG%"=="Release" if /I not "%CONFIG%"=="RelWithDebInfo" if /I not "%CONFIG%"=="MinSizeRel" (
    echo [ERROR] Invalid build config "%CONFIG%".
    echo         Use one of: Debug, Release, RelWithDebInfo, MinSizeRel
    exit /b 1
)

set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
if not exist "!VSWHERE!" (
    echo [ERROR] vswhere.exe not found at:
    echo         !VSWHERE!
    echo         Install Visual Studio Installer components.
    exit /b 1
)

for /f "usebackq delims=" %%I in (`"!VSWHERE!" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath`) do (
    set "VS_INSTALL=%%I"
)

if "%VS_INSTALL%"=="" (
    echo [ERROR] Could not locate a Visual Studio C++ installation.
    exit /b 1
)

if exist "!VS_INSTALL!\Common7\Tools\VsDevCmd.bat" (
    call "!VS_INSTALL!\Common7\Tools\VsDevCmd.bat" -arch=%ARCH% -host_arch=%ARCH%
    if errorlevel 1 (
        echo [ERROR] Failed to initialize Visual Studio developer environment.
        exit /b 1
    )
)

where cmake >nul 2>&1
if errorlevel 1 (
    if exist "!VS_INSTALL!\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe" (
        set "PATH=!VS_INSTALL!\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin;%PATH%"
    )
)

where cmake >nul 2>&1
if errorlevel 1 (
    echo [ERROR] cmake was not found after Visual Studio environment setup.
    echo         Install CMake via Visual Studio Installer or add cmake to PATH.
    exit /b 1
)

if not exist "%BUILD_DIR%" (
    mkdir "%BUILD_DIR%"
)

if "%Qt6_DIR%"=="" (
    if exist "C:\Qt\6.8.3\msvc2022_64\lib\cmake\Qt6\Qt6Config.cmake" (
        set "Qt6_DIR=C:\Qt\6.8.3\msvc2022_64\lib\cmake\Qt6"
        echo [INFO] Using detected Qt6_DIR=!Qt6_DIR!
    )
)

echo [INFO] Configuring CMake project...
if "%Qt6_DIR%"=="" (
    cmake -S "%SOURCE_DIR%" -B "%BUILD_DIR%" -G "%GENERATOR%" -A %ARCH%
) else (
    cmake -S "%SOURCE_DIR%" -B "%BUILD_DIR%" -G "%GENERATOR%" -A %ARCH% -DQt6_DIR="%Qt6_DIR%"
)
if errorlevel 1 (
    echo [ERROR] CMake configure failed.
    exit /b 1
)

echo [INFO] Building %CONFIG%...
cmake --build "%BUILD_DIR%" --config %CONFIG%
if errorlevel 1 (
    echo [ERROR] Build failed.
    exit /b 1
)

echo [OK] Build completed successfully.
echo      Output: "%BUILD_DIR%"
exit /b 0

<#
Copyright (c) 2026 Adam G. Sweeney
SPDX-License-Identifier: MIT

Contributors: 2026 Adam G. Sweeney <agsweeney@gmail.com> - ClearCore Motion Stack

File: setup-git-hooks.ps1
Purpose: Configure git core.hooksPath to .githooks (Windows).
#>
$ErrorActionPreference = "Stop"

git config core.hooksPath .githooks

Write-Host "Configured git hooks path to .githooks"
Write-Host "Pre-commit formatting enforcement is now active."

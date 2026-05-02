$ErrorActionPreference = "Stop"

git config core.hooksPath .githooks

Write-Host "Configured git hooks path to .githooks"
Write-Host "Pre-commit formatting enforcement is now active."

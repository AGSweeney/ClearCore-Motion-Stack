#!/usr/bin/env sh
# -----------------------------------------------------------------------------
# Copyright (c) 2026 Adam G. Sweeney
# SPDX-License-Identifier: MIT
#
# Contributors:
#   2026 Adam G. Sweeney <agsweeney@gmail.com> - ClearCore Motion Stack
#
# File: setup-git-hooks.sh
# Purpose: Configure git core.hooksPath to .githooks (POSIX).
# -----------------------------------------------------------------------------
set -eu

git config core.hooksPath .githooks
chmod +x .githooks/pre-commit

echo "Configured git hooks path to .githooks"
echo "Pre-commit formatting enforcement is now active."

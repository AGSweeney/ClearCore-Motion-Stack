#!/usr/bin/env sh
set -eu

git config core.hooksPath .githooks
chmod +x .githooks/pre-commit

echo "Configured git hooks path to .githooks"
echo "Pre-commit formatting enforcement is now active."

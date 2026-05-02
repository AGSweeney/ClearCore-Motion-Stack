# ClearCore Motion Stack Coding Style Rules

This document defines the coding style and contribution conventions for the ClearCore Motion Stack project.

These rules are based on the OpENer coding rules and Google C++ style guidance, with project-specific legal and attribution requirements.

## 1) Scope and Priority

- These rules apply to all project code and documentation unless a file-specific standard overrides them.
- For C/C++ style details not explicitly covered here, follow the Google C++ Style Guide.
- When conflicts occur:
  1. Project legal and attribution requirements in this document
  2. Project-specific coding rules in this document
  3. Google C++ style guidance

## 2) Licensing, Attribution, and Notices

- Use the MIT License for original project work where applicable.
- Include attribution to OpENer where code, design, object models, or behavior are derived from or based on OpENer.
- Original project work must include contributor/copyright notices naming:
  - Adam G. Sweeney `<agsweeney@gmail.com>`
- Do not remove existing third-party notices or license statements from imported or adapted files.
- Keep legal notices accurate and up to date when modifying files.

## 3) Required File Header

Each source file must include a header block with:
- Copyright line(s)
- License reference when applicable
- Contributor list
- Short file purpose

Recommended template:

```c
/******************************************************************************
 * Copyright (c) <year> Adam G. Sweeney
 * SPDX-License-Identifier: MIT
 *
 * Contributors:
 *   <date> Adam G. Sweeney <agsweeney@gmail.com> - <change summary>
 *
 * File: <filename>
 * Purpose: <short description>
 *
 * Attribution: Portions of this design/implementation are influenced by
 * OpENer (Open Source EtherNet/IP Adapter Stack), where applicable.
 ******************************************************************************/
```

## 4) Comments and Documentation

- Write comments in English.
- Prefer clear code over excessive comments, but always document non-obvious behavior and safety-critical logic.
- Remove stale or misleading comments immediately.
- Use Doxygen-style comments for public functions, structs, enums, and important globals.
- Do not rely on autobrief; use explicit `@brief`.
- Use inline Doxygen member comments (`/**< ... */`) for struct fields, enum members, and documented constants when useful.
- Use these work-tracking keywords consistently:
  - `TODO:` planned work or extension
  - `FIXME:` known bug or risky behavior that needs correction

## 5) Data Types

- Use fixed-width integer types (`uint8_t`, `int16_t`, etc.) where binary layout, protocol size, or wire compatibility matters.
- For EtherNet/IP and CIP protocol-facing code, prefer project protocol typedefs (`Eip*`, `Cip*`) where defined.
- Use plain `int`/`unsigned int` for local computation when exact width is not required and platform efficiency matters.
- Avoid implicit narrowing conversions; cast intentionally and document when needed.

## 6) Naming Conventions

Use English, self-explanatory identifiers. Avoid leading underscores for identifiers.

- Variables: `lower_snake_case`
- Global variables: prefix with `g_` (example: `g_connection_state`)
- Member/file-scope object variables that require suffixing: trailing `_` where applicable
- Constants (`const`): `kPascalCase`
- Macro constants (`#define`): `UPPER_SNAKE_CASE` (only when `const`/`constexpr` is not viable)
- Functions: `PascalCase`
- Struct/typedef names: `PascalCase`
- Enum type names: `PascalCase`
- Enum members: `k<EnumName><ValueName>` form where practical

## 7) Declarations and Initialization

- One variable declaration per line.
- Place pointer `*` with the variable name (example: `char *buffer`).
- Prefer initialization at declaration.
- Minimize variable scope; declare near first use.
- Avoid magic numbers; use named constants.

## 8) Function Design Rules

- Keep functions focused on one responsibility.
- Order parameters as:
  1. input parameters first
  2. output parameters last
- Mark input parameters `const` when not modified.
- Validate external inputs using normal control flow and explicit error handling.
- Return explicit status/error codes or booleans for expected runtime failures.

## 9) Structs and Enums

- Prefer anonymous `struct`/`enum` with `typedef` alias where it improves C compatibility and readability.
- Keep struct fields named using normal variable conventions.
- Keep enum values explicit when values are protocol-relevant.
- Document protocol-visible fields and enum values.

## 10) Formatting and Layout

- Use Google C++ formatting conventions as baseline.
- Keep consistent indentation and brace style across the file.
- Keep includes ordered and minimal.
- Prefer `clang-format`/formatter-driven consistency over manual formatting.
- Avoid unrelated formatting churn in functional commits.

## 11) Assertions and Error Handling

- Use assertions only for impossible states that indicate programmer bugs.
- Assertions must not be used to handle invalid external input, network payload issues, allocation failures, or other foreseeable runtime conditions.
- Handle foreseeable failures through normal runtime checks and error paths.
- Never allow malformed EtherNet/IP/CIP input to crash firmware via assertion.

## 12) Safety and Determinism Expectations

- Keep state transitions explicit and deterministic.
- Validate bounds, lengths, and object state before use.
- Preserve safe startup and fault behavior when modifying motion or communication paths.
- Ensure watchdog and connection-loss behavior remains robust.

## 13) Testing and Verification Expectations

- New behavior should include or update tests where feasible.
- Protocol-visible behavior changes must include validation against expected scanner/PLC interaction.
- Bug fixes should include regression coverage when practical.
- Keep diagnostics useful: return meaningful status and log enough context for root-cause analysis.

## 14) Contribution Checklist (Required Before Merge)

- File headers present and accurate
- License and attribution requirements satisfied
- Naming and formatting rules followed
- No magic numbers without named constants
- Doxygen comments added/updated for public interfaces
- Assertions used only for true programmer-error conditions
- Input validation and error handling implemented for external data
- Tests/docs updated for behavior changes

## 15) Enforcement and Tooling

Style compliance is enforced through editor defaults and a pre-commit hook.

- Repository formatting source of truth: `.clang-format`
- Workspace editor settings: `.vscode/settings.json`
- Optional editor recommendations: `.vscode/extensions.json`
- Commit-time formatting check: `.githooks/pre-commit`

Enable the local git hook path once per clone:

- PowerShell (Windows):
  - `./scripts/setup-git-hooks.ps1`
- POSIX shell (Linux/macOS/Git Bash):
  - `./scripts/setup-git-hooks.sh`

If a commit is blocked by formatting:

- Run `clang-format -i <file1> <file2> ...`
- Re-stage files with `git add`
- Retry commit


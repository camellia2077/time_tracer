---
description: Safe Clang-Tidy Refactoring Workflow
---

# Workflow: Clang-Tidy (Split Repo Structure)

## 0. Context
- **Root**: `c:/Computer/my_github/github_cpp/time_tracer/time_tracer_cpp` (Run commands from here)
- **Shell Prefix**: `C:\msys64\msys2_shell.cmd -ucrt64 -defterm -no-start -where . -c`

## 1. Pre-Check
- If `apps/time_tracer/build_tidy/tasks/` is empty or missing -> **Exit & Ask**.

## 2. Analyze
// turbo
`{Shell Prefix} "python3 ./apps/time_tracer/scripts/workflow/analyze_tasks.py <ID_RANGE>"`
- **Rule**: If unique files > 10, split batch.

## 3. Refactor & Verify
- **Impact**: `ripgrep` symbols across root before edit.
- **Action**: Fix warnings -> Run verification:
// turbo
`{Shell Prefix} "python3 ./apps/time_tracer/scripts/workflow/verify_batch.py <IDs>"`

## 4. Lifecycle
- **Success**: Cleanup logs automatically.
- **Fail**: Stop, read output, fix, then retry Step 3.
- **Pause**: Ask user every 25 tasks.
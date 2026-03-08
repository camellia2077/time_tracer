# Toolchain Architecture

This directory contains the build/test/tidy automation toolchain.

Quick links:
- `../../docs/toolchain/python_command_map.md` (change-routing map for humans/agents)
- `../../docs/toolchain/clang_tidy_architecture.md` (clang-tidy file architecture / change routing)
- `../../docs/toolchain/clang_tidy_flow.md` (clang-tidy workflow / state files)
- `../../docs/toolchain/clang_tidy_sop.md` (standard clang-tidy batch SOP)
- `../AGENTS.md` (agent editing rules in tools scope)

## Directory Structure

- `config/*.toml`: layered toolchain config sources.
  - Build profiles support `extends` inheritance in `config/build.toml`.
  - Windows CI matrix is generated from `[[build.ci.windows_matrix]]`.
- `core/`: **Infrastructure Layer**. Handles environment setup, process execution (with real-time feedback), and application registration.
- `services/`: **Logic Layer**. Pure functions for parsing diagnostics, planning rename candidates, and driving `clangd` LSP edits.
- `commands/`: **Workflow Layer**. Orchestrates multi-step processes like `configure`, `build`, `tidy`, `clean`, rename automation (`rename-plan`, `rename-apply`, `rename-audit`), and task-local clang-tidy automation (`tidy-task-patch`, `tidy-task-fix`, `tidy-task-suggest`, `tidy-step`).

## Key Design Principles

1. **Real-time Feedback**: All intensive processes (CMake, Clang-Tidy) must provide line-buffered real-time output to prevent perceived deadlocks.
2. **Context-Aware**: The `Context` object centralizes path management and environment setup.
3. **AI-Friendly**: Clearly defined boundaries between "How to run" (Core) and "What to do" (Commands) reduce the cognitive load for LLMs performing refactoring tasks.

## Usage

All commands are accessed via the root `tools/run.py`.

Core shell naming:
- Recommended semantic app name: `tracer_core_shell`
- Backward-compatible app id: `tracer_core`

```bash
# Example: Run tidy for time_tracer
python tools/run.py tidy --app tracer_core_shell

# Scoped tidy for core-family libs only
python tools/run.py tidy --app tracer_core_shell --source-scope core_family --build-dir build_tidy_core_family
python tools/run.py tidy-fix --app tracer_core_shell --source-scope core_family --tidy-build-dir build_tidy_core_family
bash apps/tracer_core_shell/scripts/run_clang_tidy_libs_core.sh check

# Configure and build are split commands
python tools/run.py configure --app tracer_core_shell
python tools/run.py build --app tracer_core_shell
python tools/run.py build --app tracer_core_shell --profile fast
python tools/run.py verify --app tracer_core_shell --profile fast --concise
python tools/run.py format --app tracer_core_shell

# Runtime lock cleanup (time_tracer_cli / native test EXEs holding core DLL) runs automatically on build for tracer_core_shell/tracer_core/tracer_windows_rust_cli.
# Build tool cleanup (cmake/ninja/ccache) remains opt-in.
python tools/run.py build --app tracer_core_shell --kill-build-procs

# Tune tidy parallelism
python tools/run.py tidy --app tracer_core_shell --jobs 16 --parse-workers 8

# Windows CLI build entry (default Rust / fallback C++)
bash apps/tracer_cli/windows/scripts/build_core_runtime_release.sh
bash apps/tracer_cli/windows/scripts/build_rust_from_windows_build.sh
bash apps/tracer_cli/windows/scripts/build_fast.sh

# Profile build targets (optional):
# - Configure in tools/toolchain/config/build.toml -> [build.profiles.<name>].build_targets
# - Applied only when user does not pass explicit --target in build extra args
# - Explicit user --target always takes precedence
# build_targets = ["tracer_core_shared"]

# Refresh report golden snapshots (optional pre-verify can be skipped)
# python tools/run.py refresh-golden --app tracer_core_shell --profile fast_ci_no_pch --build-dir build_fast

# Tidy header diagnostics scope (tools/toolchain/config/workflow.toml -> [tidy].header_filter_regex)
# Example: exclude build third-party deps under */_deps/*
# header_filter_regex = "^(?!.*[\\\\/]_deps[\\\\/]).*"

# Auto loop for rename-only tasks with periodic verify
python tools/run.py tidy-loop --app tracer_core_shell --n 10 --test-every 3 --concise
python tools/run.py tidy-loop --app tracer_core_shell --all --test-every 3 --concise

# Task-local clang-tidy automation (safe repetitive work)
python tools/run.py tidy-task-patch --app tracer_core_shell --source-scope core_family --tidy-build-dir build_tidy_core_family --batch-id 002 --task-id 011
python tools/run.py tidy-task-fix --app tracer_core_shell --source-scope core_family --tidy-build-dir build_tidy_core_family --batch-id 002 --task-id 011 --dry-run
python tools/run.py tidy-task-suggest --app tracer_core_shell --source-scope core_family --tidy-build-dir build_tidy_core_family --batch-id 002 --task-id 011
python tools/run.py tidy-step --app tracer_core_shell --source-scope core_family --tidy-build-dir build_tidy_core_family --batch-id 002 --task-id 011 --dry-run

# Run tools/toolchain minimal regression tests
python tools/run.py self-test

# Generate/refresh bundle metadata from historical config paths (default dry-run, C++ lane)
python tools/run.py config-migrate --app tracer_windows_rust_cli --show-diff
python tools/run.py config-migrate --app tracer_windows_rust_cli --apply
python tools/run.py config-migrate --app tracer_windows_rust_cli --rollback

# Build hooks auto-sync platform config when app declares `config_sync_target`
# (tracer_windows_rust_cli -> windows, tracer_android -> android).
# Sync implementation stays standalone in `tools/platform_config/run.py`.
# For CMake apps requiring windows sync args, build/configure auto-inject:
# `-DTRACER_WINDOWS_CONFIG_SOURCE_DIR=<generated-config-root>`.
# For Gradle apps (tracer_android), build auto-injects:
# `-PtimeTracerConfigRoot=<generated-config-root>`.
# Rust app
python tools/run.py build --app tracer_windows_rust_cli

# Android edit loop (fastest local path: build only, no verify suite)
python tools/run.py build --app tracer_android --profile android_edit

# Android validation loop (use only when you need checks/tests)
python tools/run.py verify --app tracer_android --profile android_style --concise
python tools/run.py verify --app tracer_android --profile android_ci --concise
python tools/run.py post-change --app tracer_android --run-tests always --concise

# Windows CLI quick validation (single command):
# `verify --app tracer_core_shell` maps to Windows CLI artifact build + test flow,
# and runs artifact checks automatically after build.
# `build_fast` here is a flexible CMake build dir example, not an Android/Gradle recommendation.
python tools/run.py verify --app tracer_core_shell --build-dir build_fast --concise

# Build and apply rename plan for naming warnings
python tools/run.py rename-plan --app tracer_core_shell
python tools/run.py rename-apply --app tracer_core_shell
python tools/run.py rename-audit --app tracer_core_shell
```

## Scoped clang-tidy workspace

- Default full-queue tidy workspace: `out/tidy/tracer_core_shell/build_tidy`
- Official clang-tidy input: `out/tidy/<app>/<tidy_workspace>/analysis_compile_db/compile_commands.json`
- Raw build `compile_commands.json` belongs to the build system only; it is not a clang-tidy contract
- Built-in source scope preset: `core_family`
  - `libs/tracer_core/src`
  - `libs/tracer_adapters_io/src`
  - `libs/tracer_core_bridge_common/src`
  - `libs/tracer_transport/src`
- Default scoped tidy workspace for `core_family`: `out/tidy/tracer_core_shell/build_tidy_core_family`
- `--source-scope` controls the selected C++ source roots
- `--tidy-build-dir` controls which tidy queue/state workspace is used by queue-management commands
- Task-local automation reports land under `out/tidy/<app>/<tidy_workspace>/automation/`
- Thin shell wrapper: `apps/tracer_core_shell/scripts/run_clang_tidy_libs_core.sh`
  - Always forwards `tracer_core_shell + core_family + build_tidy_core_family`
  - Does not own any filtering logic

## Result Visibility Contract

- State file (`post-change`): `out/build/<app>/<build_dir>/post_change_last.json`
  - Fixed-dir backend example: `out/build/tracer_android/build/post_change_last.json`
  - Step status (`configure/build/test`), failed stage, failed command, next action.
- Summary file: `out/test/<result_target>/result.json`
  - Overall success/failure and module summary.
- Case details file: `out/test/<result_target>/result_cases.json`
  - Per-case failure details.
- Aggregated log: `out/test/<result_target>/logs/output.log`
  - Key error lines for failure triage.
- Quality gate outputs: `out/test/<result_target>/quality_gates/`
  - Generated case snapshots plus markdown/triplet audit reports for `verify` and `refresh-golden`.
- Result target mapping:
  - `tracer_core` / `tracer_core_shell` / `tracer_windows_rust_cli` -> `artifact_windows_cli`
  - `tracer_android` -> `artifact_android`
  - `log_generator` -> `artifact_log_generator`
  - Unmapped apps keep `<result_target>=<app>`.

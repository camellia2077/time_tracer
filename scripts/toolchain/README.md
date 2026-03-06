# Toolchain Architecture

This directory contains the build/test/tidy automation toolchain.

Quick links:
- `../../docs/toolchain/python_command_map.md` (change-routing map for humans/agents)
- `../../docs/toolchain/clang_tidy_sop.md` (standard clang-tidy batch SOP)
- `../AGENTS.md` (agent editing rules in scripts scope)

## Directory Structure

- `config.toml` + `config/*.toml`: layered toolchain config sources.
  - Build profiles support `extends` inheritance in `config/build.toml`.
  - Windows CI matrix is generated from `[[build.ci.windows_matrix]]`.
- `core/`: **Infrastructure Layer**. Handles environment setup, process execution (with real-time feedback), and application registration.
- `services/`: **Logic Layer**. Pure functions for parsing diagnostics, planning rename candidates, and driving `clangd` LSP edits.
- `commands/`: **Workflow Layer**. Orchestrates multi-step processes like `configure`, `build`, `tidy`, `clean`, and rename automation (`rename-plan`, `rename-apply`, `rename-audit`).

## Key Design Principles

1. **Real-time Feedback**: All intensive processes (CMake, Clang-Tidy) must provide line-buffered real-time output to prevent perceived deadlocks.
2. **Context-Aware**: The `Context` object centralizes path management and environment setup.
3. **AI-Friendly**: Clearly defined boundaries between "How to run" (Core) and "What to do" (Commands) reduce the cognitive load for LLMs performing refactoring tasks.

## Usage

All commands are accessed via the root `scripts/run.py`.

Core shell naming:
- Recommended semantic app name: `tracer_core_shell`
- Backward-compatible app id: `tracer_core`

```bash
# Example: Run tidy for time_tracer
python scripts/run.py tidy --app tracer_core_shell

# Configure and build are split commands
python scripts/run.py configure --app tracer_core_shell
python scripts/run.py build --app tracer_core_shell
python scripts/run.py build --app tracer_core_shell --profile fast
python scripts/run.py verify --app tracer_core_shell --profile fast --concise
python scripts/run.py format --app tracer_core_shell

# Runtime lock cleanup (time_tracer_cli / native test EXEs holding core DLL) runs automatically on build for tracer_core_shell/tracer_core/tracer_windows_rust_cli.
# Build tool cleanup (cmake/ninja/ccache) remains opt-in.
python scripts/run.py build --app tracer_core_shell --kill-build-procs

# Tune tidy parallelism
python scripts/run.py tidy --app tracer_core_shell --jobs 16 --parse-workers 8

# Windows CLI build entry (default Rust / fallback C++)
bash apps/tracer_cli/windows/scripts/build_core_runtime_release.sh
bash apps/tracer_cli/windows/scripts/build_rust_from_windows_build.sh
bash apps/tracer_cli/windows/scripts/build_fast.sh

# Profile build targets (optional):
# - Configure in scripts/toolchain/config/build.toml -> [build.profiles.<name>].build_targets
# - Applied only when user does not pass explicit --target in build extra args
# - Explicit user --target always takes precedence
# build_targets = ["tracer_core_shared"]

# Refresh report golden snapshots (optional pre-verify can be skipped)
# python scripts/run.py refresh-golden --app tracer_core_shell --profile fast_ci_no_pch --build-dir build_fast

# Tidy header diagnostics scope (scripts/toolchain/config/workflow.toml -> [tidy].header_filter_regex)
# Example: exclude build third-party deps under */_deps/*
# header_filter_regex = "^(?!.*[\\\\/]_deps[\\\\/]).*"

# Auto loop for rename-only tasks with periodic verify
python scripts/run.py tidy-loop --app tracer_core_shell --n 10 --test-every 3 --concise
python scripts/run.py tidy-loop --app tracer_core_shell --all --test-every 3 --concise

# Run scripts/toolchain minimal regression tests
python scripts/run.py self-test

# Generate/refresh bundle metadata from historical config paths (default dry-run, C++ lane)
python scripts/run.py config-migrate --app tracer_windows_rust_cli --show-diff
python scripts/run.py config-migrate --app tracer_windows_rust_cli --apply
python scripts/run.py config-migrate --app tracer_windows_rust_cli --rollback

# Build hooks auto-sync platform config when app declares `config_sync_target`
# (tracer_windows_rust_cli -> windows, tracer_android -> android).
# Sync implementation stays standalone in `scripts/platform_config/run.py`.
# For CMake apps requiring windows sync args, build/configure auto-inject:
# `-DTRACER_WINDOWS_CONFIG_SOURCE_DIR=<generated-config-root>`.
# For Gradle apps (tracer_android), build auto-injects:
# `-PtimeTracerConfigRoot=<generated-config-root>`.
# Rust app
python scripts/run.py build --app tracer_windows_rust_cli

# Android edit loop (fastest local path: build only, no verify suite)
python scripts/run.py build --app tracer_android --profile android_edit

# Android validation loop (use only when you need checks/tests)
python scripts/run.py verify --app tracer_android --profile android_style --concise
python scripts/run.py verify --app tracer_android --profile android_ci --concise
python scripts/run.py post-change --app tracer_android --run-tests always --concise

# Windows CLI quick validation (single command):
# `verify --app tracer_core_shell` maps to Windows CLI artifact build + test flow,
# and runs artifact checks automatically after build.
# `build_fast` here is a flexible CMake build dir example, not an Android/Gradle recommendation.
python scripts/run.py verify --app tracer_core_shell --build-dir build_fast --concise

# Build and apply rename plan for naming warnings
python scripts/run.py rename-plan --app tracer_core_shell
python scripts/run.py rename-apply --app tracer_core_shell
python scripts/run.py rename-audit --app tracer_core_shell
```

## Result Visibility Contract

- State file (`post-change`): `apps/<app>/<build_dir>/post_change_last.json`
  - Fixed-dir backend example: `apps/tracer_android/build/post_change_last.json`
  - Step status (`configure/build/test`), failed stage, failed command, next action.
- Summary file: `test/output/<result_target>/result.json`
  - Overall success/failure and module summary.
- Case details file: `test/output/<result_target>/result_cases.json`
  - Per-case failure details.
- Aggregated log: `test/output/<result_target>/logs/output.log`
  - Key error lines for failure triage.
- Result target mapping:
  - `tracer_core` / `tracer_core_shell` / `tracer_windows_rust_cli` -> `artifact_windows_cli`
  - `tracer_android` -> `artifact_android`
  - `log_generator` -> `artifact_log_generator`
  - Unmapped apps keep `<result_target>=<app>`.


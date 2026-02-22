# Toolchain Architecture

This directory contains the build/test/tidy automation toolchain.

## Directory Structure

- `config.toml`: single source of toolchain command config.
- `core/`: **Infrastructure Layer**. Handles environment setup, process execution (with real-time feedback), and application registration.
- `services/`: **Logic Layer**. Pure functions for parsing diagnostics, planning rename candidates, and driving `clangd` LSP edits.
- `commands/`: **Workflow Layer**. Orchestrates multi-step processes like `configure`, `build`, `tidy`, `clean`, and rename automation (`rename-plan`, `rename-apply`, `rename-audit`).

## Key Design Principles

1. **Real-time Feedback**: All intensive processes (CMake, Clang-Tidy) must provide line-buffered real-time output to prevent perceived deadlocks.
2. **Context-Aware**: The `Context` object centralizes path management and environment setup.
3. **AI-Friendly**: Clearly defined boundaries between "How to run" (Core) and "What to do" (Commands) reduce the cognitive load for LLMs performing refactoring tasks.

## Usage

All commands are accessed via the root `scripts/run.py`.

```bash
# Example: Run tidy for time_tracer
python scripts/run.py tidy --app time_tracer

# Configure and build are split commands
python scripts/run.py configure --app time_tracer
python scripts/run.py build --app time_tracer
python scripts/run.py build --app time_tracer --profile fast
python scripts/verify.py --app time_tracer --profile fast --concise
python scripts/run.py format --app time_tracer

# Process cleanup is opt-in (single-project serial workflow is faster by default)
python scripts/run.py build --app time_tracer --kill-build-procs

# Tune tidy parallelism
python scripts/run.py tidy --app time_tracer --jobs 16 --parse-workers 8

# Auto loop for rename-only tasks with periodic verify
python scripts/run.py tidy-loop --app time_tracer --n 10 --test-every 3 --concise
python scripts/run.py tidy-loop --app time_tracer --all --test-every 3 --concise

# Run scripts/toolchain minimal regression tests
python scripts/run.py self-test

# Generate/refresh bundle metadata from legacy config (default dry-run)
python scripts/run.py config-migrate --app tracer_windows_cli --show-diff
python scripts/run.py config-migrate --app tracer_windows_cli --apply
python scripts/run.py config-migrate --app tracer_windows_cli --rollback

# Build hooks auto-sync platform config when app declares `config_sync_target`
# (tracer_windows_cli -> windows, tracer_android -> android).
# Sync implementation stays standalone in `scripts/platform_config/run.py`.
# For CMake apps (tracer_windows_cli), build/configure auto-inject:
# `-DTRACER_WINDOWS_CONFIG_SOURCE_DIR=<generated-config-root>`.
# For Gradle apps (tracer_android), build auto-injects:
# `-PtimeTracerConfigRoot=<generated-config-root>`.
python scripts/run.py build --app tracer_windows_cli
python scripts/run.py build --app tracer_android

# Windows CLI quick validation (single command):
# `verify --app time_tracer` maps to `tracer_windows_cli` suite and build target `tracer_windows_cli`,
# builds first, then runs `test/run.py --suite tracer_windows_cli`.
python scripts/verify.py --app time_tracer --build-dir build_fast --concise

# Build and apply rename plan for naming warnings
python scripts/run.py rename-plan --app time_tracer
python scripts/run.py rename-apply --app time_tracer
python scripts/run.py rename-audit --app time_tracer
```

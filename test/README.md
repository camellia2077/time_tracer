# test

Unified executable test workspace.

## Canonical structure

- `framework/`: shared Python testing framework and runner internals.
  - `framework/core/`: reusable testing engine package.
  - `framework/suite_runner.py`: suite execution adapter.
- `compat/`: legacy wrappers forwarding to unified runner.
  - `compat/time_tracer/`: `run.bat`, `run_fast.bat`, `run_agent.bat`
  - `compat/log_generator/`: `run.bat`, `run_fast.bat`, `run_agent.bat`
- `suites/tracer_windows_cli/`: core + Windows CLI integrated suite config (`apps/tracer_windows_cli` build target).
- `suites/log_generator/`: table-driven suite config for `apps/log_generator`.
- `data/`: shared input data (e.g. `data/dates`).
- `output/`: all generated runtime artifacts.
- `run.py`: unified entrypoint (`--suite tracer_windows_cli|log_generator|tracer_android`).
- `run_time_tracer_cli.bat`: scenario launcher (default `--with-build --build-dir build_fast`).
- `run_log_generator.bat`: scenario launcher (default test-only, auto-detect existing build dir).
- `run_runtime_guard.bat`: runtime bootstrap guard scenarios (missing core/config/reports dll).

## Output layering

Each suite writes into:

- `output/<suite>/workspace`: copied binaries and runtime workspace.
- `output/<suite>/logs`: per-case logs + python output log.
- `output/<suite>/artifacts`: generated report/output files.
- `output/<suite>/result.json`: machine-readable summary.
- Runner enforces this contract strictly; non-canonical result path overrides are ignored.

## Config path variables

Suite TOML files support `${repo_root}` and relative paths.

## Quick start

From `time_tracer_cpp/test`:

- `python run.py --suite tracer_windows_cli --build-dir build_fast --concise`
- `python run.py --suite log_generator --build-dir build_fast --concise`
- `python run.py --suite tracer_windows_cli --with-build --build-dir build_fast --agent --concise`
- `python run.py --suite log_generator --build-dir build_fast --agent --concise`
- `run_time_tracer_cli.bat --agent --concise`
- `run_log_generator.bat --agent --concise`
- `run_runtime_guard.bat --build-dir build_fast`
- `run_android_runtime_cpp_tests.bat`
  - Uses one command `python scripts/verify.py --app time_tracer --build-dir build_fast --concise`

## Java env for Android build (Windows)

For Android Gradle build in this workspace, use Android Studio bundled JBR:

- `JAVA_HOME=C:\Application\Android\as\jbr`

Validation (PowerShell):

```powershell
$env:JAVA_HOME
java -version
```

Example verified output:

```text
C:\Application\Android\as\jbr
openjdk version "21.0.9" 2025-10-21
OpenJDK Runtime Environment (build 21.0.9+-14649483-b1163.86)
OpenJDK 64-Bit Server VM (build 21.0.9+-14649483-b1163.86, mixed mode)
```

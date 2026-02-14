# test

Unified executable test workspace.

## Canonical structure

- `framework/`: shared Python testing framework and runner internals.
  - `framework/core/`: reusable testing engine package.
  - `framework/suite_runner.py`: suite execution adapter.
- `compat/`: legacy wrappers forwarding to unified runner.
  - `compat/time_tracer/`: `run.bat`, `run_fast.bat`, `run_agent.bat`
  - `compat/log_generator/`: `run.bat`, `run_fast.bat`, `run_agent.bat`
- `suites/time_tracer/`: table-driven suite config for `apps/time_tracer`.
- `suites/log_generator/`: table-driven suite config for `apps/log_generator`.
- `data/`: shared input data (e.g. `data/dates`).
- `output/`: all generated runtime artifacts.
- `run.py`: unified entrypoint (`--suite time_tracer|log_generator`).
- `run_time_tracer.bat`: scenario launcher (default `--with-build --build-dir build_fast`).
- `run_log_generator.bat`: scenario launcher (default test-only, auto-detect existing build dir).

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

- `python run.py --suite time_tracer --build-dir build_fast --concise`
- `python run.py --suite log_generator --build-dir build_fast --concise`
- `python run.py --suite time_tracer --with-build --build-dir build_fast --agent --concise`
- `python run.py --suite log_generator --build-dir build_fast --agent --concise`
- `run_time_tracer.bat --agent --concise`
- `run_log_generator.bat --agent --concise`

# Windows CLI Docs

## Purpose

Provide the minimal navigation map for the Windows Rust CLI implementation under
`apps/cli/windows/rust`.

## When To Open

- You are changing CLI command parsing or top-level command routing.
- You are changing Rust-side runtime bootstrap or C ABI host integration.
- You are changing report/query/exchange/pipeline command behavior.

## What This Doc Does Not Cover

- Core business logic inside `libs/tracer_core/**`
- Historical refactor notes
- Full command reference for every flag

## Start Here

1. CLI entry and command model:
   - `apps/cli/windows/rust/src/main.rs`
   - `apps/cli/windows/rust/src/cli/*.rs`
2. Command routing:
   - `apps/cli/windows/rust/src/commands/mod.rs`
   - `apps/cli/windows/rust/src/commands/handlers/`
3. Runtime host boundary:
   - `apps/cli/windows/rust/src/core/runtime.rs`
   - `apps/cli/windows/rust/src/core/runtime/*.rs`

## Command Families

- `pipeline`
  - source/processed data flows and validation
- `query`
  - semantic data queries and tree presentation
- `report`
  - textual render and export flows
- `exchange`
  - tracer exchange package export/import/inspect
- `chart`
  - report-chart HTML presenter
- utility
  - `doctor`, `licenses`, `tracer`, `motto`

## Canonical CLI Surface

- `query data`
- `query tree`
- `report render`
- `report export`
- `exchange export`
- `exchange import`
- `exchange inspect`
- `pipeline convert`
- `pipeline import`
- `pipeline ingest`
- `pipeline validate`

## Removed Compat Surface

- `blink`
- `zen`
- `--database`
- `--out`
- `--project`
- `remark-day`
- `sensitive`

## Validation

```powershell
python tools/run.py verify --app tracer_core --scope batch --concise
```

If you need explicit build confirmation for the Windows runtime + Rust CLI:

```powershell
python tools/run.py build --app tracer_core --profile release_bundle --build-dir build --runtime-platform windows
python tools/run.py build --app tracer_windows_rust_cli --profile release_bundle --build-dir build --runtime-platform windows
```

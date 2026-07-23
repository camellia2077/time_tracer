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
  - textual render/export flows and chart presentation
- `exchange`
  - tracer exchange package export/import/inspect
- `system`
  - runtime/system inspection commands
- `about`
  - `about licenses`, `about tracer`, `about motto`
- `txt`
  - shared month-TXT day-block inspection and minimal authored-event append

## Canonical CLI Surface

- `query data`
- `query tree`
- `report render`
- `report export`
- `report chart`
- `exchange export`
- `exchange import`
- `exchange inspect`
- `pipeline convert`
- `pipeline import`
- `pipeline ingest`
- `pipeline validate`
- `txt view-day`
- `txt append-event`
- `system doctor`
- `about licenses`
- `about tracer`
- `about motto`

## Current TXT Authored Event Support

- CLI does not own point/interval timeline semantics locally.
- Source TXT authored events accepted by core include:
  - `HHMMtoken`
  - `HHMM-HHMMtoken`
- `txt view-day` prints the resolved day-block body as-is, including interval lines.
- `txt append-event` appends one authored event line to an existing day block and
  persists it through shared TXT `replace_day_block` semantics.

## Recent Fixed Window

- `report render recent` and `report export recent` accept
  `--as-of YYYY-MM-DD` only for the `recent` period.
- The CLI maps `--as-of` directly to the canonical temporal request
  `anchor_date`; it does not rewrite the request into a local `range`.

Example:

```powershell
time_tracer_cli report render recent 7 --as-of 2026-03-07 --format md --db <db_path>
time_tracer_cli report export recent 7 --as-of 2026-03-07 --format md --db <db_path> --output <out_dir>
```

## Chart Semantics

- `line`, `bar`, and `heatmap-*` chart types use the trend/daily-series report
  chart contract.
- `pie` uses report composition and represents the selected period's root
  breakdown.
- `pie` does not accept `--root`, because its purpose is to show the whole
  period's root composition.

## Exchange Import Runtime Refresh

- `exchange import` replaces the active converter config (`main`,
  `alias_mapping`, and `duration_rules`).
- Core owns the runtime refresh after the replacement; the CLI must not maintain
  an independent cache-invalidation patch.
- Import-path changes require a regression proving that an immediate
  validate/query operation observes the imported config.

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
python tools/run.py verify --app tracer_core --concise
```

If you need explicit build confirmation for the Windows runtime + Rust CLI:

```powershell
python tools/run.py build --app tracer_core --profile release_bundle --build-dir build --runtime-platform windows
python tools/run.py build --app tracer_windows_rust_cli --profile release_bundle --build-dir build --runtime-platform windows
```

---
description: Navigation entry for coding agents working in apps/tools/log_generator
---

# log_generator

## Purpose

Local navigation entry for coding agents working in `apps/tools/log_generator`.

`log_generator` is a tool app that generates canonical TXT test inputs used by
the repository's validation, ingest, query, report, and golden-comparison
flows.

## When To Open

- Open this first when the task touches `apps/tools/log_generator`.
- Use it to find the next 2 to 4 documents to read.

## What This Doc Does Not Cover

- Exact generation algorithms
- File-by-file implementation details
- Detailed data format contracts already documented elsewhere
- Step-by-step refactor plans

## 5-Minute Path

1. `apps/tools/log_generator/README.md`
2. `test/README.md`
3. `docs/toolchain/test/README.md`
4. `docs/log_generator/usage.md`

Open additional docs only when needed:

- Test layering and asset boundaries:
  - `docs/toolchain/test/test_layering.md`
- Time tracer ingest and TXT semantics:
  - `docs/time_tracer/core/ingest/README.md`
  - `docs/time_tracer/core/ingest/txt_to_db_business_logic.md`
- Future interval/mixed timeline target semantics:
  - `docs/time_tracer/core/ingest/interval_event_and_mixed_timeline_semantics.md`

## Current Role In The Repo

- `apps/tools/log_generator` generates canonical TXT datasets.
- `test/data/**` stores shared TXT input assets used across CLI, shell, and
  Android.
- Main program pipelines consume those TXT files and produce downstream query /
  report / export outputs.
- Stable comparison baselines live under `test/golden/**`.

`log_generator` is therefore not an isolated utility. Changes here can affect:

- generated TXT shape and semantics
- downstream ingest expectations
- report/query/export golden outputs
- tool-app self-checks and suite guards

## Code Areas

- CLI entry and argument parsing:
  - `apps/tools/log_generator/src/main.cpp`
  - `apps/tools/log_generator/src/cli/framework/command_line_parser.cpp`
- Application wiring and workflow orchestration:
  - `apps/tools/log_generator/src/application/application.cpp`
  - `apps/tools/log_generator/src/application/workflow/workflow_handler.cpp`
- Config loading and runtime context:
  - `apps/tools/log_generator/src/application/config/config_handler.cpp`
  - `apps/tools/log_generator/src/infrastructure/config/config.cpp`
- Generation domain:
  - `apps/tools/log_generator/src/domain/impl/log_generator.cpp`
  - `apps/tools/log_generator/src/domain/components/day_generator.cpp`
  - `apps/tools/log_generator/src/domain/components/event_generator.cpp`
  - `apps/tools/log_generator/src/domain/strategies/sleep_scheduler.cpp`
- Reporting/self-check helpers:
  - `apps/tools/log_generator/src/application/workflow/workflow_monthly_average_stats.cpp`
  - `apps/tools/log_generator/src/application/reporting/report_handler.cpp`

## Suite And Guard Entry Points

- Suite root:
  - `tools/suites/log_generator/tests.toml`
- Command generation cases:
  - `tools/suites/log_generator/tests/commands_generate.toml`
- Duration/average guard cases:
  - `tools/suites/log_generator/tests/commands_average_guard.toml`
- Python guard logic:
  - `tools/suites/log_generator/scripts/check_daily_average.py`
  - `tools/suites/log_generator/scripts/check_daily_average_lib.py`

## Hard Rules

- `log_generator` is the tool app for generating canonical TXT data; it does
  not move into `test/**`.
- Prefer Python entry commands from repository root.
- Reuse `out/build/log_generator/build_fast` for incremental verification unless
  explicitly instructed otherwise.
- Do not write runtime outputs, temporary databases, or ad-hoc generated
  artifacts back into `test/data/**` or `test/golden/**` unless the task
  explicitly asks to refresh canonical assets.
- If you change generated TXT shape or timeline semantics, also inspect:
  - tool-app self-check logic
  - suite guards
  - downstream consumers and golden expectations
- Store temporary files under repository `temp/`.

## Canonical Verify Flow

Run from repository root:

```powershell
python tools/run.py verify --app log_generator --build-dir build_fast --concise
```

Optional split flow for debugging only:

```powershell
python tools/run.py configure --app log_generator --build-dir build_fast
python tools/run.py build --app log_generator --build-dir build_fast
python tools/run.py verify --app log_generator --build-dir build_fast --concise
```

Result files:

- `out/test/artifact_log_generator/result.json`
- `out/test/artifact_log_generator/result_cases.json`
- `out/test/artifact_log_generator/logs/output.log`

Expected completion state:

- verify command exits with code `0`
- `out/test/artifact_log_generator/result.json` reports `"success": true`

---
description: Run scoped Tidy tasks for log_generator (by log count or batch count)
---

### Inputs (MUST)
- Choose exactly one mode:
  - **Log mode**: clean exactly `<LOG_N>` tasks.
  - **Batch mode**: clean exactly `<BATCH_N>` non-empty `batch_*` folders.
- Batch mode target set = smallest `<BATCH_N>` non-empty batches under `apps/log_generator/build_tidy/tasks/`; freeze this set for the whole run.
- Use incremental build from existing `apps/log_generator/build_agent`; do not delete it.

### Task Source
- If any `apps/log_generator/build_tidy/tasks/batch_*/task_*.log` exists: resume only, do not regenerate.
- Only when tasks are missing:
  - `python scripts/run.py tidy-fix --app log_generator --limit <FIX_N> --keep-going`
  - `python scripts/run.py tidy --app log_generator --jobs 16 --parse-workers 8 --keep-going`

### Rename Baseline
- Run:
  - `python scripts/run.py rename-plan --app log_generator`
  - `python scripts/run.py rename-apply --app log_generator`
  - `python scripts/run.py rename-audit --app log_generator`
- Verify baseline:
  - `python scripts/run.py configure --app log_generator`
  - `python scripts/run.py build --app log_generator`
  - `python test/run.py --suite log_generator --agent --build-dir build_agent --concise`
  - `test/output/log_generator/result.json` must be `success: true`.

### Auto Loop (rename-only)
- Log mode:
  - `python scripts/run.py tidy-loop --app log_generator --n <LOG_N> --test-every <K> --concise`
- Batch mode: usually skip auto loop (avoid crossing selected batch scope).
- Exit code:
  - `0`: requested amount finished (or no eligible task).
  - `2`: manual task blocked; switch to single-task loop.
  - other non-zero: stop and diagnose.

### Single-Task Loop (one task each round)
- Pick one task:
  - Log mode: smallest global `task_NNN.log`.
  - Batch mode: smallest `task_NNN.log` within selected batches.
- Analyze only this log; if pure rename, rerun rename baseline instead of manual edits.
- Fix only this task.
- Verify:
  - `python scripts/run.py build --app log_generator`
  - `python test/run.py --suite log_generator --agent --build-dir build_agent --concise`
  - `test/output/log_generator/result.json` must stay `success: true`.
- Archive:
  - `python scripts/run.py clean --app log_generator <ID>`
  - `clean` matches exact `task_<ID>.log`, moves it to `apps/log_generator/build_tidy/tasks_done/...`, and removes empty `tasks/batch_*`.

### Stop Conditions (MUST)
- Gate: `test/output/log_generator/result.json` keeps `success: true`.
- Log mode done: cleaned count in this run reaches `<LOG_N>`.
- Batch mode done: selected `<BATCH_N>` batches are all empty/removed.

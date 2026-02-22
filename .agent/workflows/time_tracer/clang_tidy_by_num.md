---
description: Run scoped Tidy tasks for time_tracer (by log count or batch count)
---

### Inputs (MUST)
- Choose exactly one mode:
  - **Log mode**: clean exactly `<LOG_N>` tasks.
  - **Batch mode**: clean exactly `<BATCH_N>` non-empty `batch_*` folders.
- Batch mode target set = smallest `<BATCH_N>` non-empty batches under `apps/time_tracer/build_tidy/tasks/`; freeze this set for the whole run.
- Use incremental build from existing `apps/time_tracer/build_fast`; do not delete it.

### Fix Cadence (MUST)
- Work **one `task_NNN.log` at a time**; do not batch-edit multiple logs before verification.
- After each log fix, run:
  - `python scripts/verify.py --app time_tracer --build-dir build_fast --concise`
- `test/output/time_tracer/result.json` must stay `success: true` after every step.
- Batch refresh policy:
  - Each time one selected `batch_*` is fully emptied, run:
  - `python scripts/run.py tidy-refresh --app time_tracer --batch-id <BATCH_ID> --full-every 3 --keep-going`
  - This command does incremental tidy for that batch and triggers periodic full tidy every 3 counted batches.
  - Auto full-tidy fallback:
  - `tidy-refresh` will auto switch to full tidy when it detects stale-graph signals (`no such file or directory`, `GLOB mismatch`, or high `already_renamed` ratio in latest rename report).
  - After auto rebuild, treat old task logs as stale; continue only from newly generated `task_*.log`.
- Batch close shortcut (optional):
  - When one batch is fully fixed and ready to close in one shot, run:
  - `python scripts/run.py tidy-batch --app time_tracer --batch-id <BATCH_ID> --strict-clean --full-every 3 --keep-going`
  - Add `--run-verify --concise` when you want to include verify in the same command.

### ABI Boundary Suppression Policy (MUST)
- 仅允许在 ABI/FFI 边界做定点抑制（`NOLINTNEXTLINE` 或 `NOLINTBEGIN/END`），典型位置：`apps/time_tracer/src/api/core_c` 导出 C 接口签名。
- 定点抑制必须附带原因注释：`ABI compatibility`（或等价表述）。
- 禁止目录级/文件级一刀切忽略 `bugprone-*`、`readability-*`。
- 非 ABI 实现文件（如 `domain/`、`infrastructure/`）优先修复告警，不用“备注忽略”兜底。

### Refactor Guardrail (MUST)
- For long-file maintainability refactors:
  - Step 1: in-file boundary convergence first (helpers/sections/entry narrowing).
  - Step 2: physical split to new `*.cpp` only after Step 1 is stable.
  - Do not mix behavior/features into the same refactor batch.

### Task Source
- If any `apps/time_tracer/build_tidy/tasks/batch_*/task_*.log` exists: resume only, do not regenerate.
- Only when tasks are missing (bootstrap once):
  - `python scripts/run.py tidy-fix --app time_tracer --limit <FIX_N> --keep-going`
  - `python scripts/run.py tidy --app time_tracer --jobs 16 --parse-workers 8 --keep-going`

### Rename Baseline
- Run:
  - `python scripts/run.py rename-plan --app time_tracer`
  - `python scripts/run.py rename-apply --app time_tracer`
  - `python scripts/run.py rename-audit --app time_tracer`
- Verify baseline:
  - `python scripts/run.py configure --app time_tracer`
  - `python scripts/verify.py --app time_tracer --build-dir build_fast --concise`
  - `test/output/time_tracer/result.json` must be `success: true`.

### Auto Loop (rename-only)
- Log mode:
  - `python scripts/run.py tidy-loop --app time_tracer --n <LOG_N> --test-every <K> --concise`
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
  - `python scripts/verify.py --app time_tracer --build-dir build_fast --concise`
  - `test/output/time_tracer/result.json` must stay `success: true`.
- Archive:
  - Batch mode:
  - `python scripts/run.py clean --app time_tracer --strict --batch-id <BATCH_ID> <ID>`
  - Log mode:
  - `python scripts/run.py clean --app time_tracer --strict <ID>`
  - `clean` matches exact `task_<ID>.log`, strict mode requires latest verify success, and with `--batch-id` only cleans that batch.
- When one `batch_*` folder is fully cleaned, refresh tasks before next batch:
  - `python scripts/run.py tidy-refresh --app time_tracer --batch-id <BATCH_ID> --full-every 3 --keep-going`

### Stop Conditions (MUST)
- Gate: `test/output/time_tracer/result.json` keeps `success: true`.
- Log mode done: cleaned count in this run reaches `<LOG_N>`.
- Batch mode done: selected `<BATCH_N>` batches are all empty/removed.
- Final acceptance (when this run is intended as full completion):
  - `python scripts/run.py tidy-close --app time_tracer --keep-going --concise`
  - `tidy-close` includes: `tidy-refresh --final-full` + `verify` + empty-`task_*.log` check.

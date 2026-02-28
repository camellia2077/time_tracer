---
description: Run scoped Tidy tasks for log_generator (by log count or batch count)
---

### Scope Mapping (MUST)
- Tidy analysis scope:
  - `apps/tools/log_generator`
- Task queue location is fixed to:
  - `apps/tools/log_generator/build_tidy/tasks/batch_*/task_*.log`
- Keep incremental build from existing:
  - `apps/tools/log_generator/build_fast`; do not delete it.
- In this workflow, tidy task operations use:
  - `--app log_generator` (`tidy*`, `clean`, `rename-*`).
- Verify gate command:
  - task 级轻量验证：`python scripts/run.py verify --app log_generator --build-dir build_fast --concise --scope task`
  - task scope 检查集：build + native runtime smoke（不含 `runtime_guard`）
  - batch 级全量验证：由 `tidy-batch --preset sop` 内置执行
- Verify gate result file:
  - `test/output/log_generator/result.json` must keep `"success": true`.

### Inputs (MUST)
- Choose exactly one mode:
  - **Log mode**: clean exactly `<LOG_N>` tasks.
  - **Batch mode**: clean exactly `<BATCH_N>` non-empty `batch_*` folders.
- Default path is **Batch mode + tidy-batch**.
- **Log mode** is legacy/troubleshooting-only and may require manual `clean` fallback.
- Batch mode target set = smallest `<BATCH_N>` non-empty batches under `apps/tools/log_generator/build_tidy/tasks/`; freeze this set for the whole run.

### Fix Cadence (MUST)
- Work **one `task_NNN.log` at a time**; do not batch-edit multiple logs before verification.
- After each log fix, run task-scope verify:
  - `python scripts/run.py verify --app log_generator --build-dir build_fast --concise --scope task`
- `test/output/log_generator/result.json` must stay `"success": true` after every step.
- Batch close command (default and recommended):
  - When one selected `batch_*` is ready to close, run:
  - `python scripts/run.py tidy-batch --app log_generator --batch-id <BATCH_ID> --preset sop --timeout-seconds 1800`
  - This command is the unified path for verify gate + clean + incremental refresh.
  - If timeout/interruption occurs, rerun the same command to resume from checkpoint.
  - `tidy-refresh` fallback (auto inside tidy-batch):
  - It auto switches to full tidy when stale-graph signals appear (`no such file or directory`, `GLOB mismatch`, or high `already_renamed` ratio in latest rename report).
  - After auto rebuild, treat old task logs as stale and continue only from newest logs.

### ABI Boundary Suppression Policy (MUST)
- 仅允许在 ABI/FFI 边界做定点抑制（`NOLINTNEXTLINE` 或 `NOLINTBEGIN/END`）。
- 定点抑制必须附带原因注释：`ABI compatibility`（或等价表述）。
- 禁止目录级/文件级一刀切忽略 `bugprone-*`、`readability-*`。
- 非 ABI 实现文件优先修复告警，不用“备注忽略”兜底；如果当前 app 无 ABI 边界，则默认不使用该类抑制。

### Refactor Guardrail (MUST)
- For long-file maintainability refactors:
  - Step 1: in-file boundary convergence first (helpers/sections/entry narrowing).
  - Step 2: physical split to new `*.cpp` only after Step 1 is stable.
  - Do not mix behavior/features into the same refactor batch.

### Task Source
- If any `apps/tools/log_generator/build_tidy/tasks/batch_*/task_*.log` exists: resume only, do not regenerate.
- Only when tasks are missing (bootstrap once):
  - `python scripts/run.py tidy-fix --app log_generator --limit <FIX_N> --keep-going`
  - `python scripts/run.py tidy --app log_generator --jobs 16 --parse-workers 8 --keep-going`

### Rename Baseline
- Run:
  - `python scripts/run.py rename-plan --app log_generator`
  - `python scripts/run.py rename-apply --app log_generator`
  - `python scripts/run.py rename-audit --app log_generator`
- Verify baseline:
  - `python scripts/run.py configure --app log_generator`
  - `python scripts/run.py verify --app log_generator --build-dir build_fast --concise`
  - `test/output/log_generator/result.json` must be `"success": true`.

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
  - `python scripts/run.py verify --app log_generator --build-dir build_fast --concise --scope task`
  - `test/output/log_generator/result.json` must stay `"success": true`.
- Do not manually run `clean + tidy-refresh` in normal flow.
- For multiple tasks mapped to the same source file in one batch, use clustered clean:
  - `python scripts/run.py clean --app log_generator --strict --batch-id <BATCH_ID> --cluster-by-file <ID>`
- After one batch is fully fixed, close it with the unified command:
  - `python scripts/run.py tidy-batch --app log_generator --batch-id <BATCH_ID> --preset sop`
- Manual fallback (`clean` / `tidy-refresh`) is only for troubleshooting command failures.
- Legacy log-mode fallback:
  - `python scripts/run.py clean --app log_generator --strict <ID>`

### Stop Conditions (MUST)
- Gate: `test/output/log_generator/result.json` keeps `"success": true`.
- Log mode done: cleaned count in this run reaches `<LOG_N>`.
- Batch mode done: selected `<BATCH_N>` batches are all empty/removed.
- Final acceptance (when this run is intended as full completion):
  - `python scripts/run.py tidy-close --app log_generator --keep-going --concise`
  - `tidy-close` includes: `tidy-refresh --final-full` + `verify` + empty-`task_*.log` check.


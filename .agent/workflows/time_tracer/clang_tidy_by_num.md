---
description: Run scoped Tidy tasks for time_tracer (by log count or batch count, tracer_core task queue)
---

### Scope Mapping (MUST)
- Tidy analysis scope is:
  - `apps/tracer_core_shell`
- Task queue location is fixed to:
  - `apps/tracer_core_shell/build_tidy/tasks/batch_*/task_*.log`
- Current active batches (resume-first):
  - `apps/tracer_core_shell/build_tidy/tasks/batch_001`
  - `apps/tracer_core_shell/build_tidy/tasks/batch_002`
- Keep incremental build from existing:
  - `apps/tracer_core_shell/build_fast`; do not delete it.
- In this workflow, tidy task operations use:
  - `--app tracer_core` (`tidy*`, `clean`, `rename-*`).
- Verify gate command:
  - task 级轻量验证：`python scripts/run.py verify --app tracer_core --build-dir build_fast --concise --scope task`
  - task scope 检查集：build + native runtime smoke（不含 `runtime_guard`）
  - batch 级全量验证：由 `tidy-batch --preset sop` 内置执行
- Verify gate result file:
  - `test/output/artifact_windows_cli/result.json` must keep `"success": true`.
- Tidy machine summary (single source for agent):
  - `apps/tracer_core_shell/build_tidy/tidy_result.json`
  - Read this file first for `tasks.total/tasks.remaining/blocking_files/next_action`.
- check -> fix_strategy rule table:
  - `scripts/toolchain/config/workflow.toml` -> `[tidy.fix_strategy]`
  - Categories: `auto_fix`, `safe_refactor`, `nolint_allowed`, `manual_only`.

### Python Execution Directory (MUST)
- All Python commands in this workflow must run from repository root:
  - `C:/Computer/my_github/github_cpp/time_tracer/time_tracer_cpp`
- Do not `cd` into `apps/tracer_core_shell` before running `python scripts/run.py ...`.

### Inputs (MUST)
- Choose exactly one mode:
  - **Log mode**: clean exactly `<LOG_N>` tasks.
  - **Batch mode**: clean exactly `<BATCH_N>` non-empty `batch_*` folders.
- Default path is **Batch mode + tidy-batch**.
- **Log mode** is legacy/troubleshooting-only and may require manual `clean` fallback.
- Batch mode target set = smallest `<BATCH_N>` non-empty batches under `apps/tracer_core_shell/build_tidy/tasks/`; freeze this set for the whole run.

### Fix Cadence (MUST)
- Work **one `task_NNN.log` at a time**; do not batch-edit multiple logs before verification.
- After each log fix, run task-scope verify:
  - `python scripts/run.py verify --app tracer_core --build-dir build_fast --concise --scope task`
- `test/output/artifact_windows_cli/result.json` must stay `"success": true` after every step.
- Batch close command (default and recommended):
  - When one selected `batch_*` is ready to close, run:
  - `python scripts/run.py tidy-batch --app tracer_core --batch-id <BATCH_ID> --preset sop --timeout-seconds 1800`
  - This command is the unified path for verify gate + clean + incremental refresh.
  - If timeout/interruption occurs, rerun the same command to resume from checkpoint.
  - `tidy-refresh` fallback (auto inside tidy-batch):
  - It auto switches to full tidy when stale-graph signals appear (`no such file or directory`, `GLOB mismatch`, or high `already_renamed` ratio in latest rename report).
  - After auto rebuild, treat old task logs as stale and continue only from newest logs.

### ABI Boundary Suppression Policy (MUST)
- 仅允许在 ABI/FFI 边界做定点抑制（`NOLINTNEXTLINE` 或 `NOLINTBEGIN/END`），典型位置：`apps/tracer_core_shell/api/c_api` 导出 C 接口签名。
- 定点抑制必须附带原因注释：`ABI compatibility`（或等价表述）。
- 禁止目录级/文件级一刀切忽略 `bugprone-*`、`readability-*`。
- 非 ABI 实现文件（如 `domain/`、`infrastructure/`）优先修复告警，不用“备注忽略”兜底。

### Refactor Guardrail (MUST)
- For long-file maintainability refactors:
  - Step 1: in-file boundary convergence first (helpers/sections/entry narrowing).
  - Step 2: physical split to new `*.cpp` only after Step 1 is stable.
  - Do not mix behavior/features into the same refactor batch.

### Task Source
- If any `apps/tracer_core_shell/build_tidy/tasks/batch_*/task_*.log` exists: resume only, do not regenerate.
- Only when tasks are missing (bootstrap once):
  - `python scripts/run.py tidy-fix --app tracer_core --limit <FIX_N> --keep-going`
  - `python scripts/run.py tidy --app tracer_core --jobs 16 --parse-workers 8 --keep-going`

### Rename Baseline
- Run:
  - `python scripts/run.py rename-plan --app tracer_core`
  - `python scripts/run.py rename-apply --app tracer_core`
  - `python scripts/run.py rename-audit --app tracer_core`
- Verify baseline:
  - `python scripts/run.py configure --app tracer_core`
  - `python scripts/run.py verify --app tracer_core --build-dir build_fast --concise`
  - `test/output/artifact_windows_cli/result.json` must be `"success": true`.

### Auto Loop (rename-only)
- Log mode:
  - `python scripts/run.py tidy-loop --app tracer_core --n <LOG_N> --test-every <K> --concise`
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
  - `python scripts/run.py verify --app tracer_core --build-dir build_fast --concise --scope task`
  - `test/output/artifact_windows_cli/result.json` must stay `"success": true`.
- Do not manually run `clean + tidy-refresh` in normal flow.
- For multiple tasks mapped to the same source file in one batch, use clustered clean:
  - `python scripts/run.py clean --app tracer_core --strict --batch-id <BATCH_ID> --cluster-by-file <ID>`
- After one batch is fully fixed, close it with the unified command:
  - `python scripts/run.py tidy-batch --app tracer_core --batch-id <BATCH_ID> --preset sop`
- Manual fallback (`clean` / `tidy-refresh`) is only for troubleshooting command failures.
- Legacy log-mode fallback:
  - `python scripts/run.py clean --app tracer_core --strict <ID>`

### Stop Conditions (MUST)
- Gate: `test/output/artifact_windows_cli/result.json` keeps `"success": true`.
- Log mode done: cleaned count in this run reaches `<LOG_N>`.
- Batch mode done: selected `<BATCH_N>` batches are all empty/removed.
- Final acceptance (when this run is intended as full completion):
  - `python scripts/run.py tidy-close --app tracer_core --keep-going --concise`
  - `tidy-close` includes: `tidy-refresh --final-full` + `verify` + empty-`task_*.log` check.
- Tidy-only acceptance (decoupled from business verify):
  - `python scripts/run.py tidy-close --app tracer_core --tidy-only --keep-going`
  - This mode enforces only: `tidy-refresh --final-full` + empty-`task_*.log` check.



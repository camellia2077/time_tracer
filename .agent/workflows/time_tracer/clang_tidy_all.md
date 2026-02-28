---
description: Run all Tidy tasks for time_tracer (windows_cli task queue)
---

### Scope Mapping (MUST)
- Tidy analysis scope includes both:
  - `apps/tracer_windows_cli`
  - `apps/tracer_core` (most diagnostics files are here)
- Task queue location is fixed to:
  - `apps/tracer_windows_cli/build_tidy/tasks/batch_*/task_*.log`
- Keep incremental build from existing:
  - `apps/tracer_windows_cli/build_fast`; do not delete it.
- In this workflow, tidy task operations use:
  - `--app tracer_windows_cli` (`tidy*`, `clean`, `rename-*`).
- Verify gate command:
  - task 级轻量验证：`python scripts/run.py verify --app tracer_core --build-dir build_fast --concise --scope task`
  - task scope 检查集：build + native runtime smoke（不含 `runtime_guard`）
  - batch 级全量验证：由 `tidy-batch --preset sop` 内置执行
- Verify gate result file:
  - `test/output/tracer_windows_cli/result.json` must keep `"success": true`.

### Entry Command (MUST)
- `python scripts/run.py tidy-flow --app tracer_windows_cli --all --resume --test-every 3 --concise --keep-going --with-tidy-fix --tidy-fix-limit <FIX_N>`
- Exit code rule:
  - `0`: auto phase finished for now; still must pass completion gate below.
  - `2`: manual tasks remain; continue with single-task loop.
  - other non-zero: stop and diagnose.

### Hard Completion Gate (MUST)
- Completion is valid **only when** no `task_*.log` exists under `apps/tracer_windows_cli/build_tidy/tasks/`.
- All `batch_*` folders under `apps/tracer_windows_cli/build_tidy/tasks/` must be empty or removed.
- `test/output/tracer_windows_cli/result.json` must exist and keep `"success": true`.
- Partial progress is **not** completion.
- Exit code `2` is never completion.

### Execution Rules (MUST)
- ABI/FFI 边界抑制规则：
  - 仅允许在 ABI 边界做定点抑制（`NOLINTNEXTLINE` 或 `NOLINTBEGIN/END`），典型位置：`apps/tracer_core/src/api/core_c` 导出 C 接口签名。
  - 定点抑制必须附带原因注释：`ABI compatibility`（或等价表述）。
  - 禁止目录级/文件级一刀切忽略 `bugprone-*`、`readability-*`。
  - 非 ABI 实现文件优先修复告警，不用“备注忽略”兜底。
- Task source:
  - If any `apps/tracer_windows_cli/build_tidy/tasks/batch_*/task_*.log` exists, resume only.
  - Only when tasks are missing (bootstrap once):
    - `python scripts/run.py tidy-fix --app tracer_windows_cli --limit <FIX_N> --keep-going`
    - `python scripts/run.py tidy --app tracer_windows_cli --jobs 16 --parse-workers 8 --keep-going`
- Rename baseline:
  - `python scripts/run.py rename-plan --app tracer_windows_cli`
  - `python scripts/run.py rename-apply --app tracer_windows_cli`
  - `python scripts/run.py rename-audit --app tracer_windows_cli`
- Baseline verify:
  - `python scripts/run.py configure --app tracer_windows_cli`
  - `python scripts/run.py verify --app tracer_core --build-dir build_fast --concise`
  - `test/output/tracer_windows_cli/result.json` must be `"success": true`.
- Run fast loop:
  - `python scripts/run.py tidy-loop --app tracer_windows_cli --all --test-every 3 --concise`
- Auto rebuild fallback:
  - `tidy-refresh` now auto switches to full tidy when stale-graph signals are detected (`no such file or directory`, `GLOB mismatch`, or high `already_renamed` ratio in latest rename report).
  - Once auto rebuild runs, stale old logs are discarded logically; continue from newest `task_*.log` only.
- If manual tasks remain, run single-task loop (one task per round):
  - pick smallest pending `task_NNN.log`;
  - derive `<BATCH_ID>` from selected task path (`tasks/batch_xxx/task_NNN.log`);
  - analyze one log (if pure rename, rerun rename baseline);
  - fix one task only (one-log-at-a-time cadence);
- verify after each log (task scope): `python scripts/run.py verify --app tracer_core --build-dir build_fast --concise --scope task`
  - `test/output/tracer_windows_cli/result.json` must stay `"success": true`;
  - when the same source file has multiple tasks in one batch, prefer clustered clean:
  - `python scripts/run.py clean --app tracer_windows_cli --strict --batch-id <BATCH_ID> --cluster-by-file <ID>`
  - do not manually run `clean + tidy-refresh` in normal flow;
  - close each fixed batch with unified command:
  - `python scripts/run.py tidy-batch --app tracer_windows_cli --batch-id <BATCH_ID> --preset sop --timeout-seconds 1800`
  - `tidy-batch` includes verify gate + clean + tidy-refresh, and keeps the periodic full tidy cadence (`--full-every 3`);
  - if timeout/interruption occurs, rerun the same `tidy-batch` command; it resumes from checkpoint automatically.
  - manual fallback (`clean` / `tidy-refresh`) is troubleshooting-only when `tidy-batch` fails.
- Repeat until hard completion gate is satisfied.
- Final tidy acceptance (mandatory once at the end):
  - `python scripts/run.py tidy-close --app tracer_windows_cli --keep-going --concise`
  - `tidy-close` enforces: final-full refresh + verify + no `task_*.log`.


---
description: Run all Tidy tasks for log_generator
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

### Entry Command (MUST)
- `python scripts/run.py tidy-flow --app log_generator --all --resume --test-every 3 --concise --keep-going --with-tidy-fix --tidy-fix-limit <FIX_N>`
- Exit code rule:
  - `0`: auto phase finished for now; still must pass completion gate below.
  - `2`: manual tasks remain; continue with single-task loop.
  - other non-zero: stop and diagnose.

### Hard Completion Gate (MUST)
- Completion is valid **only when** no `task_*.log` exists under `apps/tools/log_generator/build_tidy/tasks/`.
- All `batch_*` folders under `apps/tools/log_generator/build_tidy/tasks/` must be empty or removed.
- `test/output/log_generator/result.json` must exist and keep `"success": true`.
- Partial progress is **not** completion.
- Exit code `2` is never completion.

### Execution Rules (MUST)
- ABI/FFI 边界抑制规则：
  - 仅允许在 ABI 边界做定点抑制（`NOLINTNEXTLINE` 或 `NOLINTBEGIN/END`）。
  - 定点抑制必须附带原因注释：`ABI compatibility`（或等价表述）。
  - 禁止目录级/文件级一刀切忽略 `bugprone-*`、`readability-*`。
  - 非 ABI 实现文件优先修复告警，不用“备注忽略”兜底；如果当前 app 无 ABI 边界，则默认不使用该类抑制。
- Task source:
  - If any `apps/tools/log_generator/build_tidy/tasks/batch_*/task_*.log` exists, resume only.
  - Only when tasks are missing (bootstrap once):
    - `python scripts/run.py tidy-fix --app log_generator --limit <FIX_N> --keep-going`
    - `python scripts/run.py tidy --app log_generator --jobs 16 --parse-workers 8 --keep-going`
- Rename baseline:
  - `python scripts/run.py rename-plan --app log_generator`
  - `python scripts/run.py rename-apply --app log_generator`
  - `python scripts/run.py rename-audit --app log_generator`
- Baseline verify:
  - `python scripts/run.py configure --app log_generator`
  - `python scripts/run.py verify --app log_generator --build-dir build_fast --concise`
  - `test/output/log_generator/result.json` must be `"success": true`.
- Run fast loop:
  - `python scripts/run.py tidy-loop --app log_generator --all --test-every 3 --concise`
- Auto rebuild fallback:
  - `tidy-refresh` now auto switches to full tidy when stale-graph signals are detected (`no such file or directory`, `GLOB mismatch`, or high `already_renamed` ratio in latest rename report).
  - Once auto rebuild runs, stale old logs are discarded logically; continue from newest `task_*.log` only.
- If manual tasks remain, run single-task loop (one task per round):
  - pick smallest pending `task_NNN.log`;
  - derive `<BATCH_ID>` from selected task path (`tasks/batch_xxx/task_NNN.log`);
  - analyze one log (if pure rename, rerun rename baseline);
  - fix one task only (one-log-at-a-time cadence);
- verify after each log (task scope): `python scripts/run.py verify --app log_generator --build-dir build_fast --concise --scope task`
  - `test/output/log_generator/result.json` must stay `"success": true`;
  - when the same source file has multiple tasks in one batch, prefer clustered clean:
  - `python scripts/run.py clean --app log_generator --strict --batch-id <BATCH_ID> --cluster-by-file <ID>`
  - do not manually run `clean + tidy-refresh` in normal flow;
  - close each fixed batch with unified command:
  - `python scripts/run.py tidy-batch --app log_generator --batch-id <BATCH_ID> --preset sop --timeout-seconds 1800`
  - `tidy-batch` includes verify gate + clean + tidy-refresh, and keeps the periodic full tidy cadence (`--full-every 3`);
  - if timeout/interruption occurs, rerun the same `tidy-batch` command; it resumes from checkpoint automatically.
  - manual fallback (`clean` / `tidy-refresh`) is troubleshooting-only when `tidy-batch` fails.
- Repeat until hard completion gate is satisfied.
- Final tidy acceptance (mandatory once at the end):
  - `python scripts/run.py tidy-close --app log_generator --keep-going --concise`
  - `tidy-close` enforces: final-full refresh + verify + no `task_*.log`.


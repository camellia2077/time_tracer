---
description: Run all Tidy tasks for log_generator
---

### Entry Command (MUST)
- `python scripts/run.py tidy-flow --app log_generator --all --resume --test-every 3 --concise --keep-going --with-tidy-fix --tidy-fix-limit <FIX_N>`
- Exit code rule:
  - `0`: auto phase finished for now; still must pass completion gate below.
  - `2`: manual tasks remain; continue with single-task loop.
  - other non-zero: stop and diagnose.

### Hard Completion Gate (MUST)
- Completion is valid **only when** no `task_*.log` exists under `apps/log_generator/build_tidy/tasks/`.
- `test/output/log_generator/result.json` must exist and keep `success: true`.
- Partial progress is **not** completion.
- Exit code `2` is never completion.

### Execution Rules (MUST)
- Keep incremental build: use existing `apps/log_generator/build_fast`; do not delete it.
- ABI/FFI 边界抑制规则：
  - 仅允许在 ABI 边界做定点抑制（`NOLINTNEXTLINE` 或 `NOLINTBEGIN/END`）。
  - 定点抑制必须附带原因注释：`ABI compatibility`（或等价表述）。
  - 禁止目录级/文件级一刀切忽略 `bugprone-*`、`readability-*`。
  - 非 ABI 实现文件优先修复告警，不用“备注忽略”兜底；如果当前 app 无 ABI 边界，则默认不使用该类抑制。
- Task source:
  - If any `apps/log_generator/build_tidy/tasks/batch_*/task_*.log` exists, resume only.
  - Only when tasks are missing (bootstrap once):
    - `python scripts/run.py tidy-fix --app log_generator --limit <FIX_N> --keep-going`
    - `python scripts/run.py tidy --app log_generator --jobs 16 --parse-workers 8 --keep-going`
- Rename baseline:
  - `python scripts/run.py rename-plan --app log_generator`
  - `python scripts/run.py rename-apply --app log_generator`
  - `python scripts/run.py rename-audit --app log_generator`
- Baseline verify:
  - `python scripts/run.py configure --app log_generator`
  - `python scripts/verify.py --app log_generator --build-dir build_fast --concise`
  - `test/output/log_generator/result.json` must be `success: true`.
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
  - verify build + test after each log:
  - `python scripts/verify.py --app log_generator --build-dir build_fast --concise`
  - `result.json` must stay `success: true`;
  - archive with `python scripts/run.py clean --app log_generator --strict --batch-id <BATCH_ID> <ID>`.
  - when one `batch_*` is emptied, run incremental refresh:
  - `python scripts/run.py tidy-refresh --app log_generator --batch-id <BATCH_ID> --full-every 3 --keep-going`
  - cadence note: `tidy-refresh` triggers a periodic full tidy every 3 counted batches.
  - optional batch-close shortcut:
  - `python scripts/run.py tidy-batch --app log_generator --batch-id <BATCH_ID> --strict-clean --full-every 3 --keep-going`
  - add `--run-verify --concise` if you want verify in the same command.
- Repeat until hard completion gate is satisfied.
- Final tidy acceptance (mandatory once at the end):
  - `python scripts/run.py tidy-close --app log_generator --keep-going --concise`
  - `tidy-close` enforces: final-full refresh + verify + no `task_*.log`.

---
description: Run all Tidy tasks for time_tracer
---

### Entry Command (MUST)
- `python scripts/run.py tidy-flow --app time_tracer --all --resume --test-every 3 --concise --keep-going --with-tidy-fix --tidy-fix-limit <FIX_N>`
- Exit code rule:
  - `0`: auto phase finished for now; still must pass completion gate below.
  - `2`: manual tasks remain; continue with single-task loop.
  - other non-zero: stop and diagnose.

### Hard Completion Gate (MUST)
- Completion is valid **only when** no `task_*.log` exists under `apps/time_tracer/build_tidy/tasks/`.
- `test/output/time_tracer/result.json` must exist and keep `success: true`.
- Partial progress is **not** completion.
- Exit code `2` is never completion.

### Execution Rules (MUST)
- Keep incremental build: use existing `apps/time_tracer/build_fast`; do not delete it.
- ABI/FFI 边界抑制规则：
  - 仅允许在 ABI 边界做定点抑制（`NOLINTNEXTLINE` 或 `NOLINTBEGIN/END`），典型位置：`apps/time_tracer/src/api/core_c` 导出 C 接口签名。
  - 定点抑制必须附带原因注释：`ABI compatibility`（或等价表述）。
  - 禁止目录级/文件级一刀切忽略 `bugprone-*`、`readability-*`。
  - 非 ABI 实现文件优先修复告警，不用“备注忽略”兜底。
- Task source:
  - If any `apps/time_tracer/build_tidy/tasks/batch_*/task_*.log` exists, resume only.
  - Only when tasks are missing (bootstrap once):
    - `python scripts/run.py tidy-fix --app time_tracer --limit <FIX_N> --keep-going`
    - `python scripts/run.py tidy --app time_tracer --jobs 16 --parse-workers 8 --keep-going`
- Rename baseline:
  - `python scripts/run.py rename-plan --app time_tracer`
  - `python scripts/run.py rename-apply --app time_tracer`
  - `python scripts/run.py rename-audit --app time_tracer`
- Baseline verify:
  - `python scripts/run.py configure --app time_tracer`
  - `python scripts/verify.py --app time_tracer --build-dir build_fast --concise`
  - `test/output/time_tracer/result.json` must be `success: true`.
- Run fast loop:
  - `python scripts/run.py tidy-loop --app time_tracer --all --test-every 3 --concise`
- Auto rebuild fallback:
  - `tidy-refresh` now auto switches to full tidy when stale-graph signals are detected (`no such file or directory`, `GLOB mismatch`, or high `already_renamed` ratio in latest rename report).
  - Once auto rebuild runs, stale old logs are discarded logically; continue from newest `task_*.log` only.
- If manual tasks remain, run single-task loop (one task per round):
  - pick smallest pending `task_NNN.log`;
  - derive `<BATCH_ID>` from selected task path (`tasks/batch_xxx/task_NNN.log`);
  - analyze one log (if pure rename, rerun rename baseline);
  - fix one task only (one-log-at-a-time cadence);
  - verify after each log: `python scripts/verify.py --app time_tracer --build-dir build_fast --concise`
  - `result.json` must stay `success: true`;
  - archive with `python scripts/run.py clean --app time_tracer --strict --batch-id <BATCH_ID> <ID>`.
  - when one `batch_*` is emptied, run incremental refresh:
  - `python scripts/run.py tidy-refresh --app time_tracer --batch-id <BATCH_ID> --full-every 3 --keep-going`
  - cadence note: `tidy-refresh` triggers a periodic full tidy every 3 counted batches.
  - optional batch-close shortcut:
  - `python scripts/run.py tidy-batch --app time_tracer --batch-id <BATCH_ID> --strict-clean --full-every 3 --keep-going`
  - add `--run-verify --concise` if you want verify in the same command.
- Repeat until hard completion gate is satisfied.
- Final tidy acceptance (mandatory once at the end):
  - `python scripts/run.py tidy-close --app time_tracer --keep-going --concise`
  - `tidy-close` enforces: final-full refresh + verify + no `task_*.log`.

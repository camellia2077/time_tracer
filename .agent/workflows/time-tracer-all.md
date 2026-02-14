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
- Task source:
  - If any `apps/time_tracer/build_tidy/tasks/batch_*/task_*.log` exists, resume only.
  - Only when tasks are missing:
    - `python scripts/run.py tidy-fix --app time_tracer --limit <FIX_N> --keep-going`
    - `python scripts/run.py tidy --app time_tracer --jobs 16 --parse-workers 8 --keep-going`
- Rename baseline:
  - `python scripts/run.py rename-plan --app time_tracer`
  - `python scripts/run.py rename-apply --app time_tracer`
  - `python scripts/run.py rename-audit --app time_tracer`
- Baseline verify:
  - `python scripts/run.py configure --app time_tracer`
  - `python scripts/run.py build --app time_tracer`
  - `python test/run.py --suite time_tracer --agent --build-dir build_fast --concise`
  - `test/output/time_tracer/result.json` must be `success: true`.
- Run fast loop:
  - `python scripts/run.py tidy-loop --app time_tracer --all --test-every 3 --concise`
- If manual tasks remain, run single-task loop (one task per round):
  - pick smallest pending `task_NNN.log`;
  - analyze one log (if pure rename, rerun rename baseline);
  - fix one task only;
  - verify build + test (`result.json` must stay `success: true`);
  - archive with `python scripts/run.py clean --app time_tracer <ID>`.
- Repeat until hard completion gate is satisfied.

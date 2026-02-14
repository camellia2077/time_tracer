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
- Task source:
  - If any `apps/log_generator/build_tidy/tasks/batch_*/task_*.log` exists, resume only.
  - Only when tasks are missing:
    - `python scripts/run.py tidy-fix --app log_generator --limit <FIX_N> --keep-going`
    - `python scripts/run.py tidy --app log_generator --jobs 16 --parse-workers 8 --keep-going`
- Rename baseline:
  - `python scripts/run.py rename-plan --app log_generator`
  - `python scripts/run.py rename-apply --app log_generator`
  - `python scripts/run.py rename-audit --app log_generator`
- Baseline verify:
  - `python scripts/run.py configure --app log_generator`
  - `python scripts/run.py build --app log_generator`
  - `python test/run.py --suite log_generator --agent --build-dir build_fast --concise`
  - `test/output/log_generator/result.json` must be `success: true`.
- Run fast loop:
  - `python scripts/run.py tidy-loop --app log_generator --all --test-every 3 --concise`
- If manual tasks remain, run single-task loop (one task per round):
  - pick smallest pending `task_NNN.log`;
  - analyze one log (if pure rename, rerun rename baseline);
  - fix one task only;
  - verify build + test (`result.json` must stay `success: true`);
  - archive with `python scripts/run.py clean --app log_generator <ID>`.
- Repeat until hard completion gate is satisfied.

---
description: Agent policy for the full time_tracer clang-tidy queue (strict profile)
---

## Shared Base (MUST)
- Read and obey: `.agent/workflows/time_tracer/clang_tidy_all_shared.md`
- This file only defines strict-profile bindings for shared placeholders.

## Fixed Contract (MUST)
- Official config profile: strict

## Config Policy (MUST)
- Always use `--strict-config`.
- Keep `--strict-config` on every follow-up `tidy-flow` / `tidy-step` / `tidy-batch` / `tidy-close` command in this run.
- Do not silently downgrade to the daily profile mid-run.

## Profile Bindings (MUST)
- `<PROFILE_FLOW_FLAGS>` = `--strict-config`
- `<PROFILE_STEP_FLAGS>` = `--strict-config`
- `<PROFILE_BATCH_FLAGS>` = `--strict-config`
- `<PROFILE_CLOSE_FLAGS>` = `--strict-config`

## Expanded Commands (MUST)
- Auto entry:
  - `python tools/run.py tidy-flow --app tracer_core_shell --source-scope core_family --tidy-build-dir build_tidy_core_family --task-view toon --all --resume --test-every 3 --concise --keep-going --with-tidy-fix --tidy-fix-limit <FIX_N> --strict-config`
- Step 4 command:
  - `tidy-step --task-log <resolved_task_json> --dry-run --strict-config`
- Step 5 command:
  - `tidy-step --task-log <resolved_task_json> --strict-config`
- Batch close command:
  - `tidy-batch --preset sop --strict-config`
- Final acceptance command:
  - `python tools/run.py tidy-close --app tracer_core_shell --source-scope core_family --tidy-build-dir build_tidy_core_family --keep-going --concise --strict-config`

---
description: Agent policy for the full time_tracer clang-tidy queue (daily profile)
---

## Shared Base (MUST)
- Read and obey: `.agent/workflows/time_tracer/clang_tidy_all_shared.md`
- This file only defines daily-profile bindings for shared placeholders.

## Fixed Contract (MUST)
- Official config profile: daily

## Config Policy (MUST)
- Always use the repo-root `.clang-tidy`.
- Do not switch to strict config inside this workflow.
- If the user explicitly asks for strict cleanup, use the strict workflow doc instead of modifying this one ad hoc.
- Do not pass `--strict-config` in this run.

## Profile Bindings (MUST)
- `<PROFILE_FLOW_FLAGS>` = *(empty)*
- `<PROFILE_STEP_FLAGS>` = *(empty)*
- `<PROFILE_BATCH_FLAGS>` = *(empty)*
- `<PROFILE_CLOSE_FLAGS>` = *(empty)*

## Expanded Commands (MUST)
- Auto entry:
  - `python tools/run.py tidy-flow --app tracer_core_shell --source-scope core_family --tidy-build-dir build_tidy_core_family --task-view toon --all --resume --test-every 3 --concise --keep-going --with-tidy-fix --tidy-fix-limit <FIX_N>`
- Step 4 command:
  - `tidy-step --task-log <resolved_task_json> --dry-run`
- Step 5 command:
  - `tidy-step --task-log <resolved_task_json>`
- Batch close command:
  - `tidy-batch --preset sop`
- Final acceptance command:
  - `python tools/run.py tidy-close --app tracer_core_shell --source-scope core_family --tidy-build-dir build_tidy_core_family --keep-going --concise`

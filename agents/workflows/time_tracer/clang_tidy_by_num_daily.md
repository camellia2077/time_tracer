---
description: Agent policy for one numbered time_tracer clang-tidy task (daily profile)
---

## Shared Base (MUST)
- Read and obey: `.agent/workflows/time_tracer/clang_tidy_by_num_shared.md`
- This file only defines daily-profile bindings for shared placeholders.

## Fixed Contract (MUST)
- Official config profile: daily

## Config Policy (MUST)
- Always use the repo-root `.clang-tidy`.
- Do not switch to strict config inside this workflow.
- Do not pass `--strict-config` in this run.

## Profile Bindings (MUST)
- `<PROFILE_STEP_FLAGS>` = *(empty)*
- `<PROFILE_BATCH_FLAGS>` = *(empty)*

## Expanded Commands (MUST)
- Step 4 command:
  - `tidy-step --task-log <resolved_task_json> --dry-run`
- Step 5 command:
  - `tidy-step --task-log <resolved_task_json>`
- Batch close command:
  - `tidy-batch --preset sop`

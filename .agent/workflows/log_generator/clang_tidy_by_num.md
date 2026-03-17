---
description: Agent policy for one numbered log_generator clang-tidy task
---

## Fixed Contract (MUST)
- Official app anchor: `log_generator`
- Official tidy workspace: `build_tidy`
- Run from repo root only: `C:\code\time_tracer`

## Fixed Paths (MUST)
- Task queue: `out/tidy/log_generator/build_tidy/tasks/batch_*/task_*.json|log|toon`
- Automation reports: `out/tidy/log_generator/build_tidy/automation/`
- Verify result: `out/test/artifact_log_generator/result.json`

## Input Policy (MUST)
- Input is exactly one pending `<TASK_ID>`.
- Resolve the real `task_<TASK_ID>` artifact path first.
- Derive `<BATCH_ID>` from that path before running anything.
- Work on one task only.

## Required Order (MUST)
- Use this order:
  1. `tidy-task-patch`
  2. `tidy-task-fix --dry-run`
  3. `tidy-task-suggest`
  4. `tidy-step --dry-run`
  5. `tidy-step`
- If flags are unclear, read the subcommand `-h` first.

## Manual Fix Policy (MUST)
- Only if the previews still show manual work:
  - fix that one task
  - keep changes minimal
  - rerun task-scope verify
- Prefer `automation/` reports over re-reading the raw task log repeatedly.

## Batch Close Policy (MUST)
- After the task is fixed and verified, close the batch with `tidy-batch --preset sop`.
- Do not manually compose `clean + tidy-refresh` in the normal path.

## Completion Rule (MUST)
- This workflow is done for the requested number only when:
  - the selected `task_<TASK_ID>` artifact is gone from `tasks/`
  - verify still reports success
  - batch handoff is completed through `tidy-batch`

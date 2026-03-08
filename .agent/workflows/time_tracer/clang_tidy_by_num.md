---
description: Agent policy for one numbered time_tracer clang-tidy task
---

## Fixed Contract (MUST)
- Official app anchor: `tracer_core_shell`
- Official source scope: `core_family`
- Official tidy workspace: `build_tidy_core_family`
- Run from repo root only: `C:\code\time_tracer`

## Fixed Paths (MUST)
- Task queue: `out/tidy/tracer_core_shell/build_tidy_core_family/tasks/batch_*/task_*.log`
- Automation reports: `out/tidy/tracer_core_shell/build_tidy_core_family/automation/`
- Verify result: `out/test/artifact_windows_cli/result.json`

## Input Policy (MUST)
- Input is exactly one pending `<TASK_ID>`.
- Resolve the real `task_<TASK_ID>.log` path first.
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
  - the selected `task_<TASK_ID>.log` is gone from `tasks/`
  - verify still reports success
  - batch handoff is completed through `tidy-batch`

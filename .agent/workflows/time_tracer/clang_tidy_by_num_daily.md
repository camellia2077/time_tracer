---
description: Agent policy for one numbered time_tracer clang-tidy task (daily profile)
---

## Fixed Contract (MUST)
- Official app anchor: `tracer_core_shell`
- Official source scope: `core_family`
- Official tidy workspace: `build_tidy_core_family`
- Official config profile: daily
- Run from repo root only: `C:\code\time_tracer`

## Config Policy (MUST)
- Always use the repo-root `.clang-tidy`.
- Do not switch to strict config inside this workflow.

## Fixed Paths (MUST)
- Task queue: `out/tidy/tracer_core_shell/build_tidy_core_family/tasks/batch_*/task_*.json|toon|log`
  - `task_*.json` is canonical for machine execution only
  - `task_*.toon` is the default reading view for humans/agents when present
  - `task_*.log` is compatibility-only
- Automation reports: `out/tidy/tracer_core_shell/build_tidy_core_family/automation/`
- Verify result: `out/test/artifact_windows_cli/result.json`

## Input Policy (MUST)
- Input is exactly one pending `<TASK_ID>`.
- Resolve the real `task_<TASK_ID>` artifact path first.
- Read `.toon` first. Only fall back to `.json`, then `.log`, when `.toon` is missing or clearly insufficient for the current decision.
- Do not switch to `.json` just for normal task reading; preserve `toon` as the low-token reading contract.
- For task-local commands, always execute with the canonical `.json` path via `--task-log <resolved_task_json>`.
- Derive `<BATCH_ID>` from that resolved path before running anything.
- After any `tidy-batch` / `tidy-refresh` / `tidy-flow` / rebase, re-resolve from the current `tasks/` tree. Do not trust an older batch/task pair.
- Work on one task only.

## Required Order (MUST)
- Use this order:
  1. `tidy-task-patch --task-log <resolved_task_json>`
  2. `tidy-task-fix --task-log <resolved_task_json> --dry-run`
  3. `tidy-task-suggest --task-log <resolved_task_json>`
  4. `tidy-step --task-log <resolved_task_json> --dry-run`
  5. `tidy-step --task-log <resolved_task_json>`
- If flags are unclear, read the subcommand `-h` first.

## Manual Fix Policy (MUST)
- Only if the previews still show manual work:
  - fix that one task
  - keep changes minimal
  - rerun build sanity check
- Prefer `automation/` reports over re-reading the raw task log repeatedly.

## Batch Close Policy (MUST)
- `tidy-step` is the normal close path for one task:
  - it runs build sanity check
  - reruns focused clang-tidy on the selected task source
  - archives the matching `task_<TASK_ID>` artifact when that re-check is clean
- After that, close the batch with `tidy-batch --preset sop`.
- Queue batch id is a queue code, not a historical identity. Rebase may change task contents, so always resolve the current task path again before continuing.
- Do not manually compose `clean + tidy-refresh` in the normal path.

## Completion Rule (MUST)
- This workflow is done for the requested number only when:
  - the selected `task_<TASK_ID>` artifact is gone from `tasks/`
  - verify still reports success
  - batch handoff is completed through `tidy-batch`

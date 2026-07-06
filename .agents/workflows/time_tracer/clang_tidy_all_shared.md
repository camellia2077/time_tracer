---
description: Shared base policy for the full time_tracer clang-tidy queue (profile-agnostic)
---

## Fixed Contract (MUST)
- Official app anchor: `tracer_core_shell`
- Official source scope: `core_family`
- Official tidy workspace: `build_tidy_core_family`
- Run from repo root only: `C:\code\time_tracer`

## Style Authority (MUST)
- Style and naming rules are defined by the active clang-tidy config profile selected by the caller workflow.
- Do not maintain a duplicated style-rule checklist in workflow markdown files.
- LLM suggestions are advisory only; clang-tidy/verify results are authoritative for acceptance.

## Fixed Paths (MUST)
- Task queue: `out/tidy/tracer_core_shell/build_tidy_core_family/tasks/batch_*/task_*.json|toon|log`
  - `task_*.json` is canonical for machine execution only
  - `task_*.toon` is the default reading view for humans/agents when present
  - `task_*.log` is compatibility-only
- Machine summary: `out/tidy/tracer_core_shell/build_tidy_core_family/tidy_result.json`
- Automation reports: `out/tidy/tracer_core_shell/build_tidy_core_family/automation/`
- Verify result: `out/test/artifact_windows_cli/result.json`

## First Command (MUST)
- Start with the official auto entry:
  - `python tools/run.py tidy-flow --app tracer_core_shell --source-scope core_family --tidy-build-dir build_tidy_core_family --task-view toon --all --resume --test-every 3 --concise --keep-going --with-tidy-fix --tidy-fix-limit <FIX_N> <PROFILE_FLOW_FLAGS>`
- If flags are unclear, read `python tools/run.py tidy-flow -h` before changing anything.

## Single-Task Policy (MUST)
- When auto flow stops on manual tasks, always pick the smallest pending task by canonical identity, but read the task through `task_NNN.toon` first when it exists.
- Only fall back to `.json`, then `.log`, when `.toon` is missing or clearly insufficient for the current decision.
- Do not switch to `.json` just for normal task reading; preserve `toon` as the low-token reading contract.
- Always derive `<BATCH_ID>` from the real task path before acting.
- For task-local commands, always execute with the canonical `.json` path via `--task-log <resolved_task_json>`.
- After any `tidy-flow` / `tidy-batch` / `tidy-refresh` / rebase, re-resolve from the current `tasks/` tree. Do not trust an older batch/task pair.
- Use this order:
  1. `tidy-task-patch --task-log <resolved_task_json>`
  2. `tidy-task-fix --task-log <resolved_task_json> --dry-run`
  3. `tidy-task-suggest --task-log <resolved_task_json>`
  4. `tidy-step --task-log <resolved_task_json> --dry-run <PROFILE_STEP_FLAGS>`
  5. `tidy-step --task-log <resolved_task_json> <PROFILE_STEP_FLAGS>`
- Treat `automation/` as the first place to read before manual fixing.

## Batch Policy (MUST)
- `tidy-step` is the normal one-task close path:
  - it runs build sanity check
  - reruns focused clang-tidy on the selected task source
  - archives the matching `task_<TASK_ID>` artifact when that re-check is clean
- Normal batch close path is `tidy-batch --preset sop <PROFILE_BATCH_FLAGS>`.
- Queue batch id is a queue code, not a historical identity. Full rebase / full refresh keeps the current pending queue namespace instead of rewinding to `batch_001`.
- `clean + tidy-refresh` is troubleshooting-only, not the normal workflow.
- If the same file has several task logs in one batch, prefer clustered clean.

## Completion Gate (MUST)
- Done means:
  - no `task_*.json` / `task_*.log` / `task_*.toon` remains under `out/tidy/tracer_core_shell/build_tidy_core_family/tasks/`
  - `out/test/artifact_windows_cli/result.json` still reports success
- Exit code `2` from auto flow is not completion.
- Final acceptance command is:
  - `python tools/run.py tidy-close --app tracer_core_shell --source-scope core_family --tidy-build-dir build_tidy_core_family --keep-going --concise <PROFILE_CLOSE_FLAGS>`

## Repo-Specific Guardrails (MUST)
- Only use pinpoint suppression at true ABI boundaries such as `apps/tracer_core_shell/api/c_api`.
- Non-ABI implementation files must prefer real fixes over suppression.
- Do not add app-side shell wrappers for clang-tidy; use `python tools/run.py ...` directly.
- For parameter syntax and defaults, always consult `python tools/run.py <subcommand> -h`.

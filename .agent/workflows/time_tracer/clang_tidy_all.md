---
description: Agent policy for the full time_tracer clang-tidy queue
---

## Fixed Contract (MUST)
- Official app anchor: `tracer_core_shell`
- Official source scope: `core_family`
- Official tidy workspace: `build_tidy_core_family`
- Run from repo root only: `C:\code\time_tracer`

## Fixed Paths (MUST)
- Task queue: `out/tidy/tracer_core_shell/build_tidy_core_family/tasks/batch_*/task_*.json|log|toon`
- Machine summary: `out/tidy/tracer_core_shell/build_tidy_core_family/tidy_result.json`
- Automation reports: `out/tidy/tracer_core_shell/build_tidy_core_family/automation/`
- Verify result: `out/test/artifact_windows_cli/result.json`

## First Command (MUST)
- Start with the official auto entry:
  - `python tools/run.py tidy-flow --app tracer_core_shell --source-scope core_family --tidy-build-dir build_tidy_core_family --all --resume --test-every 3 --concise --keep-going --with-tidy-fix --tidy-fix-limit <FIX_N>`
- If flags are unclear, read `python tools/run.py tidy-flow -h` before changing anything.

## Single-Task Policy (MUST)
- When auto flow stops on manual tasks, always pick the smallest pending `task_NNN` artifact.
- Always derive `<BATCH_ID>` from the real task path before acting.
- Use this order:
  1. `tidy-task-patch`
  2. `tidy-task-fix --dry-run`
  3. `tidy-task-suggest`
  4. `tidy-step --dry-run`
  5. `tidy-step`
- Treat `automation/` as the first place to read before manual fixing.

## Batch Policy (MUST)
- `tidy-step` is the normal one-task close path:
  - it runs build sanity check
  - reruns focused clang-tidy on the selected task source
  - archives the matching `task_<TASK_ID>` artifact when that re-check is clean
- Normal batch close path is `tidy-batch --preset sop`.
- `clean + tidy-refresh` is troubleshooting-only, not the normal workflow.
- If the same file has several task logs in one batch, prefer clustered clean.

## Completion Gate (MUST)
- Done means:
  - no `task_*.json` / `task_*.log` / `task_*.toon` remains under `out/tidy/tracer_core_shell/build_tidy_core_family/tasks/`
  - `out/test/artifact_windows_cli/result.json` still reports success
- Exit code `2` from auto flow is not completion.
- Final acceptance command is:
  - `python tools/run.py tidy-close --app tracer_core_shell --source-scope core_family --tidy-build-dir build_tidy_core_family --keep-going --concise`

## Repo-Specific Guardrails (MUST)
- Only use pinpoint suppression at true ABI boundaries such as `apps/tracer_core_shell/api/c_api`.
- Non-ABI implementation files must prefer real fixes over suppression.
- Do not add app-side shell wrappers for clang-tidy; use `python tools/run.py ...` directly.
- For parameter syntax and defaults, always consult `python tools/run.py <subcommand> -h`.

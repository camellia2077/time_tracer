---
description: Agent policy for the full log_generator clang-tidy queue
---

## Fixed Contract (MUST)
- Official app anchor: `log_generator`
- Official tidy workspace: `build_tidy`
- Run from repo root only: `C:\code\time_tracer`

## Fixed Paths (MUST)
- Task queue: `out/tidy/log_generator/build_tidy/tasks/batch_*/task_*.json|log|toon`
- Machine summary: `out/tidy/log_generator/build_tidy/tidy_result.json`
- Automation reports: `out/tidy/log_generator/build_tidy/automation/`
- Verify result: `out/test/artifact_log_generator/result.json`

## First Command (MUST)
- Start with the official auto entry:
  - `python tools/run.py tidy-flow --app log_generator --tidy-build-dir build_tidy --all --resume --test-every 3 --concise --keep-going --with-tidy-fix --tidy-fix-limit <FIX_N>`
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
- Normal close path is `tidy-batch --preset sop`.
- `clean + tidy-refresh` is troubleshooting-only, not the normal workflow.
- If the same file has several task logs in one batch, prefer clustered clean.

## Completion Gate (MUST)
- Done means:
  - no `task_*.json` / `task_*.log` / `task_*.toon` remains under `out/tidy/log_generator/build_tidy/tasks/`
  - `out/test/artifact_log_generator/result.json` still reports success
- Exit code `2` from auto flow is not completion.
- Final acceptance command is:
  - `python tools/run.py tidy-close --app log_generator --tidy-build-dir build_tidy --keep-going --concise`

## Repo-Specific Guardrails (MUST)
- Use suppression only when there is a real boundary reason; otherwise prefer real fixes.
- For parameter syntax and defaults, always consult `python tools/run.py <subcommand> -h`.

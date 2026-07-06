---
description: Agent policy for the full log_generator clang-tidy queue (strict profile)
---

## Fixed Contract (MUST)
- Official app anchor: `log_generator`
- Official tidy workspace: `build_tidy`
- Official config profile: strict
- Run from repo root only: `C:\code\time_tracer`

## Config Policy (MUST)
- Always use `--strict-config`.
- Keep `--strict-config` on every follow-up `tidy-flow` / `tidy-step` / `tidy-batch` / `tidy-close` command in this run.
- Do not silently downgrade to the daily profile mid-run.

## Fixed Paths (MUST)
- Task queue: `out/tidy/log_generator/build_tidy/tasks/batch_*/task_*.json|toon|log`
  - `task_*.json` is canonical for machine execution only
  - `task_*.toon` is the default reading view for humans/agents when present
  - `task_*.log` is compatibility-only
- Machine summary: `out/tidy/log_generator/build_tidy/tidy_result.json`
- Automation reports: `out/tidy/log_generator/build_tidy/automation/`
- Verify result: `out/test/artifact_log_generator/result.json`

## First Command (MUST)
- Start with the official auto entry:
  - `python tools/run.py tidy-flow --app log_generator --tidy-build-dir build_tidy --task-view toon --all --resume --test-every 3 --concise --keep-going --with-tidy-fix --tidy-fix-limit <FIX_N> --strict-config`
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
  4. `tidy-step --task-log <resolved_task_json> --dry-run --strict-config`
  5. `tidy-step --task-log <resolved_task_json> --strict-config`
- Treat `automation/` as the first place to read before manual fixing.

## Batch Policy (MUST)
- `tidy-step` is the normal one-task close path:
  - it runs build sanity check
  - reruns focused clang-tidy on the selected task source
  - archives the matching `task_<TASK_ID>` artifact when that re-check is clean
- Normal close path is `tidy-batch --preset sop --strict-config`.
- Queue batch id is a queue code, not a historical identity. Full rebase / full refresh keeps the current pending queue namespace instead of rewinding to `batch_001`.
- `clean + tidy-refresh` is troubleshooting-only, not the normal workflow.
- If the same file has several task logs in one batch, prefer clustered clean.

## Completion Gate (MUST)
- Done means:
  - no `task_*.json` / `task_*.log` / `task_*.toon` remains under `out/tidy/log_generator/build_tidy/tasks/`
  - `out/test/artifact_log_generator/result.json` still reports success
- Exit code `2` from auto flow is not completion.
- Final acceptance command is:
  - `python tools/run.py tidy-close --app log_generator --tidy-build-dir build_tidy --keep-going --concise --strict-config`

## Repo-Specific Guardrails (MUST)
- Use suppression only when there is a real boundary reason; otherwise prefer real fixes.
- For parameter syntax and defaults, always consult `python tools/run.py <subcommand> -h`.

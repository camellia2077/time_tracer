---
description: Shared bounded source-cluster contract for the time_tracer clang-tidy queue
---

## Fixed contract

- Run from `C:\code\time_tracer`.
- App: `tracer_core_shell`.
- Source scope: `core_family`.
- Tidy workspace: `build_tidy_core_family`.
- Canonical queue: `out/tidy/tracer_core_shell/build_tidy_core_family/tasks/`.

The queue layout is:

```text
tasks/
  clusters/
    <source_filename>_<hash>/
      cluster.json
      task_001.json
      task_001.toon
      task_001.log
  scan_manifest.json
  queue_state.json
```

`task_*.json` is the machine contract. `.toon` is the preferred reading view. A cluster groups all pending diagnostics for one source file; process the whole cluster, not one stale diagnostic.

## Bounded processing

Start with:

```powershell
python tools/run.py tidy-agent --app tracer_core_shell --source-scope core_family --tidy-build-dir build_tidy_core_family --max-clusters 3 --max-tasks 10 --max-minutes 30 <PROFILE_FLAGS>
```

The runner re-resolves the queue after every cluster. It may stop because the budget is exhausted (`paused`), because the current source cluster still needs an agent edit or refresh (`blocked`/exit `2`), or because the queue is empty and final close is required (`queue_empty_requires_tidy_close`). A blocked result is not a request for user confirmation: the agent edits the current cluster and reruns it.

For one selected task, resolve its current path, read `.toon`, and execute with the matching `.json` path:

```powershell
python tools/run.py tidy-source-step --task-log <resolved_task_json> --dry-run <PROFILE_FLAGS>
python tools/run.py tidy-source-step --task-log <resolved_task_json> <PROFILE_FLAGS>
```

`tidy-source-step` applies safe fixes, runs build sanity, re-runs focused clang-tidy for the complete source cluster, and archives the cluster only after the fresh check is clean. Never manually move or delete task files.

After any source edit, refresh, rebase, or archive, discard old task paths and resolve the current `tasks/` tree again. Use `tidy-refresh` when the queue itself is stale; it performs a full scan -> cluster -> task regeneration.

## Completion gate

A bounded slice may stop at any budget boundary; record the current runner state and resume later. The whole queue is complete only when:

- no `task_*.json`, `.toon`, or `.log` remains anywhere below `tasks/`;
- `tidy-close` exits `0`;
- `tidy_result.json.final_gate` reports final-full tidy, verify, and queue-empty as passed;
- the verify result is newer than the final source changes.

Preview and then execute:

```powershell
python tools/run.py tidy-close --app tracer_core_shell --source-scope core_family --tidy-build-dir build_tidy_core_family --dry-run <PROFILE_FLAGS>
python tools/run.py tidy-close --app tracer_core_shell --source-scope core_family --tidy-build-dir build_tidy_core_family --keep-going --concise <PROFILE_FLAGS>
```

There is no batch checkpoint or batch identity in this workflow.

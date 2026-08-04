---
description: Shared contract for processing one current task as its complete source cluster
---

The selected task is only a seed. Resolve the current `tasks/clusters/<source_filename>_<hash>/task_<local_id>.json`, read the sibling `.toon`, and process every pending task in that cluster. Do not infer a global task number and do not reuse a path after a refresh or source edit.

For the fixed time_tracer app/scope, run:

```powershell
python tools/run.py tidy-source-step --task-log <resolved_task_json> --dry-run <PROFILE_FLAGS>
python tools/run.py tidy-source-step --task-log <resolved_task_json> <PROFILE_FLAGS>
```

The command owns safe auto-fix, build sanity, focused re-check, and cluster archive. If diagnostics remain, the agent makes the smallest source edit it can justify, then re-resolves the cluster and runs it again. Exit code `2` means the agent must continue editing or refresh the cluster; it is not a request for user confirmation and not success.

The selected cluster slice is complete only when its current task artifacts are archived after a fresh clean re-check. This does not imply whole-queue completion; use `tidy-agent` to continue and `tidy-close` for the final gate.

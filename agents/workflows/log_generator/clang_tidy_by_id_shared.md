---
description: Shared contract for one log_generator task seed expanded to its source cluster
---

A selected task is only a seed. Resolve the current `out/tidy/log_generator/build_tidy/tasks/clusters/<source_filename>_<hash>/task_<local_id>.json`, read its `.toon` sibling, and process the complete cluster. Re-resolve after every source edit, refresh, or archive.

```powershell
python tools/run.py tidy-source-step --task-log <resolved_task_json> --dry-run <PROFILE_FLAGS>
python tools/run.py tidy-source-step --task-log <resolved_task_json> <PROFILE_FLAGS>
```

The command performs safe auto-fix, build sanity, focused clang-tidy re-check, and cluster archive. Exit code `2` means the agent must edit the current source cluster or refresh it. Do not wait for user confirmation. Continue with `tidy-agent`; the whole queue is complete only after `tidy-close` passes its final-full, verify, and empty-queue gate.

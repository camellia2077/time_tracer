---
description: One source-cluster-seeded time_tracer clang-tidy run using the daily profile
---

Read and obey `agents/workflows/time_tracer/clang_tidy_by_id_shared.md` and use the repo-root `.clang-tidy` without `--strict-config`.

```powershell
python tools/run.py tidy-source-step --task-log <resolved_task_json> --dry-run
python tools/run.py tidy-source-step --task-log <resolved_task_json>
```

---
description: One source-cluster-seeded time_tracer clang-tidy run using the strict profile
---

Read and obey `agents/workflows/time_tracer/clang_tidy_by_id_shared.md`. Keep `--strict-config` on every command.

```powershell
python tools/run.py tidy-source-step --task-log <resolved_task_json> --dry-run --strict-config
python tools/run.py tidy-source-step --task-log <resolved_task_json> --strict-config
```

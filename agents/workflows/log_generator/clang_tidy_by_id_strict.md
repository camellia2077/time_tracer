---
description: One log_generator source-cluster-seeded clang-tidy run using the strict profile
---

Read `agents/workflows/log_generator/clang_tidy_by_id_shared.md`. Keep `--strict-config` on both commands.

```powershell
python tools/run.py tidy-source-step --task-log <resolved_task_json> --dry-run --strict-config
python tools/run.py tidy-source-step --task-log <resolved_task_json> --strict-config
```

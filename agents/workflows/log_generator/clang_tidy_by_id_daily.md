---
description: One log_generator source-cluster-seeded clang-tidy run using the daily profile
---

Read `agents/workflows/log_generator/clang_tidy_by_id_shared.md`. Use `.clang-tidy` and omit `--strict-config`.

```powershell
python tools/run.py tidy-source-step --task-log <resolved_task_json> --dry-run
python tools/run.py tidy-source-step --task-log <resolved_task_json>
```

---
description: Bounded full log_generator clang-tidy queue using the daily profile
---

The queue contract is the same as `agents/workflows/time_tracer/clang_tidy_all_shared.md`: `scan -> source clusters -> tasks`, with `tasks/<cluster_id>/task_<local_id>.*`. There is no batch command.

Use `.clang-tidy` without `--strict-config`:

```powershell
python tools/run.py tidy-agent --app log_generator --tidy-build-dir build_tidy --max-clusters 3 --max-tasks 10 --max-minutes 30
python tools/run.py tidy-close --app log_generator --tidy-build-dir build_tidy --dry-run --keep-going --concise
python tools/run.py tidy-close --app log_generator --tidy-build-dir build_tidy --keep-going --concise
```

After exit code `2` or a `blocked` cluster, the agent fixes the current source cluster and resumes from a freshly resolved task path; do not wait for user confirmation.

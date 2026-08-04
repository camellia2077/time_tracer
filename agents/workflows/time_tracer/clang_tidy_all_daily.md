---
description: Bounded full time_tracer clang-tidy queue using the daily profile
---

Read and obey `agents/workflows/time_tracer/clang_tidy_all_shared.md`.

Use the repo-root `.clang-tidy`; do not pass `--strict-config`.

```powershell
python tools/run.py tidy-agent --app tracer_core_shell --source-scope core_family --tidy-build-dir build_tidy_core_family --max-clusters 3 --max-tasks 10 --max-minutes 30
python tools/run.py tidy-close --app tracer_core_shell --source-scope core_family --tidy-build-dir build_tidy_core_family --dry-run
python tools/run.py tidy-close --app tracer_core_shell --source-scope core_family --tidy-build-dir build_tidy_core_family --keep-going --concise
```

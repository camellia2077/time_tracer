---
description: Bounded full time_tracer clang-tidy queue using the strict profile
---

Read and obey `agents/workflows/time_tracer/clang_tidy_all_shared.md`.

Always pass `--strict-config`; do not downgrade to the daily profile during this run.

```powershell
python tools/run.py tidy-agent --app tracer_core_shell --source-scope core_family --tidy-build-dir build_tidy_core_family --max-clusters 3 --max-tasks 10 --max-minutes 30 --strict-config
python tools/run.py tidy-close --app tracer_core_shell --source-scope core_family --tidy-build-dir build_tidy_core_family --dry-run --strict-config
python tools/run.py tidy-close --app tracer_core_shell --source-scope core_family --tidy-build-dir build_tidy_core_family --keep-going --concise --strict-config
```

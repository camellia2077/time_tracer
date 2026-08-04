---
description: Bounded full log_generator clang-tidy queue using the strict profile
---

Use the cluster-first queue contract and stop rules in `agents/workflows/time_tracer/clang_tidy_all_shared.md`. There is no batch command.

Keep `--strict-config` on every command:

```powershell
python tools/run.py tidy-agent --app log_generator --tidy-build-dir build_tidy --max-clusters 3 --max-tasks 10 --max-minutes 30 --strict-config
python tools/run.py tidy-close --app log_generator --tidy-build-dir build_tidy --dry-run --keep-going --concise --strict-config
python tools/run.py tidy-close --app log_generator --tidy-build-dir build_tidy --keep-going --concise --strict-config
```

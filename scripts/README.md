# Scripts Overview

本目录包含项目的 Python 自动化入口与工具链实现。

## 快速导航

1. 命令入口：
   - `scripts/run.py`
2. 工具链实现：
   - `scripts/toolchain/`
3. 定位文档（建议先读）：
   - `docs/toolchain/python_command_map.md`
   - `docs/toolchain/clang_tidy_sop.md`
4. agent 规则：
   - `scripts/AGENTS.md`

## 常见命令

```bash
python scripts/run.py build --app tracer_windows_cli --profile release_bundle -- --target help
python scripts/run.py verify --app tracer_core --quick --concise
python scripts/run.py tidy --app tracer_windows_cli -- --target tidy_all
python scripts/run.py tidy-batch --app tracer_windows_cli --batch-id <BATCH_ID> --strict-clean --run-verify --concise --full-every 3 --keep-going
```

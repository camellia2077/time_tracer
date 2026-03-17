# Scripts Overview

本目录现在只保留开发辅助脚本，不再承载仓库级构建 / 编译 / clang-tidy 工具链。

## 目录

1. `tools/scripts/devtools/loc/`
   - 代码行数统计等辅助脚本
2. `tools/scripts/devtools/android/`
   - Android 辅助脚本
3. `tools/scripts/devtools/ps/`
   - PowerShell 辅助脚本

## 官方入口说明

- Python 构建 / 编译 / verify / tidy 官方入口：
  - `python tools/run.py ...`
- 平台配置同步入口：
  - `python tools/platform_config/run.py ...`

## 示例

```bash
# Python 工具链主入口
python tools/run.py verify --app tracer_core_shell --quick --scope batch --concise

# 开发辅助脚本示例
python tools/scripts/devtools/loc/run.py --lang py tools test tools/scripts/devtools --under 120
```

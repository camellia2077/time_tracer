# Scripts Overview

本目录现在只保留开发辅助脚本，不再承载仓库级构建 / 编译 / clang-tidy 工具链。

## 目录

1. `tools/scripts/devtools/android/`
   - Android 辅助脚本
2. `tools/scripts/devtools/ps/`
   - PowerShell 辅助脚本

## 官方入口说明

- Python 构建 / 编译 / verify / tidy 官方入口：
  - `python tools/run.py ...`
- 平台配置同步入口：
  - `python -m tools.platform_config.run ...`

## 示例

```bash
# Python 工具链主入口
python tools/run.py verify --app tracer_core_shell --profile fast --concise

# 开发辅助脚本示例
python -m tools.devtools.loc_scanner --lang py tools test tools/scripts/devtools --under 120

# 将测试 TXT 和 activity_hierarchy 注入已安装的可调试 Android 应用
python tools/scripts/devtools/android/push_test_data.py --serial <adb-serial> --launch
```

`push_test_data.py` 默认读取 `test/data` 和 `test/data/activity_hierarchy`，会先停止应用、清理私有测试输入和数据库，再通过 `run-as` 写入
`files/tracer_core/input` 与 `files/tracer_core/config/activity_hierarchy`。它不会把测试数据打进 APK；应用必须先安装 debug APK。

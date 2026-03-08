# Devtools Overview

本目录存放**开发辅助脚本**，不参与默认 `post-change/verify` 流水线。

## 子目录

1. `scripts/devtools/loc/`
   - 代码行数扫描工具（C++ / Kotlin / Python）。
2. `scripts/devtools/android/`
   - Android 相关辅助脚本（如测试输入同步）。

## 示例

```bash
python scripts/devtools/loc/scan_cpp_lines.py apps/tracer_cli/windows apps/tracer_core_shell libs/tracer_core -t 350
python scripts/devtools/loc/scan_kt_lines.py apps/tracer_android -t 350
python scripts/devtools/loc/scan_py_lines.py tools test scripts/devtools -t 200
```

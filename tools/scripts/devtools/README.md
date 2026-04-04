# Devtools Overview

本目录存放**开发辅助脚本**，不参与默认 `build/verify/validate` 流水线。

## 子目录

1. `tools/scripts/devtools/android/`
   - Android 相关辅助脚本（如测试输入同步）。

## 示例

```bash
python -m tools.devtools.loc_scanner --lang cpp apps/cli/windows apps/tracer_core_shell libs/tracer_core --over 350
python -m tools.devtools.loc_scanner --lang kt apps/android --over 350
python -m tools.devtools.loc_scanner --lang py tools test tools/scripts/devtools --over 200
```

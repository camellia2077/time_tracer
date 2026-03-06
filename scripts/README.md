# Scripts Overview

本目录包含项目的 Python 自动化入口与工具链实现。

## 快速导航

1. 命令入口：
   - `scripts/run.py`
2. 工具链实现：
   - `scripts/toolchain/`
3. 开发者工具（不参与默认构建/测试流水线）：
   - `scripts/devtools/loc/`（代码行数统计）
   - `scripts/devtools/android/`（Android 辅助脚本）
4. 定位文档（建议先读）：
   - `docs/toolchain/python_command_map.md`
   - `docs/toolchain/clang_tidy_sop.md`
5. agent 规则：
   - `scripts/AGENTS.md`

## 常见命令

```bash
# 日常唯一入口（构建+编译+测试，推荐语义名 tracer_core_shell）
python scripts/run.py post-change --app tracer_core_shell --run-tests always --build-dir build_fast --concise

# 里程碑唯一入口（兼容旧 app id: tracer_core）
python scripts/run.py verify --app tracer_core_shell --quick --scope batch --concise

# 先构建 core runtime DLL（apps/tracer_core_shell/build/bin）
bash apps/tracer_cli/windows/scripts/build_core_runtime_release.sh

# 再基于 apps/tracer_core_shell/build/bin 编译 Rust CLI
bash apps/tracer_cli/windows/scripts/build_rust_from_windows_build.sh

# Rust CLI 构建（Windows）
python scripts/run.py build --app tracer_windows_rust_cli --build-dir build_fast

# Android 编辑期（固定构建目录后端，不使用 --build-dir）
python scripts/run.py build --app tracer_android --profile android_edit

# Android 验证期
python scripts/run.py verify --app tracer_android --profile android_style --concise
python scripts/run.py post-change --app tracer_android --run-tests always --concise
```

## 结果文件（统一契约）

- State: `apps/<app>/<build_dir>/post_change_last.json`
  - 例如 `tracer_core` / `tracer_core_shell` 默认可用 `apps/tracer_core_shell/build_fast/post_change_last.json`
  - `tracer_android` 固定使用 `apps/tracer_android/build/post_change_last.json`
- Summary: `test/output/<result_target>/result.json`
- Case details: `test/output/<result_target>/result_cases.json`
- Aggregated log: `test/output/<result_target>/logs/output.log`

`<result_target>` 映射：

- `tracer_core` / `tracer_core_shell` / `tracer_windows_rust_cli` -> `artifact_windows_cli`
- `tracer_android` -> `artifact_android`
- `log_generator` -> `artifact_log_generator`
- 未映射 app 保持 `<result_target>=<app>`


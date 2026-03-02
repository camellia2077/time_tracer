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
# 日常唯一入口（构建+编译+测试）
python scripts/run.py post-change --app tracer_core --run-tests always --build-dir build_fast --concise

# 里程碑唯一入口
python scripts/run.py verify --app tracer_core --quick --scope batch --concise

# 先构建 core runtime DLL（apps/tracer_core/build/bin）
bash apps/tracer_cli/windows/scripts/build_core_runtime_release.sh

# 再基于 apps/tracer_core/build/bin 编译 Rust CLI
bash apps/tracer_cli/windows/scripts/build_rust_from_windows_build.sh

# Rust CLI 构建（Windows）
python scripts/run.py build --app tracer_windows_rust_cli --build-dir build_fast
```

## 结果文件（统一契约）

- State: `apps/<app>/build_fast/post_change_last.json`
- Summary: `test/output/<result_target>/result.json`
- Case details: `test/output/<result_target>/result_cases.json`
- Aggregated log: `test/output/<result_target>/logs/output.log`

`<result_target>` 映射：

- `tracer_core` / `tracer_windows_rust_cli` -> `artifact_windows_cli`
- `tracer_android` -> `artifact_android`
- `log_generator` -> `artifact_log_generator`
- 未映射 app 保持 `<result_target>=<app>`


# Tools Overview

本目录包含项目的 Python 自动化入口与工具链实现。

## 快速导航

1. 命令入口：
   - `tools/run.py`
2. 工具链实现：
   - `tools/toolchain/`
3. 开发者工具（不参与默认构建/测试流水线）：
   - `tools/scripts/devtools/loc/`（代码行数统计）
   - `tools/scripts/devtools/android/`（Android 辅助脚本）
4. 定位文档（建议先读）：
   - `docs/toolchain/python_command_map.md`
   - `docs/toolchain/clang_tidy_architecture.md`
   - `docs/toolchain/clang_tidy_flow.md`
   - `docs/toolchain/clang_tidy_sop.md`
5. agent 规则：
   - `tools/AGENTS.md`

## 常见命令

```bash
# 默认全量 clang-tidy 队列（workspace: out/tidy/tracer_core_shell/build_tidy）
python tools/run.py tidy --app tracer_core_shell

# libs core 正式 scoped tidy（workspace: out/tidy/tracer_core_shell/build_tidy_core_family）
python tools/run.py tidy --app tracer_core_shell --source-scope core_family --build-dir build_tidy_core_family
python tools/run.py tidy --app tracer_core_shell --source-scope core_family --build-dir build_tidy_core_family --task-view json
python tools/run.py tidy --app tracer_core_shell --source-scope core_family --build-dir build_tidy_core_family --task-view toon
python tools/run.py tidy --app tracer_core_shell --source-scope core_family --build-dir build_tidy_core_family --task-view text+toon
python tools/run.py tidy-fix --app tracer_core_shell --source-scope core_family --tidy-build-dir build_tidy_core_family

# Task 级自动化（patch / fix / suggest / 单步执行）
python tools/run.py tidy-task-patch --app tracer_core_shell --source-scope core_family --tidy-build-dir build_tidy_core_family --batch-id 002 --task-id 011
python tools/run.py tidy-task-fix --app tracer_core_shell --source-scope core_family --tidy-build-dir build_tidy_core_family --batch-id 002 --task-id 011 --dry-run
python tools/run.py tidy-task-suggest --app tracer_core_shell --source-scope core_family --tidy-build-dir build_tidy_core_family --batch-id 002 --task-id 011
python tools/run.py tidy-step --app tracer_core_shell --source-scope core_family --tidy-build-dir build_tidy_core_family --batch-id 002 --task-id 011 --dry-run

# Windows CLI 单命令验证入口（推荐语义名 tracer_core_shell）
python tools/run.py verify --app tracer_core_shell --build-dir build_fast --concise

# 里程碑唯一入口（兼容旧 app id: tracer_core）
python tools/run.py verify --app tracer_core_shell --profile fast --concise

# 先构建 Windows core runtime DLL（staged output: out/build/tracer_core_shell/build/bin）
python tools/run.py build --app tracer_core --profile release_bundle --build-dir build --runtime-platform windows

# 再基于当前 build 目录中的 runtime 产物编译 Rust CLI
python tools/run.py build --app tracer_windows_rust_cli --profile release_bundle --build-dir build --runtime-platform windows

# Rust CLI 构建（Windows）
python tools/run.py build --app tracer_windows_rust_cli --build-dir build_fast --runtime-platform windows

# Android 编辑期（固定构建目录后端，不使用 --build-dir）
python tools/run.py build --app tracer_android --profile android_edit

# Android 验证期
python tools/run.py verify --app tracer_android --profile android_style --concise
python tools/run.py verify --app tracer_android --profile android_ci --concise
```

## clang-tidy scoped workspace

- 默认全量队列继续使用逻辑 workspace 名 `build_tidy`，物理目录为 `out/tidy/tracer_core_shell/build_tidy`
- clang-tidy 的唯一官方输入是 `out/tidy/<app>/<tidy_workspace>/analysis_compile_db/compile_commands.json`
- raw build `compile_commands.json` 只属于构建系统，不再属于 clang-tidy contract
- `--source-scope core_family` 对应 4 个 core libs 的生产源码：
  - `libs/tracer_core/src`
  - `libs/tracer_adapters_io/src`
  - `libs/tracer_core_bridge_common/src`
  - `libs/tracer_transport/src`
- `core_family` 的默认独立 tidy workspace 是 `out/tidy/tracer_core_shell/build_tidy_core_family`
- 对 scoped 队列继续执行 `tidy-fix` / `tidy-batch` / `tidy-close` / `tidy-flow` 时，显式带上：
  - `--source-scope core_family`
  - `--tidy-build-dir build_tidy_core_family`（或 `tidy` / `tidy-refresh` / `tidy-split` 的 `--build-dir`）
- task 级自动化报告统一写到 `out/tidy/<app>/<tidy_workspace>/automation/`
  - `tidy-task-patch`：候选 patch / skip reason
  - `tidy-task-fix`：安全自动修复报告
  - `tidy-task-suggest`：decode helper / protocol constants 建议
  - `tidy-step`：单步执行状态
- clang-tidy 不再依赖应用侧 shell wrapper；统一直接使用 `python tools/run.py ...`
- `--task-view` 当前支持 4 档：
  - `json` -> `task_*.json`
  - `text` -> `task_*.json + task_*.log`
  - `toon` -> `task_*.json + task_*.toon`
  - `text+toon` -> `task_*.json + task_*.log + task_*.toon`

## 结果文件（统一契约）

- Validation summary: `out/validate/<run_name>/summary.json`
- Validation logs:
  - `out/validate/<run_name>/logs/output.log`
  - `out/validate/<run_name>/logs/output.full.log`
- Summary: `out/test/<result_target>/result.json`
- Case details: `out/test/<result_target>/result_cases.json`
- Aggregated log: `out/test/<result_target>/logs/output.log`
- Quality gates: `out/test/<result_target>/quality_gates/`
  - Generated case snapshots and audit reports used by `verify` / `refresh-golden`

`<result_target>` 映射：

- `tracer_core` / `tracer_core_shell` / `tracer_windows_rust_cli` -> `artifact_windows_cli`
- `tracer_android` -> `artifact_android`
- `log_generator` -> `artifact_log_generator`
- 未映射 app 保持 `<result_target>=<app>`

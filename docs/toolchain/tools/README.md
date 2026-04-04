# Python Toolchain Guide

本文档说明 `tools/run.py` 及 `tools/toolchain/` 的官方入口、结构分层、常见命令和 clang-tidy 约定。

## 1. 官方入口

1. 命令入口：
   - `tools/run.py`
2. 工具链实现：
   - `tools/toolchain/`
3. 开发辅助脚本：
   - `tools/scripts/devtools/loc/`
   - `tools/scripts/devtools/android/`

## 2. 分层结构

1. `tools/run.py`
   - 统一入口，构建 `Context` 并分发 CLI 子命令
2. `tools/toolchain/cli/`
   - 参数注册、CLI handler、入参组装
3. `tools/toolchain/commands/`
   - 真实执行逻辑，如 `build` / `verify` / `tidy` / `validate`
4. `tools/toolchain/core/`
   - 配置、上下文、进程执行等基础设施
5. `tools/toolchain/config/`
   - profile、workflow、validate 路径等配置源
6. `tools/toolchain/services/`
   - 纯逻辑层与辅助状态服务
7. `tools/toolchain/formats/`
   - TOON 等内部格式编解码

## 3. 改动路由

1. 只改命令行参数定义：
   - `tools/toolchain/cli/handlers/*.py`
2. 只改构建/验证/clang-tidy 执行逻辑：
   - `tools/toolchain/commands/**`
3. 只改默认配置与 profile：
   - `tools/toolchain/config/*.toml`
   - `tools/toolchain/core/config.py`
4. 只改入口转发：
   - `tools/run.py`
5. 只改 Windows CLI wrapper 参数通道：
   - `apps/cli/windows/scripts/build*.sh`

## 4. 常见命令

```bash
# C++ 轨默认 clang-tidy 队列
python tools/run.py tidy --app tracer_core_shell --task-view toon

# core_family scoped clang-tidy
python tools/run.py tidy --app tracer_core_shell --source-scope core_family --build-dir build_tidy_core_family --task-view toon
python tools/run.py tidy --app tracer_core_shell --source-scope core_family --build-dir build_tidy_core_family --task-view json
python tools/run.py tidy --app tracer_core_shell --source-scope core_family --build-dir build_tidy_core_family --task-view text+toon
python tools/run.py tidy-fix --app tracer_core_shell --source-scope core_family --tidy-build-dir build_tidy_core_family

# task 级自动化
python tools/run.py tidy-task-patch --task-log out/tidy/tracer_core_shell/build_tidy_core_family/tasks/batch_002/task_011.json
python tools/run.py tidy-task-fix --task-log out/tidy/tracer_core_shell/build_tidy_core_family/tasks/batch_002/task_011.json --dry-run
python tools/run.py tidy-task-suggest --task-log out/tidy/tracer_core_shell/build_tidy_core_family/tasks/batch_002/task_011.json
python tools/run.py tidy-step --task-log out/tidy/tracer_core_shell/build_tidy_core_family/tasks/batch_002/task_011.json --dry-run
python tools/run.py tidy-batch --app tracer_core_shell --source-scope core_family --tidy-build-dir build_tidy_core_family --batch-id batch_002 --preset sop

# configure / build / format
python tools/run.py configure --app tracer_core_shell
python tools/run.py build --app tracer_core_shell --profile fast --concise
python tools/run.py format --app tracer_core_shell

# Windows runtime + Rust CLI 构建
python tools/run.py build --app tracer_core --profile release_bundle --build-dir build --runtime-platform windows
python tools/run.py build --app tracer_windows_rust_cli --profile release_bundle --build-dir build --runtime-platform windows

# Android
python tools/run.py build --app tracer_android --profile android_edit
python tools/run.py verify --app tracer_android --profile android_style --concise
python tools/run.py verify --app tracer_android --profile android_ci --concise
# Android single-invocation merged profiles
python tools/run.py verify --app tracer_android --profile android_style --profile android_ci --concise
```

## 5. clang-tidy workspace 契约

1. 默认全量队列 workspace：
   - `out/tidy/tracer_core_shell/build_tidy`
2. `core_family` 默认 scoped workspace：
   - `out/tidy/tracer_core_shell/build_tidy_core_family`
3. clang-tidy 的唯一官方输入：
   - `out/tidy/<app>/<tidy_workspace>/analysis_compile_db/compile_commands.json`
4. raw build `compile_commands.json` 只属于构建系统，不属于 clang-tidy contract
5. `--source-scope core_family` 当前对应：
   - `libs/tracer_core/src`
   - `libs/tracer_adapters_io/src`
   - `libs/tracer_core_bridge_common/src`
   - `libs/tracer_transport/src`
6. queue 管理命令应显式带上：
   - `--source-scope core_family`
   - `--tidy-build-dir build_tidy_core_family`

## 6. task artifact 契约

1. `task_*.json` 是 canonical machine contract
2. 对人和 agent：
   - 默认先读 `task_*.toon`
   - `.toon` 缺失或明显不足时才回退 `.json` / `.log`
3. 对命令执行：
   - `tidy-task-*` / `tidy-step` 统一使用 `task_*.json` 作为 `--task-log`
   - `app` / `tidy workspace` / `source scope` 统一从 `--task-log` 路径反推
4. `--task-view` 只控制附加阅读视图：
   - `json` -> `task_*.json`
   - `text` -> `task_*.json + task_*.log`
   - `toon` -> `task_*.json + task_*.toon`
   - `text+toon` -> `task_*.json + task_*.log + task_*.toon`

## 7. queue 身份约定

1. `batch_id` 是队列代号，不是稳定历史身份
2. `tidy-batch` / `tidy-refresh` / rebase 之后必须重新解析 `tasks/` 当前队列
3. `queue_requires_reresolve=true` 是硬约束，不能继续沿用旧 `batch_id` / `task_id`
4. 若 `replacement_queue_head` / `queue_transition_summary` 出现，表示 refresh 已把旧选择替换为新的 queue head
5. `tidy-step` 在“当前 batch 只剩 1 个 task”时只关闭该 task，不再自动串联 `tidy-batch --preset sop`

## 8. 延伸阅读

1. [../README.md](../README.md)
2. [../command_map/README.md](../command_map/README.md)
3. [../tidy/README.md](../tidy/README.md)
4. [../validate/README.md](../validate/README.md)
5. [../workflows/README.md](../workflows/README.md)
6. [../notes/README.md](../notes/README.md)
7. [../history/README.md](../history/README.md)

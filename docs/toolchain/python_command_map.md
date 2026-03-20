# Python Toolchain Command Map

本文档用于快速定位 Python 工具链修改点，避免在 `scripts/`、`test/`、wrapper 脚本之间改错层。

## 1. 入口链路

1. `tools/run.py`
   - 统一入口，构建 `Context`，分发到 CLI 子命令
2. `tools/toolchain/cli/parser.py`
   - 注册全部子命令与通用参数
3. `tools/toolchain/cli/handlers/*.py`
   - 每个命令的参数定义与入参组装（build/verify/tidy 等）
4. `tools/toolchain/commands/**`
   - 真实业务执行逻辑（调用 cmake、test runner、配置同步等）
5. `tools/toolchain/core/**`
   - 基础设施：配置模型、上下文、进程执行器

## 2. 需求 -> 修改位置速查

1. 调整 `build/verify/tidy` 命令参数（新增/改名/默认值）
   - 先改：`tools/toolchain/cli/handlers/<command>.py`
   - 再看是否需要改：`tools/toolchain/commands/**`

2. 调整构建行为（自动 configure、`--target` 注入、build dir、平台分支）
   - 先改：`tools/toolchain/commands/cmd_build/cmake.py`
   - 关联：`tools/toolchain/commands/cmd_build/command.py`

3. 调整 profile 默认策略（如 `cmake_args`、`build_targets`、`BUILD_TESTING`）
   - 先改：`tools/toolchain/config/build.toml`
   - 字段模型：`tools/toolchain/core/config.py`
   - 读取适配：`tools/toolchain/commands/cmd_build/common/profile_backend.py`

4. 调整 verify 流程（build + test 转发规则）
   - 主入口：`tools/run.py verify`
   - 参数层：`tools/toolchain/cli/handlers/verify.py`
   - 执行层：`tools/toolchain/commands/cmd_quality/verify.py`

5. 调整 Windows CLI Python 构建入口参数（release/runtime sync/icon 覆盖）
   - `tools/toolchain/cli/handlers/build.py`
   - `tools/toolchain/commands/cmd_build/command.py`
   - `tools/toolchain/commands/cmd_build/cargo.py`

6. 调整 clang-tidy 第三方头过滤
   - 配置优先：`tools/toolchain/config/workflow.toml` -> `[tidy].header_filter_regex`
   - 默认回退：`tools/toolchain/commands/cmd_build/cmake.py`

7. 执行 clang-tidy 批次收口（统一入口）
   - 命令（C++ 轨）：`python tools/run.py tidy-batch --app tracer_core_shell --batch-id <BATCH_ID> --strict-clean --run-verify --concise --full-every 3 --keep-going`
   - 参数层：`tools/toolchain/cli/handlers/tidy_batch.py`
   - 执行层：`tools/toolchain/commands/tidy/batch.py`

8. 针对单个 clang-tidy task 做自动 patch / fix / suggest / step
   - 命令：
     - `python tools/run.py tidy-task-patch --app tracer_core_shell --batch-id <BATCH_ID> --task-id <TASK_ID>`
     - `python tools/run.py tidy-task-fix --app tracer_core_shell --batch-id <BATCH_ID> --task-id <TASK_ID>`
     - `python tools/run.py tidy-task-suggest --app tracer_core_shell --batch-id <BATCH_ID> --task-id <TASK_ID>`
     - `python tools/run.py tidy-step --app tracer_core_shell --batch-id <BATCH_ID> --task-id <TASK_ID>`
   - 参数层：
     - `tools/toolchain/cli/handlers/tidy/tidy_task_fix.py`
     - `tools/toolchain/cli/handlers/tidy/tidy_task_patch.py`
     - `tools/toolchain/cli/handlers/tidy/tidy_task_suggest.py`
     - `tools/toolchain/cli/handlers/tidy/tidy_step.py`
   - 执行层：
     - `tools/toolchain/commands/tidy/task_log.py`
     - `tools/toolchain/commands/tidy/task_auto_fix.py`
     - `tools/toolchain/commands/tidy/task_fix.py`
     - `tools/toolchain/commands/tidy/task_patch.py`
     - `tools/toolchain/commands/tidy/task_suggest.py`
     - `tools/toolchain/commands/tidy/step.py`

## 3. 最小回归命令

```bash
# Windows build/bin 产出 core runtime DLL
python tools/run.py build --app tracer_core --profile release_bundle --build-dir build --runtime-platform windows

# 基于 windows/build/bin 编译 Rust CLI
python tools/run.py build --app tracer_windows_rust_cli --profile release_bundle --build-dir build --runtime-platform windows

# C++ 轨专用（clang-tidy / build verify）
python tools/run.py build --app tracer_core --profile fast --concise
python tools/run.py verify --app tracer_core --profile fast --concise
python tools/run.py tidy --app tracer_core_shell -- --target tidy_all
python tools/run.py tidy-batch --app tracer_core_shell --batch-id <BATCH_ID> --strict-clean --run-verify --concise --full-every 3 --keep-going
python tools/run.py tidy-task-patch --app tracer_core_shell --source-scope core_family --tidy-build-dir build_tidy_core_family --batch-id <BATCH_ID> --task-id <TASK_ID>
python tools/run.py tidy-step --app tracer_core_shell --source-scope core_family --tidy-build-dir build_tidy_core_family --batch-id <BATCH_ID> --task-id <TASK_ID> --dry-run
```

## 4. 相关导航

1. `tools/AGENTS.md`
2. `tools/README.md`
3. `tools/toolchain/README.md`
4. `docs/toolchain/clang_tidy_architecture.md`
5. `docs/toolchain/clang_tidy_flow.md`
6. `docs/toolchain/clang_tidy_sop.md`

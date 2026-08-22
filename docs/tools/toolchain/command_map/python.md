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
   - 参数层：`tools/toolchain/cli/handlers/quality/verify.py`
   - 执行层：`tools/toolchain/commands/cmd_quality/verify.py`

5. Android 编译、安装和测试数据注入流程
   - 编译：`python tools/run.py android --variant debug`
   - 编译并安装：`python tools/run.py android --variant debug --install`
   - 编译、安装并注入测试数据：`python tools/run.py android --variant debug --install --with-test-data`
   - 安装已有 APK：`python tools/run.py android --variant debug --install-only`
   - Release 编译与安装：`android --variant release`、`android --variant release --install`
   - 多设备时追加：`--serial DEVICE_SERIAL`
   - `--with-test-data` 只注入 TXT 与层级测试文件；只有显式附加
     `--rebuild-database` 才会解析全部 TXT 并重建数据库。
   - 参数与设备流程：`tools/toolchain/cli/handlers/android.py`
   - 测试数据注入：复用 `tools/scripts/devtools/android/push_test_data.py`

6. Android 全模块 Detekt 静态分析
   - 命令：`python tools/run.py android-detekt --concise`
   - 该快捷命令会依次运行 `app`、`contract`、`feature-data`、
     `feature-insights`、`feature-record`、`feature-ui-common`、`runtime`
     的 `detekt` task；额外参数可放在命令末尾转发给 Gradle。
   - 参数层：`tools/toolchain/cli/handlers/android_detekt.py`
   - 任务配置：`tools/toolchain/config/build.toml` 的
     `[build.profiles.android_detekt]`

7. Android 模块或单测类定向测试
   - 命令：`python tools/run.py android-test --module feature-insights --tests com.example.tracer.QueryInsightsResultDisplayRobolectricTest`
   - `--module` 支持 `app`、`contract`、所有 `feature-*` 模块和 `runtime`；
     省略 `--tests` 时运行该模块全部 debug unit tests，重复 `--tests` 可筛选类或方法。
   - 参数与执行：`tools/toolchain/cli/handlers/android_test.py`

8. 调整 Windows CLI Python 构建入口参数（release/runtime sync/icon 覆盖）
   - `tools/toolchain/cli/handlers/build.py`
   - `tools/toolchain/commands/cmd_build/command.py`
   - `tools/toolchain/commands/cmd_build/cargo.py`

9. 调整 clang-tidy 第三方头过滤
   - 配置优先：`tools/toolchain/config/workflow.toml` -> `[tidy].header_filter_regex`
   - 默认回退：`tools/toolchain/commands/cmd_build/cmake.py`

10. 执行 clang-tidy 队列收口（统一入口）
   - 命令（C++ 轨）：`python tools/run.py tidy-close --app tracer_core_shell --source-scope core_family --tidy-build-dir build_tidy_core_family --keep-going --concise`
   - 参数层：`tools/toolchain/cli/handlers/tidy/tidy_close.py`
   - 执行层：`tools/toolchain/commands/tidy/execution/close.py`

11. 针对单个 clang-tidy task 做自动 fix / suggest / source-step
   - 命令：
     - `python tools/run.py tidy-task-fix --task-log <resolved_task_json>`
     - `python tools/run.py tidy-task-suggest --task-log <resolved_task_json>`
     - `python tools/run.py tidy-source-step --task-log <resolved_task_json>`
   - 约束：
     - `app` / `tidy workspace` / `source scope` 全部从 `--task-log` 路径反推
   - 参数层：
     - `tools/toolchain/cli/handlers/tidy/tidy_task_fix.py`
     - `tools/toolchain/cli/handlers/tidy/tidy_task_suggest.py`
     - `tools/toolchain/cli/handlers/tidy/tidy_source_step.py`
   - 执行层：
     - `tools/toolchain/commands/tidy/queue/task_log.py`
     - `tools/toolchain/commands/tidy/queue/task_auto_fix.py`
     - `tools/toolchain/commands/tidy/queue/task_fix.py`
     - `tools/toolchain/commands/tidy/queue/task_suggest.py`
     - `tools/toolchain/commands/tidy/execution/source_step.py`

12. Clang 底层工具适配
   - `tools/toolchain/commands/clang/format.py`
   - `tools/toolchain/commands/clang/tidy/`
   - `tools/toolchain/commands/clang/clangd/`
   - task queue 和 tidy workflow 仍归 `tools/toolchain/commands/tidy/`

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
python tools/run.py tidy-source-step --task-log <resolved_task_json> --dry-run
```

## 4. 相关导航

1. `tools/AGENTS.md`
2. `tools/README.md`
3. `docs/tools/toolchain/README.md`
4. `docs/tools/toolchain/tidy/README.md`
5. `docs/tools/toolchain/tidy/architecture.md`
6. `docs/tools/toolchain/tidy/flow.md`
7. `docs/tools/toolchain/tools/README.md`
8. `docs/tools/toolchain/test/README.md`
9. `docs/tools/toolchain/workflows/README.md`

# Windows Rust CLI 结构规范

本文档定义 `apps/tracer_cli/windows/rust_cli` 的稳定分层与改动路由。

## 1. 分层与职责

1. 入口层：`src/main.rs`
   - 负责进程入口、UTF-8 控制台初始化、全局错误出口。
2. 参数层：`src/cli/mod.rs`
   - 负责 `clap` 命令模型、参数校验、`--help/--version` 行为。
3. 分发层：`src/commands/mod.rs`、`src/commands/handler.rs`
   - 负责命令到 handler 的路由，不承载业务逻辑。
4. 命令实现层：`src/commands/handlers/*.rs`
   - 每个命令独立处理参数组装、调用 runtime、渲染输出。
5. Core 适配层：`src/core/runtime.rs`
   - 负责动态加载 `tracer_core.dll` 并调用 C ABI。
6. 错误模型层：`src/error/mod.rs`
   - 统一 `AppError` 与 `AppExitCode`。
7. 许可证元数据：`src/licenses.rs`
   - 负责 `licenses` 与 `licenses --full` 文本来源。
8. 运行时资源：`runtime/config/`、`runtime/assets/`
   - 交付时随 CLI 一起复制。

## 2. 稳定执行链路

1. `main` 收集参数并调用 `parse_cli`。
2. `commands::execute` 进入分发层。
3. 对应 handler 调用 `core/runtime.rs`。
4. runtime 返回文本或 JSON，再由 handler 输出。
5. 统一错误在 `main` 转换为退出码。

## 3. 改动路由（Agent 快速表）

1. 新增命令：
   - `src/cli/mod.rs` 增加子命令定义。
   - `src/commands/mod.rs` 增加路由。
   - `src/commands/handlers/` 增加实现文件。
2. 改参数校验或帮助文案：
   - `src/cli/mod.rs`
3. 改 Core 调用或字段映射：
   - `src/core/runtime.rs`
4. 改错误码或错误文本：
   - `src/error/mod.rs`
   - `src/main.rs`（解析错误映射）
5. 改 licenses 输出：
   - `src/licenses.rs`
   - `src/commands/handlers/licenses.rs`

## 4. 测试契约落点

1. 套件入口：`test/suites/tracer_windows_rust_cli/tests.toml`
2. 命令集：`test/suites/tracer_windows_rust_cli/tests/command_groups.toml`
3. 细分用例：
   - `commands_tree_version.toml`
   - `commands_query_data.toml`
   - `commands_crypto.toml`
   - `commands_failure_modes.toml`
4. 报告 gate：`test/suites/tracer_windows_rust_cli/tests/gate_cases.toml`

## 5. 最小验证命令

```powershell
python tools/run.py build --app tracer_windows_rust_cli --build-dir build_fast --runtime-platform windows
python tools/run.py verify --app tracer_core --build-dir build_fast --scope artifact --concise
```

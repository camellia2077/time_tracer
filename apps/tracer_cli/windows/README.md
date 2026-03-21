# tracer_windows_rust_cli

Windows CLI 当前交付实现为 Rust-only 版本。

## 目录

1. `apps/tracer_cli/windows/rust_cli/`
   - Rust CLI 主工程
   - 负责参数解析、命令分发、Core C ABI 调用、运行时装配
2. `apps/tracer_cli/windows/agent.md`
   - 本模块的 agent 约束与验证入口
3. `docs/time_tracer/clients/windows_cli/README.md`
   - Windows CLI 文档入口

## 常用命令

```powershell
# 阶段性批量验证
python tools/run.py verify --app tracer_core --quick --scope batch --concise

# 默认发布构建入口：先编 Windows runtime，再编 Rust CLI
python tools/run.py build --app tracer_core --profile release_bundle --build-dir build --runtime-platform windows
python tools/run.py build --app tracer_windows_rust_cli --profile release_bundle --build-dir build --runtime-platform windows
```

## 结果文件

- 主结果：`out/test/artifact_windows_cli/result.json`
- 用例结果：`out/test/artifact_windows_cli/result_cases.json`
- 聚合日志：`out/test/artifact_windows_cli/logs/output.log`

## Runtime Config Boundary

- Canonical shared source：`assets/tracer_core/config`
- Windows generated runtime copy：`apps/tracer_cli/windows/rust_cli/runtime/config`

## Canonical Commands

- `query data`
- `query tree`
- `report render`
- `report export`
- `exchange export/import/inspect`
- `pipeline convert/import/ingest/validate`

## Removed Compat Surface

- `blink`
- `zen`
- `--database`
- `--out`
- `--project`
- `remark-day`
- `sensitive`

## Agent 修改定位

1. CLI 入口
   - `apps/tracer_cli/windows/rust_cli/src/main.rs`
2. 参数模型
   - `apps/tracer_cli/windows/rust_cli/src/cli/*.rs`
3. 命令分发
   - `apps/tracer_cli/windows/rust_cli/src/commands/mod.rs`
4. 命令处理
   - `apps/tracer_cli/windows/rust_cli/src/commands/handlers/pipeline/*`
   - `apps/tracer_cli/windows/rust_cli/src/commands/handlers/query/*`
   - `apps/tracer_cli/windows/rust_cli/src/commands/handlers/report/*`
   - `apps/tracer_cli/windows/rust_cli/src/commands/handlers/exchange/*`
   - `apps/tracer_cli/windows/rust_cli/src/commands/handlers/chart/*`
5. Core ABI 调用
   - `apps/tracer_cli/windows/rust_cli/src/core/runtime.rs`
   - `apps/tracer_cli/windows/rust_cli/src/core/runtime/*.rs`
6. 错误模型
   - `apps/tracer_cli/windows/rust_cli/src/error/mod.rs`

## 文档驱动定位

1. 总入口
   - `docs/time_tracer/clients/windows_cli/README.md`
2. 结构与职责分层
   - `docs/time_tracer/clients/windows_cli/specs/STRUCTURE.md`
3. Core C ABI 契约
   - `docs/time_tracer/core/contracts/c_abi.md`

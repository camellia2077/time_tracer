# tracer_windows_rust_cli

Windows CLI 当前交付实现为 Rust-only 版本。

## 目录

1. `apps/cli/windows/rust/`
   - Rust CLI 主工程
   - 负责参数解析、命令分发、Core C ABI 调用、运行时装配
2. `apps/cli/windows/agent.md`
   - 本模块的 agent 约束与验证入口
3. `docs/time_tracer/clients/windows_cli/README.md`
   - Windows CLI 文档入口

## 常用命令

```powershell
# 阶段性批量验证
python tools/run.py verify --app tracer_core --concise

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
- Windows generated runtime copy：`apps/cli/windows/rust/runtime/config`

## Canonical Commands

- `query data`
- `query tree`
- `report render`
- `report export`
- `exchange export/import/inspect`
- `pipeline convert/import/ingest/validate`

## Recent Fixed Window

- `recent` 默认仍然以“当天”为锚点。
- 若需要固定逻辑当天窗口，可用 `--as-of`（仅 `recent` 支持）：

```powershell
time_tracer_cli report render recent 7 --as-of 2026-03-07 --format md --db <db_path>
time_tracer_cli report export recent 7 --as-of 2026-03-07 --format md --db <db_path> --output <out_dir>
```

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
   - `apps/cli/windows/rust/src/main.rs`
2. 参数模型
   - `apps/cli/windows/rust/src/cli/*.rs`
3. 命令分发
   - `apps/cli/windows/rust/src/commands/mod.rs`
4. 命令处理
   - `apps/cli/windows/rust/src/commands/handlers/pipeline/*`
   - `apps/cli/windows/rust/src/commands/handlers/query/*`
   - `apps/cli/windows/rust/src/commands/handlers/report/*`
   - `apps/cli/windows/rust/src/commands/handlers/exchange/*`
   - `apps/cli/windows/rust/src/commands/handlers/chart/*`
5. Core ABI 调用
   - `apps/cli/windows/rust/src/core/runtime.rs`
   - `apps/cli/windows/rust/src/core/runtime/*.rs`
6. 错误模型
   - `apps/cli/windows/rust/src/error/mod.rs`

## 文档驱动定位

1. 总入口
   - `docs/time_tracer/clients/windows_cli/README.md`
2. 结构与职责分层
   - `docs/time_tracer/clients/windows_cli/specs/STRUCTURE.md`
3. Core C ABI 契约
   - `docs/time_tracer/core/contracts/c_abi.md`

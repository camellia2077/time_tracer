---
description: Windows Rust CLI capability-family layout, build, and verification entrypoints
---

## 当前范围

- 唯一 CLI 实现：`apps/cli/windows/rust/`
- 运行时核心：`tracer_core.dll`
- Rust-side runtime session / capability client root：
  `apps/cli/windows/rust/src/core/runtime.rs`

## 必跑流程

1. 标准验证：
   - `python tools/run.py verify --app tracer_core --scope batch --concise`
2. 如需显式确认 Windows runtime + Rust CLI 构建：
   - `python tools/run.py build --app tracer_core --profile release_bundle --build-dir build --runtime-platform windows`
   - `python tools/run.py build --app tracer_windows_rust_cli --profile release_bundle --build-dir build --runtime-platform windows`
3. 结果检查：
   - `out/test/artifact_windows_cli/result.json`
   - `out/test/artifact_windows_cli/logs/output.log`
4. 通过标准：
   - `out/test/artifact_windows_cli/result.json` 中应包含 `"success": true`

## Agent 快速定位

1. CLI 入口与参数解析：
   - `apps/cli/windows/rust/src/main.rs`
   - `apps/cli/windows/rust/src/cli/*.rs`
2. 命令分发：
   - `apps/cli/windows/rust/src/commands/mod.rs`
   - `apps/cli/windows/rust/src/commands/handler.rs`
3. 命令实现：
   - `apps/cli/windows/rust/src/commands/handlers/pipeline/*`
   - `apps/cli/windows/rust/src/commands/handlers/query/*`
   - `apps/cli/windows/rust/src/commands/handlers/report/*`
   - `apps/cli/windows/rust/src/commands/handlers/exchange/*`
   - `apps/cli/windows/rust/src/commands/handlers/chart/*`
4. Core C ABI 桥接：
   - `apps/cli/windows/rust/src/core/runtime.rs`
   - `apps/cli/windows/rust/src/core/runtime/*.rs`
5. 错误模型与错误文案：
   - `apps/cli/windows/rust/src/error/mod.rs`
6. 运行时模板与配置：
   - `apps/cli/windows/rust/runtime/assets/`
   - `apps/cli/windows/rust/runtime/config/`
   - canonical source：`assets/tracer_core/config`
7. 输出契约测试入口：
   - `test/suites/tracer_windows_rust_cli/tests/command_groups.toml`
   - `test/suites/tracer_windows_rust_cli/tests/commands_tree_version.toml`
   - `test/suites/tracer_windows_rust_cli/tests/commands_query_data.toml`
   - `test/suites/tracer_windows_rust_cli/tests/commands_exchange.toml`

## Canonical 命令面

- `query data`
- `query tree`
- `report render`
- `report export`
- `exchange export/import/inspect`
- `pipeline convert/import/ingest/validate`

## 已移除兼容入口

- `blink`
- `zen`
- `--database`
- `--out`
- `--project`
- `remark-day`
- `sensitive`

## 文档优先定位

1. 总索引：
   - `docs/time_tracer/clients/windows_cli/README.md`
2. 结构与分层：
   - `docs/time_tracer/clients/windows_cli/specs/STRUCTURE.md`
3. Core ABI 契约：
   - `docs/time_tracer/core/contracts/c_abi.md`

## 修改约束

- 使用 `tools/run.py` 作为统一入口
- 不要重新引入已归档前端实现的运行时依赖
- 测试输入源固定为 `test/data`
- 临时产物放在 `temp/`
- `ingest` 的持久化语义必须保持严格边界：
  - 只有全部校验通过后，才允许创建数据库并写入
  - 若运行前数据库不存在，则失败的 ingest 不得留下新的 `.sqlite3` / `-wal` / `-shm` / `-journal`

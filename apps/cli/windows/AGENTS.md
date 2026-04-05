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
   - `python tools/run.py verify --app tracer_core --concise`
2. 如需显式确认 Windows runtime + Rust CLI 构建：
   - `python tools/run.py build --app tracer_core --profile release_bundle --build-dir build --runtime-platform windows`
   - `python tools/run.py build --app tracer_windows_rust_cli --profile release_bundle --build-dir build --runtime-platform windows`
3. 结果检查：
   - `out/test/artifact_windows_cli/result.json`
   - `out/test/artifact_windows_cli/logs/output.log`
4. 通过标准：
   - `out/test/artifact_windows_cli/result.json` 中应包含 `"success": true`

## Build 顺序约束

- 如果改动落在 `libs/tracer_core/`、`apps/tracer_core_shell/` 或任何会进入
  `tracer_core.dll` 的 native/core 代码，不能只执行
  `--app tracer_windows_rust_cli`。
- 原因：`python tools/run.py build --app tracer_windows_rust_cli ...` 主要负责
  构建 Rust CLI，并同步当前已有的 runtime 文件；它不会替你重新编译一遍最新
  的 core native 产物。
- 正确顺序是先重建 core，再重建 CLI：
  - `python tools/run.py build --app tracer_core --profile release_bundle --build-dir build --runtime-platform windows`
  - `python tools/run.py build --app tracer_windows_rust_cli --profile release_bundle --build-dir build --runtime-platform windows`
- 否则很容易出现 Rust CLI 已更新、但实际加载的 `tracer_core.dll` 仍是旧版本，
  从而让 `exchange import`、`validate`、`query` 之类能力表现得像“修复没生效”。

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
   - `test/suites/tracer_windows_rust_cli/tests.toml`
   - `test/suites/tracer_windows_rust_cli/tests/commands_reporting.toml`
   - `test/suites/tracer_windows_rust_cli/tests/commands_pipeline.toml`
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

## Recent 固定窗口

- `report render recent` / `report export recent` 新增 `--as-of YYYY-MM-DD`（仅 recent 支持）。
- CLI 会把该请求在本地转换为固定 `range` 查询窗口，不改 C ABI / core 契约。
- 示例：
  - `time_tracer_cli report render recent 7 --as-of 2026-03-07 --format md --db <db_path>`
  - `time_tracer_cli report export recent 7 --as-of 2026-03-07 --format md --db <db_path> --output <out_dir>`

## Exchange Import 语义

- `exchange import` 会覆盖 runtime active converter config（`main` / `alias_mapping` / `duration_rules`）。
- 配置覆盖后的运行态刷新由 core 负责完成，CLI 不应再维护独立的缓存失效补丁。
- 若你改动 import 链路，至少补一条“导入后立刻 validate/query 使用新 config”的回归。

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

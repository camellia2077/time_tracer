# Windows Rust CLI 文档索引

本目录是 `tracer_windows_rust_cli` 的文档主入口。

## 范围

1. 命令语义、参数模型与输出契约。
2. Windows 运行时交付（`time_tracer_cli.exe` + `tracer_core.dll` + runtime config/assets）。
3. 测试套件 `tracer_windows_rust_cli` 的约束与验收口径。
4. `exchange import` 会覆盖 active converter config，且由 core 在导入流程内立即刷新运行态配置视图。

## 快速导航

1. 结构与改动落点：`specs/STRUCTURE.md`
2. 输出文案与解析约定：`specs/cli-output-style.md`
3. 颜色与终端显示：`specs/console-color.md`
4. 用户可见变更：`CHANGELOG.md`
5. 历史阶段记录：`history.md`

## 对应代码目录

1. `apps/cli/windows/rust/src/`
2. `apps/cli/windows/rust/runtime/`
3. `apps/cli/windows/scripts/`

## 测试与验收

1. 集成套件目录：`test/suites/tracer_windows_rust_cli/`
2. 推荐命令：
   - `python tools/run.py verify --app tracer_core_shell --build-dir build_fast --concise`
   - `python tools/run.py verify --app tracer_core --profile fast --concise`
3. 验收结果目录：`out/test/artifact_windows_cli/`
4. 通过判定：`out/test/artifact_windows_cli/result.json` 中 `success=true`

## Report 日期参数约定

1. `report render day` / `report export day`
   - 接受 `YYYYMMDD` 或 `YYYY-MM-DD`
   - CLI 会在调用 runtime 前统一归一化为 ISO `YYYY-MM-DD`
2. `report render month` / `report export month`
   - 接受 `YYYYMM` 或 `YYYY-MM`
   - CLI 会在调用 runtime 前统一归一化为 ISO `YYYY-MM`
3. `report render range`
   - 接受 `<from>|<to>`
   - 两端都接受 `YYYYMMDD` 或 `YYYY-MM-DD`
   - CLI 会在调用 runtime 前把两端分别归一化为 ISO，再拼成 `YYYY-MM-DD|YYYY-MM-DD`
4. `report render/export week`、`year`、`recent`
   - 继续使用各自 canonical 参数形式，不做 day/month 风格的紧凑日期归一化

## Reporting Fixture Range

1. `test/data/` 当前共享报表 fixture 覆盖范围是 `2025-01-01` 到 `2026-12-31`。
2. 编写 `tracer_windows_rust_cli` reporting suite 目标日期时，应保证 canonical ISO 目标落在这个闭区间内。
3. 若扩展 fixture 年份范围，需同步更新：
   - `test/data/README.md`
   - `test/suites/tracer_windows_rust_cli/tests/commands_reporting.toml`

## 文档维护规则

1. 改 CLI 参数或命令：同步更新 `specs/STRUCTURE.md` 与对应测试 TOML。
2. 改输出文本：同步更新 `specs/cli-output-style.md` 与测试断言。
3. 改 C ABI 交互：同步更新 `docs/time_tracer/core/contracts/c_abi.md`。
4. 改 `exchange import` 的配置应用语义：同步更新 `test/suites/tracer_windows_rust_cli/tests/commands_exchange.toml` 中“导入后立即 validate/query”回归。
5. 本目录不再维护旧前端主线说明；历史仅在归档文档中保留。

## Ingest 持久化边界

1. `ingest` 必须先完成输入收集、解析、结构校验与逻辑校验，再进入数据库持久化阶段。
2. 若运行前数据库不存在，则失败的 `ingest` 不得留下新的 `.sqlite3`、`-wal`、`-shm` 或 `-journal`。
3. Windows CLI 不应依赖“失败后删除空库文件”的补偿逻辑；该规则由 core/runtime 持久化边界直接保证。
4. `query` / `report` 在数据库缺失时应返回明确错误，而不是隐式建库掩盖问题。

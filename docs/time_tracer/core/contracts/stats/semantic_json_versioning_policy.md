# Semantic JSON Versioning Policy

## 目标
1. 保证 `semantic_json` 可被 CLI/Android/脚本稳定消费。
2. 在演进字段时避免破坏既有集成链路。

## v1 基线要求
1. 顶层包络字段必须存在：
   - `schema_version`
   - `action`
   - `output_mode`（固定 `semantic_json`）
2. `schema_version=1` 时，`action` 语义必须与 `DataQueryAction` 一致。
3. 未命中数据时，仍返回结构化空结果，不返回缺字段或非对象 payload。

## 变更分级
1. 非破坏性变更（允许直接在 v1 追加）：
   - 追加新字段（可选字段）。
   - 追加新 action（不影响旧 action 字段语义）。
2. 破坏性变更（必须升级 schema 大版本）：
   - 删除字段。
   - 重命名字段。
   - 变更字段类型。
   - 变更既有字段统计口径。

## 版本升级流程（v1 -> v2）
1. 新增 v2 文档：
   - `capability_contract_v2.md`
   - `json_schema_v2.md`
2. 在迁移窗口内保持双版本可读（至少一个发布周期）。
3. 在 `docs/time_tracer/history/` 明确：
   - 起始版本、结束版本、迁移建议。

## 端侧消费约束
1. Android/CLI 默认仅依赖已发布版本文档中的字段。
2. 端侧不可推断未声明字段语义（包括默认值与单位）。
3. 对未知字段按“忽略但不失败”处理，保证前向兼容。

## 测试门禁
1. `apps/tracer_core/src/infrastructure/tests/data_query/data_query_refactor_period_tests.cpp`
2. `apps/tracer_core/src/infrastructure/tests/data_query/data_query_refactor_tree_tests.cpp`
3. `apps/tracer_core/src/infrastructure/tests/data_query/data_query_refactor_stats_tests.cpp`
   - 三个测试入口合计必须覆盖 `schema_version/action/output_mode` 基础包络。
   - 必须覆盖关键 action 的字段快照（至少 `days-stats`、`report-chart`、`tree`）。
4. `test/suites/tracer_windows_cli/tests/commands_query_data.toml`
   - 必须覆盖 `--data-output json` 的关键 fallback 场景。

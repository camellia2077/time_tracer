# Core Stats 文档索引

本目录定义 `time_tracer` 内核统计能力的跨端契约，用于 CLI / Android / 脚本工具统一复用。

## Migration Note
1. Capability-first query routing now starts at
   `docs/time_tracer/core/capabilities/query/README.md`.
2. This directory still holds the detailed stats/query contract set.

## Group Indexes
1. `docs/time_tracer/core/contracts/stats/capability/README.md`
   - 能力定义、能力矩阵、`report-chart` 契约。
2. `docs/time_tracer/core/contracts/stats/semantic_json/README.md`
   - `semantic_json` schema 与版本策略。
3. `docs/time_tracer/core/contracts/stats/adapters/README.md`
   - Core/CLI/Android adapter 代码落点与评审清单。

## Flat Docs Retained For Compatibility
1. `docs/time_tracer/core/contracts/stats/report_chart_contract_v1.md`
2. `docs/time_tracer/core/contracts/stats/capability_contract_v1.md`
3. `docs/time_tracer/core/contracts/stats/json_schema_v1.md`
4. `docs/time_tracer/core/contracts/stats/capability_matrix_v1.md`
5. `docs/time_tracer/core/contracts/stats/semantic_json_versioning_policy.md`
6. `docs/time_tracer/core/contracts/stats/adapter_code_map.md`
7. `docs/time_tracer/core/contracts/stats/adapter_reviewer_checklist.md`
8. `docs/time_tracer/core/contracts/stats/code_map.md`

## 使用建议
1. 先读 `capability/README.md`，再读 `semantic_json/README.md`，最后看 `adapters/README.md`。
2. 新增统计字段时，必须同步更新契约文档和测试基线。
3. 默认输出模式仍为 `text`，跨端集成建议优先使用 `semantic_json`。
4. 变更 `report-chart` 字段时，先看 `report_chart_contract_v1.md`，再更新 schema 与测试基线。

# DataQuery Refactor Completion (v1)

## 目标边界
1. 统计计算：`stats/`
2. 时间范围编排：`orchestrators/`
3. 格式渲染：`renderers/`
4. 入口适配：Core/CLI/Android adapter

## 完成清单
1. 兼容壳下线完成：
   - `data_query_output.cpp`
   - `data_query_semantic.cpp/.hpp`
   - `data_query_statistics.cpp`
2. Core 主链路收口到：
   - `apps/tracer_core/src/infrastructure/query/data/stats/`
   - `apps/tracer_core/src/infrastructure/query/data/orchestrators/`
   - `apps/tracer_core/src/infrastructure/query/data/renderers/`
3. CMake 编译清单移除旧兼容入口，改为新目录编译项。
4. 契约文档补齐：
   - `capability_contract_v1.md`
   - `json_schema_v1.md`
   - `capability_matrix_v1.md`
   - `semantic_json_versioning_policy.md`
   - `adapter_code_map.md`
   - `adapter_reviewer_checklist.md`
5. 自动化测试补齐：
   - Core refactor 回归：
     - `apps/tracer_core/src/infrastructure/tests/data_query/data_query_refactor_period_tests.cpp`
     - `apps/tracer_core/src/infrastructure/tests/data_query/data_query_refactor_tree_tests.cpp`
     - `apps/tracer_core/src/infrastructure/tests/data_query/data_query_refactor_stats_tests.cpp`
   - CLI query-data 场景回归：`commands_query_data.toml`

## 稳态维护规则
1. 统计口径变更先改 `stats/`，再改文档与测试，不允许端侧先行分叉。
2. adapter 层只承载 IO/映射/展示，不承载统计与编排公式。
3. `semantic_json` 破坏性变更必须升级 schema 大版本。

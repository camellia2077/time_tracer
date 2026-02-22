# Core 文档域

本目录用于承载 TimeTracer 内核（领域 + 应用 + 端口契约）相关文档。

## 范围
1. 领域模型、业务规则、用例编排。
2. 核心接口（ports/dto）与跨端契约。
3. 与具体 UI 或平台无关的设计说明。

## 不在此处
1. Windows CLI 参数交互、命令行展示细节。
2. Android Compose UI、页面状态管理。
3. 构建机或 IDE 个人使用说明。

## 目录分层（按职责边界）
1. `docs/time_tracer/core/contracts/`
   - 对外契约、错误语义、统计契约与 schema。
2. `docs/time_tracer/core/ingest/`
   - 导入数据结构与转换算法约束。
3. `docs/time_tracer/core/architecture/`
   - Core 边界、模块职责与重构约束。

## 当前权威文档（沿用）

### Core 分层文档
1. `docs/time_tracer/core/contracts/README.md`
2. `docs/time_tracer/core/contracts/c_abi.md`
3. `docs/time_tracer/core/contracts/error-model.md`
4. `docs/time_tracer/core/contracts/error-codes.md`
5. `docs/time_tracer/core/contracts/stats/README.md`
6. `docs/time_tracer/core/contracts/stats/capability_contract_v1.md`
7. `docs/time_tracer/core/contracts/stats/json_schema_v1.md`
8. `docs/time_tracer/core/contracts/stats/capability_matrix_v1.md`
9. `docs/time_tracer/core/contracts/stats/semantic_json_versioning_policy.md`
10. `docs/time_tracer/core/contracts/stats/adapter_code_map.md`
11. `docs/time_tracer/core/contracts/stats/adapter_reviewer_checklist.md`
12. `docs/time_tracer/core/contracts/stats/code_map.md`
13. `docs/time_tracer/core/ingest/README.md`
14. `docs/time_tracer/core/ingest/ingest_data_structures.md`
15. `docs/time_tracer/core/ingest/ingest_conversion_algorithms.md`
41. `docs/time_tracer/core/architecture/README.md`
42. `docs/time_tracer/core/architecture/data_lifecycle_parsing_to_storage.md`
43. `docs/time_tracer/core/architecture/domain_model_and_rules.md`
44. `docs/time_tracer/core/architecture/application_pipeline_and_ports.md`
45. `docs/time_tracer/core/architecture/infrastructure_persistence.md`
46. `docs/time_tracer/core/architecture/refactor_module_boundaries.md`
47. `docs/time_tracer/core/architecture/data_query/README.md`
48. `docs/time_tracer/core/architecture/data_query/data_query_refactor_completion_v1.md`
49. `docs/time_tracer/core/architecture/data_query/data_query_responsibility_boundaries_v1.md`

### 相关上游文档
1. `docs/time_tracer/design/core_logic.md`
2. `docs/time_tracer/guides/database/database_schema.md`
3. `docs/time_tracer/guides/config/config_reference.md`
4. `docs/time_tracer/guides/native/native_modules.md`
5. `docs/time_tracer/guides/database/parsing/README.md`
6. `docs/time_tracer/guides/database/parsing/01_period_normalization.md`
7. `docs/time_tracer/guides/database/parsing/02_filter_sql_build.md`
8. `docs/time_tracer/guides/database/parsing/03_sql_execution_row_decode.md`
9. `docs/time_tracer/guides/database/parsing/04_semantic_projection.md`
10. `docs/time_tracer/guides/database/generation/tree_algorithms.md`
11. `docs/time_tracer/guides/database/storage/data_structures.md`

## 规则
1. 新增 Core 文档优先放在本目录。
2. 新增文档按职责边界放入 `contracts/ingest/architecture` 对应目录。
3. Core（`domain + application`）不得依赖 `modules/tracer_adapters_io` 等 adapter 实现目录。
4. Core 只通过 `application/ports` 暴露契约，不直接包含 `infrastructure/*` 头。

# DataQuery 职责边界与代码落点（v1）

本文说明 `DataQuery` 在 Core/CLI/Android 中按职责边界的落位方式，便于快速定位代码与后续维护。

## 1. 统计计算（Stats Compute）

职责：只负责统计公式与派生指标计算，不做时间范围推导，不做渲染。

代码位置：
1. `apps/time_tracer/src/infrastructure/query/data/stats/day_duration_stats_calculator.cpp`
2. `apps/time_tracer/src/infrastructure/query/data/stats/day_duration_stats_calculator.hpp`
3. `apps/time_tracer/src/infrastructure/query/data/stats/report_chart_stats_calculator.cpp`
4. `apps/time_tracer/src/infrastructure/query/data/stats/report_chart_stats_calculator.hpp`
5. `apps/time_tracer/src/infrastructure/query/data/stats/stats_models.hpp`

## 2. 时间范围编排（Period Orchestration）

职责：解析 period/range/lookback，组织 action 级查询流程，不写统计公式，不做文本/JSON 渲染。

代码位置：
1. `apps/time_tracer/src/infrastructure/query/data/orchestrators/date_range_resolver.cpp`
2. `apps/time_tracer/src/infrastructure/query/data/orchestrators/date_range_resolver.hpp`
3. `apps/time_tracer/src/infrastructure/query/data/orchestrators/list_query_orchestrator.cpp`
4. `apps/time_tracer/src/infrastructure/query/data/orchestrators/list_query_orchestrator.hpp`
5. `apps/time_tracer/src/infrastructure/query/data/orchestrators/days_stats_orchestrator.cpp`
6. `apps/time_tracer/src/infrastructure/query/data/orchestrators/days_stats_orchestrator.hpp`
7. `apps/time_tracer/src/infrastructure/query/data/orchestrators/report_chart_orchestrator.cpp`
8. `apps/time_tracer/src/infrastructure/query/data/orchestrators/report_chart_orchestrator.hpp`
9. `apps/time_tracer/src/infrastructure/query/data/orchestrators/tree_orchestrator.cpp`
10. `apps/time_tracer/src/infrastructure/query/data/orchestrators/tree_orchestrator.hpp`

## 3. 格式渲染（Rendering）

职责：把统一语义结果渲染为 `text` 或 `semantic_json`，不做 DB 查询与统计计算。

代码位置：
1. `apps/time_tracer/src/infrastructure/query/data/renderers/data_query_renderer.cpp`
2. `apps/time_tracer/src/infrastructure/query/data/renderers/data_query_renderer.hpp`
3. `apps/time_tracer/src/infrastructure/query/data/renderers/text_renderer.cpp`
4. `apps/time_tracer/src/infrastructure/query/data/renderers/text_renderer.hpp`
5. `apps/time_tracer/src/infrastructure/query/data/renderers/semantic_json_renderer.cpp`
6. `apps/time_tracer/src/infrastructure/query/data/renderers/semantic_json_renderer.hpp`

## 4. 入口适配（Entry Adapters）

职责：参数接入、请求转发、结果解码、错误映射；不承载统计公式与时间范围推导。

### 4.1 Core adapter

1. `apps/time_tracer/src/infrastructure/persistence/sqlite_data_query_service.cpp`
2. `apps/time_tracer/src/infrastructure/persistence/sqlite_data_query_service_request.cpp`
3. `apps/time_tracer/src/infrastructure/persistence/sqlite_data_query_service_dispatch.cpp`
4. `apps/time_tracer/src/infrastructure/persistence/sqlite_data_query_service_report_mapping.cpp`
5. `apps/time_tracer/src/infrastructure/persistence/sqlite_data_query_service_period.cpp`

### 4.2 Windows CLI adapter

1. `apps/tracer_windows_cli/src/api/cli/impl/commands/query/data_query_parser.cpp`
2. `apps/tracer_windows_cli/src/api/cli/impl/commands/query/query_command.cpp`
3. `apps/tracer_windows_cli/src/bootstrap/cli_runtime_factory_proxy.cpp`

### 4.3 Android adapter

1. `apps/tracer_android/runtime/src/main/java/com/example/tracer/runtime/controller/RuntimeQueryDelegate.kt`
2. `apps/tracer_android/runtime/src/main/java/com/example/tracer/runtime/NativeRuntimeQueryOps.kt`
3. `apps/tracer_android/runtime/src/main/java/com/example/tracer/runtime/NativeRuntimeController.kt`

## 5. 主链路示例（report-chart）

1. CLI 参数解析：`apps/tracer_windows_cli/src/api/cli/impl/commands/query/data_query_parser.cpp`
2. CLI 转发 Core：`apps/tracer_windows_cli/src/bootstrap/cli_runtime_factory_proxy.cpp`
3. Core 入口接收：`apps/time_tracer/src/infrastructure/persistence/sqlite_data_query_service.cpp`
4. action 分发：`apps/time_tracer/src/infrastructure/persistence/sqlite_data_query_service_dispatch.cpp`
5. 编排器：`apps/time_tracer/src/infrastructure/query/data/orchestrators/report_chart_orchestrator.cpp`
6. report-chart 语义组装：`apps/time_tracer/src/infrastructure/persistence/sqlite_data_query_service_report_mapping.cpp`
7. 统计计算：`apps/time_tracer/src/infrastructure/query/data/stats/report_chart_stats_calculator.cpp`
8. 输出渲染：`apps/time_tracer/src/infrastructure/query/data/renderers/data_query_renderer.cpp`

## 6. 变更时改哪里（快速指引）

1. 改均值/方差/百分位口径：`stats/*.cpp`
2. 改 `period/range/lookback` 解析规则：`orchestrators/date_range_resolver.*`
3. 改 text/semantic_json 字段展示：`renderers/*.cpp`
4. 改 CLI 参数语义：`apps/tracer_windows_cli/src/api/cli/impl/commands/query/data_query_parser.cpp`
5. 改 Android 数据映射：`apps/tracer_android/runtime/src/main/java/com/example/tracer/runtime/NativeRuntimeQueryOps.kt`

## 7. 对应契约与测试

1. 统计能力契约：`docs/time_tracer/core/contracts/stats/capability_contract_v1.md`
2. 能力矩阵：`docs/time_tracer/core/contracts/stats/capability_matrix_v1.md`
3. JSON 字段契约：`docs/time_tracer/core/contracts/stats/json_schema_v1.md`
4. adapter 边界清单：`docs/time_tracer/core/contracts/stats/adapter_reviewer_checklist.md`
5. Core 回归测试：`apps/time_tracer/src/infrastructure/tests/data_query_refactor_tests.cpp`
6. CLI 场景回归：`test/suites/tracer_windows_cli/tests/commands_query_data.toml`

# Core Stats 代码映射

本文件用于快速定位“统计计算、语义输出、端侧消费”的关键代码位置。

## 迁移护栏（Phase 0）
1. 新增统计公式与派生指标优先落在 `apps/time_tracer/src/infrastructure/query/data/stats/`。
2. 新增时间范围解析与 action 编排优先落在 `apps/time_tracer/src/infrastructure/query/data/orchestrators/`。
3. 新增 `text/semantic_json` 输出逻辑优先落在 `apps/time_tracer/src/infrastructure/query/data/renderers/`。
4. `sqlite_data_query_service_*`、CLI、Android 路径仅承担入口适配职责，不新增业务统计实现。

## 契约与守卫文档（Phase 7/8）
1. `docs/time_tracer/core/contracts/stats/capability_matrix_v1.md`
2. `docs/time_tracer/core/contracts/stats/semantic_json_versioning_policy.md`
3. `docs/time_tracer/core/contracts/stats/adapter_code_map.md`
4. `docs/time_tracer/core/contracts/stats/adapter_reviewer_checklist.md`

## Core 契约层
1. `apps/time_tracer/src/application/dto/core_requests.hpp`
   - `DataQueryRequest`、`DataQueryOutputMode` 契约定义。
2. `apps/time_tracer/src/application/dto/core_responses.hpp`
   - 通用文本响应结构（`TextOutput`）。

## Core 查询与统计语义层
1. `apps/time_tracer/src/infrastructure/query/data/data_query_types.hpp`
   - 统计数据结构（`DayDurationRow`、`DayDurationStats`）。
2. `apps/time_tracer/src/infrastructure/query/data/stats/day_duration_stats_calculator.cpp`
   - `days-stats` 的均值、方差、标准差、百分位、MAD 等计算。
3. `apps/time_tracer/src/infrastructure/query/data/stats/report_chart_stats_calculator.cpp`
   - `report-chart` 的日序列聚合与总时长/平均值/活跃天数计算。
4. `apps/time_tracer/src/infrastructure/query/data/renderers/data_query_renderer.cpp`
5. `apps/time_tracer/src/infrastructure/query/data/renderers/text_renderer.cpp`
6. `apps/time_tracer/src/infrastructure/query/data/renderers/semantic_json_renderer.cpp`
   - 统一渲染入口与 text/semantic_json 分层实现。

## Core 数据访问与编排层
1. `apps/time_tracer/src/infrastructure/query/data/data_query_repository.hpp`
2. `apps/time_tracer/src/infrastructure/query/data/data_query_repository.cpp`
3. `apps/time_tracer/src/infrastructure/query/data/data_query_repository_sql.cpp`
   - 数据查询与 root/project 过滤 SQL 逻辑。
4. `apps/time_tracer/src/infrastructure/query/data/orchestrators/date_range_resolver.cpp`
5. `apps/time_tracer/src/infrastructure/query/data/orchestrators/list_query_orchestrator.cpp`
6. `apps/time_tracer/src/infrastructure/query/data/orchestrators/days_stats_orchestrator.cpp`
7. `apps/time_tracer/src/infrastructure/query/data/orchestrators/report_chart_orchestrator.cpp`
8. `apps/time_tracer/src/infrastructure/query/data/orchestrators/tree_orchestrator.cpp`
   - data query action 编排与 period/range 解析。
9. `apps/time_tracer/src/infrastructure/persistence/sqlite_data_query_service_request.cpp`
   - 请求参数归一化、过滤参数解析与校验。
10. `apps/time_tracer/src/infrastructure/persistence/sqlite_data_query_service_dispatch.cpp`
   - 轻量 action 路由（调用 orchestrators）。
11. `apps/time_tracer/src/infrastructure/persistence/sqlite_data_query_service_report_mapping.cpp`
   - `report-chart` 统计字段组装（平均值、总时长、活跃天数、范围天数）。

## Transport 层
1. `modules/tracer_transport/include/tracer/transport/runtime_requests.hpp`
2. `modules/tracer_transport/src/runtime_codec_query.cpp`
   - runtime 请求编解码（含 `output_mode` 等字段透传）。

## Windows CLI 适配层
1. `apps/tracer_windows_cli/src/api/cli/impl/commands/query/data_query_parser.cpp`
   - CLI 参数解析（含 `--data-output`、`--root` 等）。
2. `apps/tracer_windows_cli/src/api/cli/impl/commands/query/query_command.cpp`
   - query 命令执行与输出。
3. `apps/tracer_windows_cli/src/bootstrap/cli_runtime_factory_proxy.cpp`
   - runtime proxy 请求映射与响应解码。

## Android 适配层
1. `apps/tracer_android/runtime/src/main/java/com/example/tracer/runtime/controller/RuntimeQueryDelegate.kt`
   - Android query 参数透传（含 root/output_mode）。
2. `apps/tracer_android/runtime/src/main/java/com/example/tracer/runtime/NativeRuntimeQueryOps.kt`
   - Core 返回 payload 解析与模型映射。
3. `apps/tracer_android/runtime/src/main/java/com/example/tracer/runtime/NativeRuntimeController.kt`
   - runtime 调用总入口。

## 测试参考
1. `apps/time_tracer/src/application/tests/modules/data_query_tests.cpp`
2. `apps/time_tracer/src/infrastructure/tests/android_runtime/android_runtime_smoke_tests.cpp`
3. `apps/tracer_android/feature-report/src/test/java/com/example/tracer/QueryReportViewModelChartTest.kt`
4. `apps/time_tracer/src/infrastructure/tests/data_query_refactor_tests.cpp`

# DataQuery Architecture

本目录聚焦 `DataQuery` 架构文档，说明“统计计算 / 时间范围编排 / 格式渲染 / 入口适配”的职责边界与落位方式。

## Migration Note
1. Capability-first query routing now starts at
   `docs/time_tracer/core/capabilities/query/README.md`.
2. This directory still holds detailed data-query architecture docs.

## 文档
1. `docs/time_tracer/core/architecture/data_query/data_query_responsibility_boundaries_v1.md`
   - 职责边界说明与代码落点索引（Core/CLI/Android）。
2. `docs/time_tracer/core/architecture/data_query/data_query_refactor_completion_v1.md`
   - 重构收口清单与稳态维护规则。

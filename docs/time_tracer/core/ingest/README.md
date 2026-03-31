# Core Ingest

本目录放置导入链路（ingest）相关文档，聚焦数据结构与转换算法。

## Migration Note
1. Capability-first ingest routing now starts at
   `docs/time_tracer/core/capabilities/ingest/README.md`.
2. This legacy folder still retains the long-form ingest topic docs during the
   transition.

## 文档
1. `docs/time_tracer/core/ingest/txt_to_db_business_logic.md`
   - 解释原始 TXT 合同、wake/sleep 语义、跨日跨月补链，以及 `validate -> parse -> insert db`
2. `docs/time_tracer/core/ingest/ingest_data_structures.md`
3. `docs/time_tracer/core/ingest/ingest_conversion_algorithms.md`
4. `docs/time_tracer/core/ingest/day_bucket_and_wake_anchor_semantics.md`
   - 解释 day bucket、wake anchor、自动补睡眠和 `>24h` 合法口径
5. `docs/time_tracer/core/ingest/record_input_and_day_completeness_semantics.md`
   - 解释首条记录、day completeness、跨日上下文和 warning/error 边界


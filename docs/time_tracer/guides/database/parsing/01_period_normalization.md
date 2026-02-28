# 01 周期归一化算法

本文描述第 1 步：把不同查询周期参数统一成可执行过滤条件的基础形态。

## 1. 目标

把 `day/week/month/year/recent/range` 统一归一化成：

1. `from_date` / `to_date`
2. 基础过滤字段（`year/month/date/project/remark/status/...`）

## 2. 输入与输出

输入：

1. `DataQueryRequest`

输出：

1. `QueryFilters`

## 3. 核心步骤

1. `BuildCliFilters(...)` 先提取基础过滤字段。
2. `ApplyTreePeriod(...)` 按周期类型修正日期边界：
   1. `day`：同一天起止。
   2. `week/month/year`：按自然周期展开。
   3. `recent`：以数据库最新日期为锚点回看 N 天。
   4. `range`：直接使用输入起止区间。
3. 输出统一的 `QueryFilters` 供下一步拼 SQL。

## 4. 伪代码

```text
filters = BuildCliFilters(request)
if request.tree_period exists:
  ApplyTreePeriod(request, db_conn, filters)
return filters
```

## 5. 源码定位

- `apps/tracer_core/src/infrastructure/persistence/sqlite_data_query_service.cpp`：`BuildCliFilters(...)`
- `apps/tracer_core/src/infrastructure/persistence/sqlite_data_query_service.cpp`：`ApplyTreePeriod(...)`
- `apps/tracer_core/src/infrastructure/persistence/sqlite_data_query_service.cpp`：`SqliteDataQueryService::RunDataQuery(...)`

## 6. 下一步

- `docs/time_tracer/guides/database/parsing/02_filter_sql_build.md`

# 03 SQL 执行与取行解码算法

本文描述第 3 步：执行 SQL，并把结果行解码为中间记录。

## 1. 目标

完成查询执行生命周期：

1. `sqlite3_prepare_v2`
2. `bind` 参数
3. `sqlite3_step` 逐行读取
4. `finalize` 释放语句

## 2. 关键查询入口

1. `QueryDatesByFilters(...)`
2. `QueryDayDurations(...)`
3. `QueryActivitySuggestions(...)`
4. `QueryProjectTree(...)`

## 3. 快照列约束

涉及项目路径过滤/聚合时，先执行：

1. `EnsureProjectPathSnapshotColumnOrThrow(...)`

若缺少 `time_records.project_path_snapshot`，直接报错并要求升级或重建数据库。

## 4. 核心步骤

1. 按查询类型拼接 SQL 主体与 `WHERE/GROUP BY/ORDER BY`。
2. 使用统一参数绑定规则写入语句参数。
3. `step` 循环中读取列值，转换为中间行结构。
4. 返回给语义投影阶段做统一语义整理。

## 5. 伪代码

```text
stmt = prepare(sql)
bind(stmt, params)
while step(stmt) == ROW:
  row = decode_columns(stmt)
  rows.push(row)
finalize(stmt)
return rows
```

## 6. 源码定位

- `apps/time_tracer/src/infrastructure/query/data/data_query_repository.cpp`：`QueryDatesByFilters(...)`
- `apps/time_tracer/src/infrastructure/query/data/data_query_repository.cpp`：`QueryDayDurations(...)`
- `apps/time_tracer/src/infrastructure/query/data/data_query_repository.cpp`：`QueryActivitySuggestions(...)`
- `apps/time_tracer/src/infrastructure/query/data/data_query_repository.cpp`：`QueryProjectTree(...)`
- `apps/time_tracer/src/infrastructure/query/data/data_query_repository.cpp`：`EnsureProjectPathSnapshotColumnOrThrow(...)`
- `apps/time_tracer/src/infrastructure/query/data/data_query_repository_sql.cpp`：`BindAll(...)`

## 7. 下一步

- `docs/time_tracer/guides/database/parsing/04_semantic_projection.md`

# 02 过滤器与 SQL 片段构建算法

本文描述第 2 步：把 `QueryFilters` 转成安全、可绑定参数的 SQL 条件片段。

## 1. 目标

把过滤条件转换为：

1. `WHERE` 子句片段列表
2. 对应参数列表（`?` 绑定）

## 2. 输入与输出

输入：

1. `QueryFilters`

输出：

1. `std::vector<std::string>`（SQL clauses）
2. `std::vector<SqlParam>`（绑定参数）

## 3. 核心步骤

1. `BuildWhereClauses(...)` 为各过滤项生成统一 SQL 片段。
2. 项目路径过滤固定使用快照列：
   1. `tr.project_path_snapshot LIKE ? ESCAPE '\'`
3. `EscapeLikeLiteral(...)` 对 `%/_/\` 做转义。
4. 在需要 `d + tr` 连接时，使用 `BuildProjectDateJoinSql()` 生成查询骨架。

## 4. 伪代码

```text
clauses = []
params = []
append year/month/date/status/... clauses
if filters.project:
  clauses += "tr.project_path_snapshot LIKE ? ESCAPE '\\'"
  params += "%" + EscapeLikeLiteral(project) + "%"
return clauses, params
```

## 5. 源码定位

- `apps/time_tracer/src/infrastructure/query/data/data_query_repository_sql.cpp`：`BuildWhereClauses(...)`
- `apps/time_tracer/src/infrastructure/query/data/data_query_repository_sql.cpp`：`EscapeLikeLiteral(...)`
- `apps/time_tracer/src/infrastructure/query/data/data_query_repository_sql.cpp`：`BuildProjectDateJoinSql()`
- `apps/time_tracer/src/infrastructure/query/data/data_query_repository_sql.hpp`

## 6. 下一步

- `docs/time_tracer/guides/database/parsing/03_sql_execution_row_decode.md`

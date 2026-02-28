# SQLite 数据库解析算法总览

本文是数据库解析算法的入口页，只负责两件事：

1. 给出解析顺序（1 -> 4）。
2. 给出每一步对应文档和源码入口。

## 1. 解析顺序（按序阅读）

1. 周期归一化：`docs/time_tracer/guides/database/parsing/01_period_normalization.md`
2. 过滤器与 SQL 片段构建：`docs/time_tracer/guides/database/parsing/02_filter_sql_build.md`
3. SQL 执行与取行解码：`docs/time_tracer/guides/database/parsing/03_sql_execution_row_decode.md`
4. 语义投影（把行数据变成稳定业务结构）：`docs/time_tracer/guides/database/parsing/04_semantic_projection.md`

## 2. 总调用链

以 `query data tree` 为例，主链路如下：

1. `SqliteDataQueryService::RunDataQuery(...)` 进入查询服务。
2. 归一化周期参数并构建过滤器（步骤 1 + 2）。
3. 执行 SQL、逐行读取（步骤 3）。
4. 做语义投影后交给树生成算法（步骤 4 -> generation）。

## 3. 主源码入口

- `apps/tracer_core/src/infrastructure/persistence/sqlite_data_query_service.cpp`
- `apps/tracer_core/src/infrastructure/query/data/data_query_repository_sql.cpp`
- `apps/tracer_core/src/infrastructure/query/data/data_query_repository.cpp`

## 4. 与树生成算法的边界

本目录只负责“从 SQLite 读并解析语义化记录”。  
树构建和树渲染不在这里，见：

- `docs/time_tracer/guides/database/generation/tree_algorithms.md`

## 5. 维护约定（防回退）

1. 解析算法入口统一引用本页：`docs/time_tracer/guides/database/parsing/README.md`。
2. 算法正文只维护在 `01~04` 文档中。
3. `core_algorithms.md` 与 `implementation.md` 仅作为兼容入口，不再写算法正文。
4. 新增解析相关文档时，需先在本页添加入口，再在其它文档引用。

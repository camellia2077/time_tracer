# 04 语义投影算法

本文描述第 4 步：把 SQL 行结果投影为稳定业务语义结构，供后续模块消费。

## 1. 目标

把扁平行数据规范化成稳定结构，例如：

1. `(path, duration)` 记录
2. `DayDurationRow`
3. `ActivitySuggestionRow`

## 2. 核心语义规则

1. 路径协议固定 `_` 为数据库内部分隔符。
2. `project_path_snapshot` 为 `NULL` 或空串时跳过。
3. 输出结构保持稳定字段语义，不混入展示层逻辑。

## 3. 与树生成的边界

语义投影负责：

1. 路径与时长等业务字段的标准化输出。

语义投影不负责：

1. `ProjectTree` 的层级累计构建。
2. 树文本渲染（`├──`、`└──`、深度限制等）。

树生成算法见：

- `docs/time_tracer/guides/database/generation/tree_algorithms.md`

## 4. 伪代码

```text
for row in sql_rows:
  if path is null or empty:
    continue
  records.push((path, duration))
return records
```

## 5. 源码定位

- `apps/tracer_core/src/infrastructure/query/data/data_query_repository.cpp`：`QueryProjectTree(...)` 取行与空值过滤
- `apps/tracer_core/src/infrastructure/reports/data/utils/project_tree_builder.cpp`：`BuildProjectTreeFromRecords(...)`（投影结果的下游消费者）

## 6. 回到总览

- `docs/time_tracer/guides/database/parsing/README.md`

# Tree 生成算法（聚合与渲染）

本文专门说明：在数据库解析得到标准记录后，如何生成项目树并输出树形结果。

权威代码入口：
- `apps/time_tracer/src/infrastructure/reports/data/utils/project_tree_builder.cpp`
- `apps/time_tracer/src/infrastructure/query/data/renderers/data_query_renderer.cpp`
- `apps/time_tracer/src/infrastructure/query/data/renderers/text_renderer.cpp`
- `apps/time_tracer/src/infrastructure/reports/data/cache/project_name_cache.hpp`
- `apps/time_tracer/src/infrastructure/reports/services/batch_export_helpers.hpp`

## 1. 输入与输出

输入（常见两种）：

1. 路径记录：`std::vector<std::pair<std::string, long long>>`
   1. 形如 `(project_path, duration_seconds)`。
2. ID 记录：`std::vector<std::pair<long long, long long>>`
   1. 形如 `(project_id, duration_seconds)`。

输出：

1. `reporting::ProjectTree`（内存树）
2. 树形文本（CLI）

## 2. 路径记录构树算法

入口：
- `BuildProjectTreeFromRecords(...)`

核心逻辑：

1. `project_path` 按 `_` 拆分层级。
2. 从根到叶逐层插入/查找节点。
3. 每层节点都累加当前记录的 `duration`。

伪代码：

```text
for (path, duration) in records:
  parts = split(path, '_')
  if parts.empty(): continue

  node = tree[parts[0]]
  node.duration += duration

  for part in parts[1:]:
    node = node.children[part]
    node.duration += duration
```

结果语义：

1. 父节点时长包含子节点总和（包含式聚合）。
2. 同一路径重复出现自动累加。

源码定位：
- `apps/time_tracer/src/infrastructure/reports/data/utils/project_tree_builder.cpp`：`BuildProjectTreeFromRecords(...)`
- `apps/time_tracer/src/infrastructure/reports/data/utils/project_tree_builder.hpp`

## 3. ID 记录构树算法

入口：
- `BuildProjectTreeFromIds(...)`

核心逻辑：

1. 通过 `IProjectInfoProvider::GetPathParts(project_id)` 取得路径片段。
2. 后续逐层累计逻辑与路径记录构树一致。

常见 provider：
- `ProjectNameCache`
  1. 先加载 `projects(id, name, parent_id)` 到缓存。
  2. 通过 `parent_id` 向上回溯并反转，恢复路径片段。

源码定位：
- `apps/time_tracer/src/infrastructure/reports/data/utils/project_tree_builder.cpp`：`BuildProjectTreeFromIds(...)`
- `apps/time_tracer/src/infrastructure/reports/data/cache/project_name_cache.hpp`：`ProjectNameCache`
- `apps/time_tracer/src/domain/reports/interfaces/i_project_info_provider.hpp`：`IProjectInfoProvider`

## 4. 树渲染算法（CLI）

入口：
- `RenderProjectTreeText(...)`
- `AppendTreeChildren(...)`

核心逻辑：

1. 根节点排序
   1. 按名称字典序输出，保证日志稳定可对比。
2. 子节点排序
   1. 每层按名称字典序排序后递归输出。
3. 结构字符输出
   1. 使用 `├──`、`└──`、`│` 组织层级。
4. 深度限制
   1. `max_depth` 控制递归层数。
   2. `-1` 表示无限制。

源码定位：
- `apps/time_tracer/src/infrastructure/query/data/renderers/text_renderer.cpp`：`RenderProjectTreeText(...)`
- `apps/time_tracer/src/infrastructure/query/data/renderers/text_renderer.cpp`：`AppendTreeChildren(...)`

## 5. 复杂度与实现权衡

1. 构树复杂度
   1. 近似 `O(总路径段数)`。
2. 容器选择
   1. 内部 `unordered_map` 优先构建性能。
3. 输出稳定性
   1. 渲染前排序牺牲少量性能，换取稳定输出（便于测试和 diff）。

## 6. 与数据库解析算法的职责边界

Tree 生成算法负责：

1. 从标准化记录构建层级结构。
2. 把层级结构渲染成可读文本。

Tree 生成算法不负责：

1. 周期参数归一化。
2. SQL 条件拼接与执行。
3. 数据库解析层的列校验与错误处理策略。

数据库解析算法请看：
- `docs/time_tracer/guides/database/parsing/README.md`

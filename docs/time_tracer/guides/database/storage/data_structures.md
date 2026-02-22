# 数据库存储数据结构模型

本文描述的是“数据库里实际保存的数据形态（Data Model）”，用于帮助外部程序理解和消费数据。  
表结构细节（DDL、索引、约束）请看 `database_schema.md`。

## 1. 模型总览

当前存储模型由三类核心实体组成：

1. `Day`：按天的聚合信息与标记。
2. `ProjectNode`：项目层级节点（树形邻接表）。
3. `TimeRecord`：活动明细记录（时间片段）。

可以理解为：

```text
Day(1) ---- (N) TimeRecord(N) ---- (1) ProjectNode
ProjectNode(1) ---- (N) ProjectNode.children
```

## 2. 核心实体结构

### 2.1 Day（天维度聚合）

语义：一行代表一天的“状态 + 统计”。

| 字段组 | 代表字段 |
| --- | --- |
| 主键/时间 | `date`, `year`, `month` |
| 日标记 | `status`, `sleep`, `exercise` |
| 文本信息 | `remark`, `getup_time` |
| 聚合统计（秒） | `study_time`, `total_exercise_time`, `cardio_time`, `anaerobic_time`, `recreation_time`, `sleep_total_time` 等 |

### 2.2 ProjectNode（项目层级）

语义：`projects` 表中的一行代表树中的一个节点。

| 字段 | 语义 |
| --- | --- |
| `id` | 节点唯一标识 |
| `name` | 当前层名称 |
| `parent_id` | 父节点 ID（根节点可空） |
| `full_path` | 从根到当前节点的完整路径快照 |
| `depth` | 节点深度（根=0） |

### 2.3 TimeRecord（活动明细）

语义：一行代表一个活动时间区间。

| 字段组 | 代表字段 |
| --- | --- |
| 主键/定位 | `logical_id`, `date` |
| 时间区间 | `start_timestamp`, `end_timestamp`, `start`, `end`, `duration` |
| 项目关联 | `project_id`, `project_path_snapshot` |
| 文本信息 | `activity_remark` |

## 3. 存储协议（关键约定）

1. 时间单位
   1. 所有统计与时长统一为“秒”。
2. 日期/时间格式
   1. `date`：`YYYY-MM-DD`
   2. `start/end/getup_time`：`HH:MM`
3. 路径协议
   1. 数据库内部层级分隔符固定为 `_`。
   2. 示例：`study_math_linear-algebra`
4. 路径快照语义
   1. `project_path_snapshot` 保存“写入当时”的完整路径。
   2. 查询建议优先使用快照字段，避免每次递归还原路径。
5. 逻辑 ID 语义
   1. `logical_id` 为单日内有序记录标识（由转换流程生成）。

## 4. 典型读取视图（建议对外暴露）

外部程序常用三种视图：

1. `DaySummary`
   1. 直接读取 `days`，用于日历/趋势统计。
2. `RecordWithPath`
   1. 读取 `time_records`，优先使用 `project_path_snapshot` 作为路径。
3. `RecordWithProjectNode`
   1. 作为项目结构校验/诊断视图，不作为主查询路径。

## 5. 最小示例（概念）

```json
{
  "day": {
    "date": "2026-01-03",
    "status": 1,
    "exercise": 1,
    "study_time": 3600
  },
  "record": {
    "logical_id": 20260103000001,
    "date": "2026-01-03",
    "start": "08:00",
    "end": "09:00",
    "duration": 3600,
    "project_path_snapshot": "study_english_words"
  },
  "project_node": {
    "id": 42,
    "name": "words",
    "parent_id": 41,
    "full_path": "study_english_words",
    "depth": 2
  }
}
```

## 6. 与其他文档的关系

1. DDL/约束/索引：`docs/time_tracer/guides/database/database_schema.md`
2. 数据库解析算法：`docs/time_tracer/guides/database/parsing/README.md`
3. 树聚合与渲染算法：`docs/time_tracer/guides/database/generation/tree_algorithms.md`

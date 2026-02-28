# 数据库表设计说明（SQLite）

本文用于说明当前 `time_tracer` 的 SQLite 存储结构，方便外部程序（Python、Go、Rust、BI 工具等）直接查询。

权威来源（以代码为准）：
- `apps/tracer_core/src/infrastructure/persistence/importer/sqlite/connection.cpp`
- `apps/tracer_core/src/infrastructure/schema/day_schema.hpp`
- `apps/tracer_core/src/infrastructure/schema/sqlite_schema.hpp`

相关文档：
- `docs/time_tracer/core/ingest/ingest_data_structures.md`（文本内容转换为 struct 的流程与字段说明）
- `docs/time_tracer/guides/database/storage/data_structures.md`（数据库中存储数据的结构模型）
- `docs/time_tracer/guides/database/parsing/README.md`（SQLite 数据库解析总览与执行顺序）
- `docs/time_tracer/guides/database/parsing/01_period_normalization.md`（第 1 步：周期归一化）
- `docs/time_tracer/guides/database/parsing/02_filter_sql_build.md`（第 2 步：过滤器与 SQL 片段构建）
- `docs/time_tracer/guides/database/parsing/03_sql_execution_row_decode.md`（第 3 步：SQL 执行与取行解码）
- `docs/time_tracer/guides/database/parsing/04_semantic_projection.md`（第 4 步：语义投影）
- `docs/time_tracer/guides/database/generation/tree_algorithms.md`（树结构聚合与渲染算法）

## 1. 数据库位置

数据库为单文件 SQLite，路径由 `config/config.toml` 的 `db_path` 指定。

## 2. 表总览

- `days`：按天聚合的统计和标记。
- `projects`：活动项目树（层级节点）。
- `time_records`：原始时间记录（每条活动）。

## 3. 表结构

### 3.1 `days`

主键：`date` (`TEXT`, `YYYY-MM-DD`)

| 列名 | 类型 | 约束 | 说明 |
| --- | --- | --- | --- |
| `date` | TEXT | PRIMARY KEY | 日期 |
| `year` | INTEGER |  | 年 |
| `month` | INTEGER |  | 月（1~12） |
| `status` | INTEGER |  | 状态标记（0/1） |
| `sleep` | INTEGER |  | 睡眠标记（0/1） |
| `remark` | TEXT |  | 当天备注 |
| `getup_time` | TEXT |  | 起床时间（`HH:MM`） |
| `exercise` | INTEGER |  | 运动标记（0/1） |
| `total_exercise_time` | INTEGER |  | 运动总时长（秒） |
| `cardio_time` | INTEGER |  | 有氧时长（秒） |
| `anaerobic_time` | INTEGER |  | 无氧时长（秒） |
| `gaming_time` | INTEGER |  | 游戏时长（秒） |
| `grooming_time` | INTEGER |  | 洗漱时长（秒） |
| `toilet_time` | INTEGER |  | 如厕时长（秒） |
| `study_time` | INTEGER |  | 学习时长（秒） |
| `sleep_night_time` | INTEGER |  | 夜间睡眠（秒） |
| `sleep_day_time` | INTEGER |  | 白天睡眠（秒） |
| `sleep_total_time` | INTEGER |  | 睡眠总时长（秒） |
| `recreation_time` | INTEGER |  | 娱乐总时长（秒） |
| `recreation_zhihu_time` | INTEGER |  | 知乎娱乐（秒） |
| `recreation_bilibili_time` | INTEGER |  | B 站娱乐（秒） |
| `recreation_douyin_time` | INTEGER |  | 抖音娱乐（秒） |

### 3.2 `projects`

主键：`id` (`INTEGER`, AUTOINCREMENT)

| 列名 | 类型 | 约束 | 说明 |
| --- | --- | --- | --- |
| `id` | INTEGER | PRIMARY KEY AUTOINCREMENT | 项目节点 ID |
| `name` | TEXT | NOT NULL | 当前层级名称 |
| `parent_id` | INTEGER | FK -> `projects.id` | 父节点 ID，根节点可空 |
| `full_path` | TEXT | NOT NULL DEFAULT `''` | 完整路径快照（`_` 分隔） |
| `depth` | INTEGER | NOT NULL DEFAULT `0` | 层级深度（根为 0） |

说明：
- 项目层级分隔符在数据库内部固定为 `_`（例如 `study_math_linear-algebra`）。
- 展示层可以自行把 `_` 替换为任意连接符（如 `/`、` > `、emoji 等）。

### 3.3 `time_records`

主键：`logical_id` (`INTEGER`)

| 列名 | 类型 | 约束 | 说明 |
| --- | --- | --- | --- |
| `logical_id` | INTEGER | PRIMARY KEY | 逻辑记录 ID |
| `start_timestamp` | INTEGER | NOT NULL, CHECK `>= 0` | 开始时间戳（秒） |
| `end_timestamp` | INTEGER | NOT NULL, CHECK `>= start_timestamp` | 结束时间戳（秒） |
| `date` | TEXT | NOT NULL, FK -> `days.date` | 日期 |
| `start` | TEXT | NOT NULL | 开始时间（`HH:MM`） |
| `end` | TEXT | NOT NULL | 结束时间（`HH:MM`） |
| `project_id` | INTEGER | NOT NULL, FK -> `projects.id` | 项目节点 ID |
| `duration` | INTEGER | NOT NULL, CHECK `>= 0` | 时长（秒） |
| `project_path_snapshot` | TEXT | NOT NULL DEFAULT `''` | 写入时的完整路径快照（`_` 分隔） |
| `activity_remark` | TEXT |  | 活动备注（可空） |

## 4. 索引设计（当前实现）

| 索引名 | 表 | 列 | 类型 | 作用 |
| --- | --- | --- | --- | --- |
| `idx_year_month` | `days` | (`year`, `month`) | 普通索引 | 月/年统计查询 |
| `idx_projects_full_path_unique` | `projects` | (`full_path`) | 部分唯一索引（`WHERE full_path <> ''`） | 路径去重 |
| `idx_projects_parent_name_unique` | `projects` | (`parent_id`, `name`) | 唯一索引 | 同父节点下名称去重 |
| `idx_time_records_date_project` | `time_records` | (`date`, `project_id`) | 普通索引 | 按日期+项目查询 |
| `idx_time_records_date_path_snapshot` | `time_records` | (`date`, `project_path_snapshot`) | 普通索引 | 按日期+路径快照查询 |

## 5. 外键关系

- `time_records.date` -> `days.date`
- `time_records.project_id` -> `projects.id`
- `projects.parent_id` -> `projects.id`

SQLite 连接初始化时会执行 `PRAGMA foreign_keys = ON;`。

## 6. 外部程序常用查询示例

```sql
-- 1) 某月日统计
SELECT *
FROM days
WHERE year = 2026 AND month = 1
ORDER BY date;
```

```sql
-- 2) 某日期区间总时长（秒）
SELECT date, SUM(duration) AS total_duration
FROM time_records
WHERE date BETWEEN '2026-01-01' AND '2026-01-31'
GROUP BY date
ORDER BY date;
```

```sql
-- 3) 直接用快照路径过滤（推荐）
-- 注意：_ 在 LIKE 里是通配符，需 ESCAPE。
SELECT date, SUM(duration) AS study_seconds
FROM time_records
WHERE date BETWEEN '2026-01-01' AND '2026-01-31'
  AND (
    project_path_snapshot = 'study'
    OR project_path_snapshot LIKE 'study\_%' ESCAPE '\'
  )
GROUP BY date
ORDER BY date;
```

```sql
-- 4) 结构诊断场景：从 projects 反推完整路径（非主查询路径）
WITH RECURSIVE project_paths(id, path) AS (
  SELECT id, name
  FROM projects
  WHERE parent_id IS NULL
  UNION ALL
  SELECT p.id, pp.path || '_' || p.name
  FROM projects p
  JOIN project_paths pp ON p.parent_id = pp.id
)
SELECT tr.logical_id, tr.date, tr.start, tr.end, tr.duration,
       pp.path AS project_path, tr.activity_remark
FROM time_records tr
JOIN project_paths pp ON pp.id = tr.project_id
ORDER BY tr.logical_id;
```

## 7. 兼容性与接入建议

- 运行时查询统一使用 `time_records.project_path_snapshot` 做路径过滤与聚合。
- 数据库需包含 `project_path_snapshot` 列；缺失时应先升级或重建数据库。
- 建议保证入库流程始终写入非空快照路径，避免查询阶段过滤掉空路径记录。
- 接入方写查询时建议先 `EXPLAIN QUERY PLAN`，确认命中索引。

## 8. 字段常量参考

- `apps/tracer_core/src/infrastructure/schema/day_schema.hpp`
- `apps/tracer_core/src/infrastructure/schema/sqlite_schema.hpp`

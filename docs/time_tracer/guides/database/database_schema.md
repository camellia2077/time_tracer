# 数据库说明（SQLite）

本项目定位为**文本分析工具**，不包含任何图片生成逻辑。折线图/柱状图等可由外部脚本（如 Python）读取数据库后生成。本页记录数据库结构与约束，便于其他工具直接读取。

## 1. 数据库位置

数据库为单文件 SQLite。路径由 `config/config.toml` 中的 `db_path` 指定。

## 2. 表结构

### 2.1 `days`

- **主键**：`date` (TEXT, `YYYY-MM-DD`)
- **用途**：存放“按天聚合”的统计与标记

| 列名 | 类型 | 含义 | 备注 |
| --- | --- | --- | --- |
| `date` | TEXT | 日期 | 主键，格式 `YYYY-MM-DD` |
| `year` | INTEGER | 年 | 例：2026 |
| `month` | INTEGER | 月 | 1~12 |
| `status` | INTEGER | 当天状态标记 | 0/1 |
| `sleep` | INTEGER | 是否睡眠标记 | 0/1 |
| `remark` | TEXT | 当天备注 | 可为空 |
| `getup_time` | TEXT | 起床时间 | `HH:MM`，可能为空或 `00:00` |
| `exercise` | INTEGER | 是否运动标记 | 0/1 |
| `total_exercise_time` | INTEGER | 运动总时长 | 秒 |
| `cardio_time` | INTEGER | 有氧时长 | 秒 |
| `anaerobic_time` | INTEGER | 无氧时长 | 秒 |
| `gaming_time` | INTEGER | 游戏时长 | 秒 |
| `grooming_time` | INTEGER | 洗漱时长 | 秒 |
| `toilet_time` | INTEGER | 如厕时长 | 秒 |
| `study_time` | INTEGER | 学习时长 | 秒 |
| `sleep_night_time` | INTEGER | 夜间睡眠时长 | 秒 |
| `sleep_day_time` | INTEGER | 白天睡眠时长 | 秒 |
| `sleep_total_time` | INTEGER | 睡眠总时长 | 秒 |
| `recreation_time` | INTEGER | 娱乐总时长 | 秒 |
| `recreation_zhihu_time` | INTEGER | 知乎娱乐时长 | 秒 |
| `recreation_bilibili_time` | INTEGER | B 站娱乐时长 | 秒 |
| `recreation_douyin_time` | INTEGER | 抖音娱乐时长 | 秒 |

#### 2.1.1 字段语义与生成规则（更细说明）

- **`date/year/month`**：从日期字符串解析得到，`year/month` 直接由 `date` 拆分。
- **`status`**：当天是否存在以 `study` 开头的活动（存在=1，不存在=0）。
- **`sleep`**：当天是否存在睡眠活动（存在=1，不存在=0）。睡眠活动可能由解析逻辑自动补齐（例如依据前一天最后事件与起床时间生成 `sleep_night`）。
- **`exercise`**：当天是否存在以 `exercise` 开头的活动（存在=1，不存在=0）。
- **`remark`**：当天所有备注合并为一个字符串，按输入顺序用换行拼接。
- **`getup_time`**：起床时间 `HH:MM`。若为“续写日”则写入 `Null` 并最终存入数据库为 `NULL`；若缺省则写入 `00:00`。
- **时间统计字段（统一单位：秒）**：
  - `sleep_night_time`：项目路径 **等于** `sleep_night` 的时长累计。
  - `sleep_day_time`：项目路径 **等于** `sleep_day` 的时长累计。
  - `sleep_total_time`：`sleep_night_time + sleep_day_time`。
  - `study_time`：项目路径 **以** `study` 开头的时长累计。
  - `total_exercise_time`：项目路径 **以** `exercise` 开头的时长累计。
  - `cardio_time`：项目路径 **以** `exercise_cardio` 开头的时长累计。
  - `anaerobic_time`：项目路径 **以** `exercise_anaerobic` 开头的时长累计。
  - `grooming_time`：项目路径 **以** `routine_grooming` 开头的时长累计。
  - `toilet_time`：项目路径 **以** `routine_toilet` 开头的时长累计。
  - `gaming_time`：项目路径 **以** `recreation_game` 开头的时长累计。
  - `recreation_time`：项目路径 **以** `recreation` 开头的时长累计。
  - `recreation_zhihu_time`：项目路径 **以** `recreation_zhihu` 开头的时长累计。
  - `recreation_bilibili_time`：项目路径 **以** `recreation_bilibili` 开头的时长累计。
  - `recreation_douyin_time`：项目路径 **以** `recreation_douyin` 开头的时长累计。

### 2.2 `time_records`

- **主键**：`logical_id` (INTEGER)
- **用途**：存放“原始活动记录”

| 列名 | 类型 | 含义 | 备注 |
| --- | --- | --- | --- |
| `logical_id` | INTEGER | 活动逻辑 ID | 由日期 + 日内序号组合生成 |
| `start_timestamp` | INTEGER | 开始时间戳 | 秒级时间戳 |
| `end_timestamp` | INTEGER | 结束时间戳 | 秒级时间戳 |
| `date` | TEXT | 日期 | `YYYY-MM-DD`，外键到 `days.date` |
| `start` | TEXT | 开始时间 | `HH:MM` |
| `end` | TEXT | 结束时间 | `HH:MM` |
| `project_id` | INTEGER | 项目 ID | 外键到 `projects.id` |
| `duration` | INTEGER | 时长 | 秒 |
| `activity_remark` | TEXT | 活动备注 | 可为空 |

#### 2.2.1 字段语义与生成规则（更细说明）

- **`logical_id`**：按天递增的逻辑 ID，格式为 `(YYYYMMDD * 1,000,000) + 序号`。  
  例：`20210101` 第 42 条记录 → `20210101000042`。
- **`start/end`**：活动的起止时间字符串 `HH:MM`。
- **`duration`**：由 `start/end` 计算得到，若结束时间早于开始时间，则视为跨天并加 24 小时。
- **`start_timestamp/end_timestamp`**：用 `date + time` 生成秒级时间戳，若为跨天结束则 `end_timestamp` 会额外加 24 小时。
- **`project_id`**：由 `project_path` 解析得到的项目 ID（见 `projects` 表）。
- **`activity_remark`**：单条活动备注，允许为空。

### 2.3 `projects`

- **主键**：`id` (INTEGER, AUTOINCREMENT)
- **用途**：存放活动所属项目树

| 列名 | 类型 | 含义 | 备注 |
| --- | --- | --- | --- |
| `id` | INTEGER | 项目 ID | 主键 |
| `name` | TEXT | 项目名 | 不为空 |
| `parent_id` | INTEGER | 父节点 ID | 可为空，外键到 `projects.id` |

#### 2.3.1 字段语义与生成规则（更细说明）

- **层级关系**：`parent_id` 为空代表根节点；非空代表子节点。
- **路径规则**：完整路径通过递归 CTE 生成，用 `_` 连接各级 `name`。
  - 示例：`study_math_linear-algebra` 表示三层结构。
- **注意**：数据库层面不强制 `name` 的唯一性或同级唯一性，建议读取端自行处理。

## 3. 索引

| 索引名 | 表 | 列 | 用途 |
| --- | --- | --- | --- |
| `idx_year_month` | `days` | (`year`, `month`) | 加速按月/按年查询 |

## 4. 关系与约束

- `time_records.date` → `days.date`
- `time_records.project_id` → `projects.id`
- `projects.parent_id` → `projects.id`

## 5. 常用访问示例

```sql
-- 查询某月所有天的统计
SELECT * FROM days WHERE year = 2026 AND month = 1;
```

```sql
-- 按天汇总活动时长
SELECT date, SUM(duration) AS total_duration
FROM time_records
GROUP BY date;
```

```sql
-- 递归展开项目路径（示例）
WITH RECURSIVE project_paths(id, path) AS (
  SELECT id, name FROM projects WHERE parent_id IS NULL
  UNION ALL
  SELECT p.id, pp.path || '_' || p.name
  FROM projects p JOIN project_paths pp ON p.parent_id = pp.id
)
SELECT * FROM project_paths;
```

## 6. JSON 对应关系

JSON 与数据库字段命名保持一致，权威字段列表可参考：
- `apps/time_tracer/src/infrastructure/schema/day_schema.hpp`
- `apps/time_tracer/src/infrastructure/schema/time_records_schema.hpp`

如需扩展字段，建议同步更新上述 schema 常量与相关导入/查询逻辑。

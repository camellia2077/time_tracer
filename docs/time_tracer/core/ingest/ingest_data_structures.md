# 数据转换后的 Struct 结构说明（Ingest Pipeline）

本文说明 TimeTracer 在“文本内容 -> 结构化数据 -> SQLite 入库”过程中，核心 struct 如何设计与流转。

权威代码入口：
- `apps/tracer_core/src/application/parser/text_parser.cpp`
- `apps/tracer_core/src/domain/model/daily_log.hpp`
- `apps/tracer_core/src/domain/model/time_data_models.hpp`
- `apps/tracer_core/src/application/parser/memory_parser.cpp`
- `apps/tracer_core/src/application/importer/model/import_models.hpp`
- `apps/tracer_core/src/infrastructure/persistence/importer/sqlite/writer.cpp`

相关算法文档：
- `docs/time_tracer/core/ingest/ingest_conversion_algorithms.md`（文本到 struct 的转换算法细节）

## 1. 总体流转

```text
txt 行文本
  -> TextParser
  -> DailyLog.rawEvents
  -> DayProcessor(ActivityMapper + DayStats)
  -> DailyLog.processedActivities + DailyLog.stats
  -> LogProcessingResult (map<YYYY-MM, vector<DailyLog>>)
  -> MemoryParser
  -> ParsedData { vector<DayData>, vector<TimeRecordInternal> }
  -> sqlite::Writer
  -> days / projects / time_records
```

## 2. 解析层（Raw）

### 2.1 `SourceSpan`

文件：`apps/tracer_core/src/domain/model/source_span.hpp`

作用：记录源文本定位信息，便于错误定位与诊断。

关键字段：
- `file_path`
- `line_start`, `line_end`
- `column_start`, `column_end`
- `raw_text`

### 2.2 `RawEvent`

文件：`apps/tracer_core/src/domain/model/daily_log.hpp`

作用：`TextParser` 逐行解析事件后得到的最原始事件结构。

关键字段：
- `endTimeStr`: 行首时间（`HHMM` 原始格式）
- `description`: 活动描述（尚未做 mapping）
- `remark`: 行内备注（分隔符 `//` / `#` / `;`）
- `source_span`: 对应输入行位置

## 3. 领域转换层（Normalized Domain）

### 3.1 `ActivityStats`

文件：`apps/tracer_core/src/domain/model/time_data_models.hpp`

作用：一天内统计聚合结果（单位秒）。

字段包括：
- 睡眠：`sleep_night_time`, `sleep_day_time`, `sleep_total_time`
- 运动：`total_exercise_time`, `cardio_time`, `anaerobic_time`
- 例行：`grooming_time`, `toilet_time`
- 娱乐：`gaming_time`, `recreation_time`, `recreation_zhihu_time`, `recreation_bilibili_time`, `recreation_douyin_time`
- 学习：`study_time`

### 3.2 `BaseActivityRecord`

文件：`apps/tracer_core/src/domain/model/time_data_models.hpp`

作用：统一活动记录结构（已完成时间与项目路径归一化）。

关键字段：
- 时间/编号：`logical_id`, `start_timestamp`, `end_timestamp`
- 文本时间：`start_time_str`, `end_time_str`（`HH:MM`）
- 项目路径：`project_path`（数据库内部统一 `_` 分隔）
- 时长：`duration_seconds`
- 备注：`remark`
- 诊断：`source_span`

### 3.3 `DailyLog`

文件：`apps/tracer_core/src/domain/model/daily_log.hpp`

作用：单日完整领域对象，是转换阶段的核心载体。

关键字段：
- 日期与标记：`date`, `hasStudyActivity`, `hasExerciseActivity`, `hasSleepActivity`
- 起床与备注：`getupTime`, `generalRemarks`
- 原始事件：`rawEvents`（`vector<RawEvent>`）
- 归一化事件：`processedActivities`（`vector<BaseActivityRecord>`）
- 日统计：`stats`（`ActivityStats`）
- 其他：`isContinuation`, `activityCount`, `source_span`

说明：
- `TextParser` 先填充 `rawEvents`。
- `DayProcessor` 再生成 `processedActivities` 和 `stats`。
- 跨天睡眠（如 `sleep_night`）在 `DayProcessor` / `LogLinker` 阶段补齐。

### 3.4 `LogProcessingResult`

文件：`apps/tracer_core/src/domain/logic/converter/log_processor.hpp`

作用：单个输入源转换结果。

字段：
- `success`
- `processed_data`: `std::map<std::string, std::vector<DailyLog>>`

当前 key 约定：
- 使用 `YYYY-MM`（由 `DailyLog.date.substr(0, 7)` 得到）。

## 4. 入库适配层（Import Models）

### 4.1 `DayData`

文件：`apps/tracer_core/src/application/importer/model/import_models.hpp`

作用：`days` 表的入库模型。

字段：
- 基础：`date`, `remark`, `getup_time`
- 日期拆分：`year`, `month`
- 标记：`status`, `sleep`, `exercise`
- 聚合：`stats`（`ActivityStats`）

当前实现注意：
- `getup_time` 为 `std::optional<std::string>`。
- `MemoryParser` 在续写日写入 `nullopt`；`sqlite::Writer` 写入 `NULL`。

### 4.2 `TimeRecordInternal`

文件：`apps/tracer_core/src/application/importer/model/import_models.hpp`

作用：`time_records` 表入库模型。

定义：
- `struct TimeRecordInternal`（显式字段，不继承 `BaseActivityRecord`）
- 额外字段：`date`

说明：
- `MemoryParser` 按字段复制（时间、路径、时长、备注等），再补 `date`。

### 4.3 `ParsedData`

文件：`apps/tracer_core/src/application/importer/model/import_models.hpp`

作用：一次导入的批量入库数据包。

字段：
- `days: std::vector<DayData>`
- `records: std::vector<TimeRecordInternal>`

## 5. 从 Struct 到 SQLite 的映射

### 5.1 `DayData -> days`

文件：`apps/tracer_core/src/infrastructure/persistence/importer/sqlite/writer.cpp`

映射关系要点：
- `DayData.date/year/month/status/sleep/exercise/remark/getup_time` -> `days` 同名列
- `DayData.stats.*` -> `days` 中对应统计列（全部为秒）
- `getup_time` 为 `nullopt` 时写入 `NULL`

### 5.2 `TimeRecordInternal -> projects + time_records`

文件：
- `apps/tracer_core/src/infrastructure/persistence/importer/sqlite/project_resolver.cpp`
- `apps/tracer_core/src/infrastructure/persistence/importer/sqlite/writer.cpp`

流程：
1. 先用 `project_path`（`_` 分隔）经 `ProjectResolver` 解析/创建 `projects` 树节点，得到 `project_id`。
2. 写入 `time_records`：
   - `logical_id`, `start_timestamp`, `end_timestamp`, `date`
   - `start_time_str -> start`, `end_time_str -> end`
   - `project_id`, `duration_seconds -> duration`
   - `project_path -> project_path_snapshot`
   - `remark -> activity_remark`

## 6. 一个最小示例

输入文本（简化）：

```text
y2021
0101
0641wake
0827英语单词
1138有氧训练 //remark
```

关键结构变化（概念示例）：

```text
RawEvent:
  { endTimeStr:"0827", description:"英语单词", remark:"" }
  { endTimeStr:"1138", description:"有氧训练", remark:"remark" }

BaseActivityRecord (经过 mapping):
  { start_time_str:"06:41", end_time_str:"08:27", project_path:"study_english_words", ... }
  { start_time_str:"08:27", end_time_str:"11:38", project_path:"exercise_cardio", remark:"remark", ... }

DayData:
  { date:"2021-01-01", status:1, exercise:1, sleep:0, stats:{...} }
```

## 7. 对外接入建议

- 外部程序若只读 SQLite，可重点理解：
  - `days`（日聚合）
  - `time_records`（明细）
  - `projects`（项目树）
- 若需要复现核心转换逻辑，需要同时理解：
  - `TextParser`（文本语法）
  - `DayProcessor`（活动映射与统计）
  - `MemoryParser`（领域对象到入库对象）
- 路径协议建议统一按 `_` 作为数据库内部分隔符；展示层再做连接符替换。

# 文本内容转换为 Struct 的核心算法（Ingest）

本文聚焦算法层面，说明 TimeTracer 如何把输入文本逐步转换为可入库的结构化 `struct` 数据。

权威代码入口（以实现为准）：
- `apps/tracer_core/src/application/parser/text_parser.cpp`
- `apps/tracer_core/src/application/service/converter_service.cpp`
- `apps/tracer_core/src/domain/logic/converter/convert/core/converter_core.cpp`
- `apps/tracer_core/src/domain/logic/converter/log_processor.cpp`
- `apps/tracer_core/src/application/pipeline/steps/pipeline_stages.cpp`
- `apps/tracer_core/src/application/parser/memory_parser.cpp`
- `apps/tracer_core/src/application/importer/model/import_models.hpp`

## 1. 输入与输出

输入：
- 年/月/日+事件行文本（例如 `y2021`、`0101`、`0827英语单词`）

输出（入库前）：
- `ParsedData`
  - `days: std::vector<DayData>`
  - `records: std::vector<TimeRecordInternal>`

## 2. 总体算法链路

```text
txt
 -> TextParser (语法解析)
 -> DailyLog.rawEvents
 -> DayProcessor (映射/补全/统计)
 -> DailyLog.processedActivities + stats
 -> LogProcessor (按 YYYY-MM 分桶)
 -> Pipeline merge + LogLinker (跨月补链)
 -> MemoryParser
 -> ParsedData(days, records)
```

## 3. TextParser：把行文本转成 RawEvent

核心规则：

1. 预处理
   1. 每行 `Trim` 后处理。
   2. 空行跳过。
2. 年标记
   1. `y` + 4 位数字视为年份头（如 `y2025`）。
3. 日标记
   1. 4 位数字视为 `MMDD`。
   2. 拼成 `YYYY-MM-DD` 并开启新 `DailyLog`。
4. 事件行
   1. 前 4 位必须是 `HHMM` 且时分合法（`00:00`~`23:59`）。
   2. 后半段解析描述和备注，备注分隔符支持 `//`、`#`、`;`。
   3. 写入 `RawEvent{endTimeStr, description, remark, source_span}`。
5. 起床与续写识别
   1. 描述命中 `wake_keywords` 时记录 `getupTime`。
   2. 如果首事件不是 wake 且无 `getupTime`，标记 `isContinuation=true`（表示延续上一天）。

源码定位：
- `apps/tracer_core/src/application/parser/text_parser.cpp`：`TextParser::Parse(...)`
- `apps/tracer_core/src/application/parser/text_parser.cpp`：`TextParser::ParseLine(...)`、`TextParser::ExtractRemark(...)`
- `apps/tracer_core/src/application/parser/text_parser.cpp`：`TextParser::IsYearMarker(...)`、`TextParser::IsNewDayMarker(...)`

## 4. ConverterService：滑动窗口日处理

`ConverterService::ExecuteConversion(...)` 使用“前一天 + 当前天”的滑动窗口：

1. 第一天：`processor.Process(empty_day, current_day)`。
2. 后续天：`processor.Process(previous_day, current_day)`，用当前天补全前一天逻辑。
3. 去重规则：跨年边界 `12-31 -> 01-01` 的重复日不重复输出。
4. flush：解析结束后，再处理并输出最后一天。

该设计保证跨天逻辑可在单遍扫描内完成，不需要全量回看历史。

源码定位：
- `apps/tracer_core/src/application/service/converter_service.cpp`：`ConverterService::ExecuteConversion(...)`

## 5. DayProcessor：RawEvent 转 BaseActivityRecord

`DayProcessor::Process(...)` 由三部分组成：

1. `ActivityMapper::MapActivities`
   1. 以 `getupTime` 作为首段起点。
   2. 遍历 `rawEvents`，每个事件的 `endTime` 构成当前段终点。
   3. `description` 先经过 `text_mapping` / `text_duration_mapping` 映射。
   4. 计算时长（分钟）后应用 `duration_mappings` 规则二次映射。
   5. 项目路径按 `_` 分层，应用 `top_parent_mapping` / `initial_top_parents` 后重组。
   6. 生成 `BaseActivityRecord`（含 `start/end/project_path/remark/source_span`）。
2. 同日跨天睡眠补全
   1. 若 `previous_day` 存在末事件且 `current_day` 有有效 `getupTime`，在当天头部插入 `sleep_night`。
3. 续写日修复
   1. `isContinuation=true` 时，用上一天末事件时间回填 `getupTime`。

源码定位：
- `apps/tracer_core/src/domain/logic/converter/convert/core/converter_core.cpp`：`DayProcessor::Process(...)`
- `apps/tracer_core/src/domain/logic/converter/convert/core/converter_core.cpp`：`ActivityMapper`（活动映射）

## 6. DayStats：为 struct 填充统计与主键字段

`DayStats::CalculateStats(...)` 对 `processedActivities` 逐条计算：

1. `logical_id`
   1. `YYYYMMDD * 1_000_000 + sequence`，同一天内顺序递增。
2. `duration_seconds`
   1. 由 `HH:MM` 计算；若终点小于起点视为跨午夜，自动 +24h。
3. `start_timestamp/end_timestamp`
   1. 基于 `date + HH:MM` 转时间戳；跨日结束时间自动补一天。
4. 日标记
   1. `project_path` 前缀命中 `study` / `exercise`，置 `hasStudyActivity` / `hasExerciseActivity`。
5. 聚合统计
   1. 通过 `kStatsRules` 按路径前缀累加到 `ActivityStats`（学习、运动、娱乐、洗漱等）。
   2. `sleep_total_time = sleep_night_time + sleep_day_time`。

源码定位：
- `apps/tracer_core/src/domain/logic/converter/convert/core/converter_core.cpp`：`DayStats::CalculateStats(...)`

## 7. LogProcessor + Pipeline：分桶与跨月补链

1. `LogProcessor::ProcessSourceContent(...)`
   1. 每个 `DailyLog` 以 `date.substr(0, 7)` 分桶到 `processed_data["YYYY-MM"]`。
2. `ConverterStep::Execute(...)`
   1. 多文件并行转换后，按 `YYYY-MM` 把结果 `append` 合并进全局上下文。
3. `LogicLinker::LinkLogs(...)`
   1. 处理“上月最后一天 -> 本月第一天”的睡眠衔接。
   2. 条件满足时在本月首日插入生成睡眠段并重算统计。

源码定位：
- `apps/tracer_core/src/domain/logic/converter/log_processor.hpp`：`LogProcessor::ProcessSourceContent(...)`
- `apps/tracer_core/src/application/pipeline/steps/pipeline_stages.cpp`：`ConverterStep::Execute(...)`
- `apps/tracer_core/src/domain/logic/converter/convert/core/converter_core.cpp`：`LogLinker::LinkLogs(...)`

## 8. MemoryParser：领域结构转入库结构

`MemoryParser::Parse(...)` 把 `std::map<std::string, std::vector<DailyLog>>` 转成 `ParsedData`：

1. 预估容量
   1. 先遍历一次统计 `expected_days/expected_records`，提前 `reserve`。
2. DayData 生成
   1. 从 `date` 解析 `year/month`，失败则告警并跳过该天。
   2. 标记映射：
      1. `status = hasStudyActivity ? 1 : 0`
      2. `sleep = hasSleepActivity ? 1 : 0`
      3. `exercise = hasExerciseActivity ? 1 : 0`
   3. `remark` 把 `generalRemarks` 用换行拼接。
   4. `getup_time` 规则：
      1. 续写日 -> `nullopt`
      2. 空值 -> `"00:00"`
      3. 否则 -> 原值
   5. `day.stats = input_day.stats`（整包复制统计）。
3. TimeRecordInternal 生成
   1. 对每条 `processedActivities` 复制核心字段：
      1. `logical_id/start_timestamp/end_timestamp`
      2. `start_time_str/end_time_str`
      3. `project_path/duration_seconds/remark`
      4. `date`

源码定位：
- `apps/tracer_core/src/application/parser/memory_parser.cpp`：`MemoryParser::Parse(...)`
- `apps/tracer_core/src/application/importer/model/import_models.hpp`：`ParsedData`、`DayData`、`TimeRecordInternal`

## 9. 关键特性总结

1. 单遍主流程：解析和转换靠滑动窗口完成，不需要先构建全局大对象。
2. 跨日/跨月睡眠自动补链：同月在 `DayProcessor`，跨月在 `LogLinker`。
3. 路径协议固定：数据库内部层级分隔符统一 `_`。
4. 容错策略明确：日期不可解析的日数据在 `MemoryParser` 被过滤并记录告警。

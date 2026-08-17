# Activity Aggregate Contract v1

## 目的

统一活动次数与活动时长的统计来源，避免 Trend、Breakdown、周期活动概览和
Android 对同一批活动事实重复实现累计逻辑。

## Core 所有权

Core 使用 `ActivityAggregate` 表示同一统计范围内的活动聚合：

- `total_duration_seconds`：活动总时长，单位为秒。
- `occurrence_count`：活动发生次数，即活动事实数量。
- `average_duration_per_occurrence_seconds`：
  `total_duration_seconds / occurrence_count`；发生次数为 `0` 时为 `0`。

所有累计都通过该聚合结构完成。活动时长和活动次数必须来自同一查询范围、同一
过滤条件，不能分别从不同数据集推导。

## 跨端约束

- Trend 的 `total_duration_seconds`、`total_occurrence_count` 和
  `average_duration_per_occurrence_seconds` 由 Core 一次聚合后输出。
- Breakdown 节点的 `duration_seconds`、`occurrence_count` 和每次平均耗时遵循相同
  的除法规则。
- 周期结构化 Insights 在 `insights` 顶层输出 `total_duration` 与
  `matched_record_count`，分别对应聚合的总时长和发生次数。Android 将其保存为
  `ActivityAggregate`，活动概览和周期对比直接消费该结果。
- Android、CLI 和其他端侧只负责映射与展示，不得对完整查询范围再次 `sum` 活动
  日数据来重建总时长或发生次数。

按月份分组的 Records 子视图属于已查询周期内的本地展示子集；如果未来需要把某个
子分组作为独立查询范围使用，必须新增 Core 聚合输出，而不是把展示层的局部累计
当作全局统计来源。

## 代码落点

- Core 聚合模型：
  `libs/tracer_core/src/domain/insights/models/activity_aggregate.hpp`
- 日/周期 Insights 模型：
  `libs/tracer_core/src/domain/insights/models/daily_insights_data.hpp`、
  `range_insights_data.hpp`
- Trend 与 Breakdown 统计：
  `libs/tracer_core/src/infra/query/data/stats/insights_chart_stats_calculator.cpp`
- Android 结构化结果映射：
  `apps/android/runtime/src/main/java/com/example/tracer/runtime/translators/StructuredInsightsJsonDecoder.kt`、
  `StructuredInsightsResultParser.kt`
- Android 活动概览消费：
  `apps/android/feature-insights/src/main/java/com/example/tracer/ui/screen/InsightsPeriodActivityBrowser.kt`

## 测试要求

新增或修改活动统计时，至少覆盖：空数据、零时长活动、多个活动事实、总时长与发生
次数的比例，以及 Trend/Breakdown/结构化周期结果之间的一致性。

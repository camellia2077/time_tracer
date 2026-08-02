# Report Composition Contract v1

## 元信息

1. 版本：`v1`
2. 生效日期：`2026-07-11`
3. 适用 action：`report-composition`
4. 适用输出模式：`output_mode=semantic_json`，以及默认 `text` 模式中的 JSON content

## 目的

定义当前时间窗口的完整加权活动树；树是唯一的端侧分布数据源。

## 顶层字段

1. `lookback_days` (`int`, 必填)
2. `from_date`、`to_date` (`string`, `YYYY-MM-DD`，必填成对出现)
3. `total_duration_seconds` (`int`, 必填，非负)
4. `active_root_count` (`int`, 必填，非负)
5. `active_days` (`int`, 必填，非负)：筛选范围内至少有一条记录的日期数；平均时长的分母。
6. `range_days` (`int`, 必填，非负)：筛选范围的自然日数。
7. `tree` (`object[]`, 必填)：完整、未折叠的加权活动树

## `tree[]`

每个节点包含：

1. `name` (`string`, 必填)
2. `duration_seconds` (`int`, 必填，非负，单位：秒)
3. `occurrence_count` (`int`, 必填，非负)：该活动路径在查询范围内直接匹配的 `time_records` 条数；父节点为其所有后代路径的累计次数。
4. `average_duration_seconds` (`int`, 必填，非负)：`duration_seconds / active_days`，`active_days=0` 时为 `0`。
5. `average_occurrence_count` (`number`, 必填，非负)：`occurrence_count / active_days`，`active_days=0` 时为 `0`。
6. `average_occurrence_ratio` (`number`, 必填，`0..1`)：当前节点记录次数除以当前层级所有节点记录次数；当前层级没有记录时为 `0`。
7. `children` (`object[]`, 必填，递归使用同一结构)

`tree` 不得使用 `Others` 折叠，且按照节点名称稳定排序。端侧从根节点或当前节点的 `children` 直接构建所有分布图；时长分布使用 `duration_seconds`，频率分布使用 `occurrence_count`。点击含有 `children` 的节点后进入下一层，叶节点没有下钻目标。

## 空数据

1. 空数据时保留完整结构：`tree=[]`，统计字段为 `0`。
2. 不提供 `slices` 字段，也不支持以扁平根节点分布作为回退。

## 代码落点

1. Core 输出：`libs/tracer_core/src/infra/query/data/repository/query_runtime_service_report_mapping.cpp`
2. Android 解析：`apps/android/runtime/src/main/java/com/example/tracer/runtime/NativeRuntimeQueryParsing.kt`
3. Android 饼图下钻：`apps/android/feature-report/src/main/java/com/example/tracer/ui/screen/ReportCompositionVisualizationSection.kt`

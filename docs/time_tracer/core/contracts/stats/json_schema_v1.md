# Core Stats Semantic JSON Schema v1

## 元信息
1. 版本：`v1`
2. 生效日期：`2026-02-22`
3. 适用输出模式：`output_mode=semantic_json`
4. `insights-chart` 专项字段约束见：`docs/time_tracer/core/contracts/stats/insights_chart_contract_v1.md`

## 通用包络
语义 JSON 顶层对象应包含以下基础字段：

```json
{
  "schema_version": 1,
  "action": "days_stats",
  "output_mode": "semantic_json"
}
```

## 按 action 的数据结构

## `years|months|days|search`
```json
{
  "schema_version": 1,
  "action": "years",
  "output_mode": "semantic_json",
  "items": ["2024", "2025"],
  "total_count": 2
}
```

## `days_duration`
```json
{
  "schema_version": 1,
  "action": "days_duration",
  "output_mode": "semantic_json",
  "rows": [
    { "date": "2026-02-20", "duration_seconds": 3600 }
  ],
  "total_count": 1
}
```

## `days_stats`
```json
{
  "schema_version": 1,
  "action": "days_stats",
  "output_mode": "semantic_json",
  "stats": {
    "count": 3,
    "mean_seconds": 1800.0,
    "variance_seconds": 120000.0,
    "stddev_seconds": 346.41,
    "median_seconds": 1700.0,
    "p25_seconds": 1500.0,
    "p75_seconds": 2100.0,
    "p90_seconds": 2200.0,
    "p95_seconds": 2200.0,
    "min_seconds": 1500.0,
    "max_seconds": 2200.0,
    "iqr_seconds": 600.0,
    "mad_seconds": 200.0
  },
  "rows": [
    { "date": "2026-02-18", "duration_seconds": 1500 }
  ],
  "total_count": 3,
  "top_n_requested": 2,
  "top_longest_rows": [
    { "date": "2026-02-20", "duration_seconds": 2200 }
  ],
  "top_shortest_rows": [
    { "date": "2026-02-18", "duration_seconds": 1500 }
  ]
}
```

说明：
1. `top_n_requested`、`top_longest_rows`、`top_shortest_rows` 仅在请求 `top_n>0` 时出现。
2. `rows` 为参与统计的样本序列（非仅 top N 子集）。

## `insights_chart`
`insights-chart` 的字段定义与跨端口径以
`docs/time_tracer/core/contracts/stats/insights_chart_contract_v1.md` 为准。

```json
{
  "schema_version": 1,
  "action": "insights_chart",
  "output_mode": "semantic_json",
  "roots": ["study", "sleep"],
  "root_tree": [
    {
      "name": "study",
      "path": "study",
      "duration_seconds": 12600,
      "children": []
    }
  ],
  "selected_root": "study",
  "lookback_days": 7,
  "from_date": "2026-02-16",
  "to_date": "2026-02-22",
  "average_duration_seconds": 1800,
  "total_occurrence_count": 7,
  "average_duration_per_occurrence_seconds": 1800,
  "mode_duration_seconds": 1800.0,
  "median_duration_seconds": 1800.0,
  "minimum_duration_seconds": 0.0,
  "maximum_duration_seconds": 3600.0,
  "lower_quartile_duration_seconds": 900.0,
  "upper_quartile_duration_seconds": 2700.0,
  "coefficient_of_variation": 0.8165,
  "mean_absolute_deviation_seconds": 1200.0,
  "total_duration_seconds": 12600,
  "active_days": 5,
  "range_days": 7,
  "series": [
    { "date": "2026-02-16", "duration_seconds": 2400, "epoch_day": 20400 }
  ]
}
```

说明：
1. `average_duration_seconds` 按 `active_days` 计算；没有任何活动记录的日期不计入分母。
2. `average_duration_per_occurrence_seconds = total_duration_seconds / total_occurrence_count`；发生次数为 0 时为 0。
3. 空数据时 `series` 可为空，统计字段为 0。
4. `series[].epoch_day` 为可选字段，用于端侧优化 x 轴渲染。
5. `mode_duration_seconds`、`median_duration_seconds`、`minimum_duration_seconds`、`maximum_duration_seconds`、`lower_quartile_duration_seconds`、`upper_quartile_duration_seconds`、`coefficient_of_variation`、`mean_absolute_deviation_seconds` 基于实际发生过活动事实的日期时长计算，不包含补齐的 0 时长日期；发生事实但时长为 0 的日期仍计入。无重复样本时 `mode_duration_seconds` 为 `null`；CV 使用活动样本的总体标准差除以平均值，四分位数采用线性插值，平均绝对偏差以均值为中心。

## `tree`
```json
{
  "schema_version": 1,
  "action": "tree",
  "output_mode": "semantic_json",
  "max_depth": -1,
  "max_available_depth": 1,
  "root_count": 1,
  "roots": [
    {
      "name": "study",
      "duration_seconds": 3600,
      "children": [
        {
          "name": "cpp",
          "duration_seconds": 1800,
          "parent_duration_percent": 50.0,
          "children": []
        }
      ]
    }
  ]
}
```

说明：
1. 每个节点的 `duration_seconds` 为该节点聚合时长，单位为秒。
2. 根节点不包含 `parent_duration_percent`；子节点包含时，该字段表示其时长占直接父节点时长的百分比。
3. `max_available_depth` 表示当前时间范围内完整树结构的最大层级，根节点为第 0 层；
   它与本次请求的 `max_depth` 独立，供端侧构造层级选择项。

## `insights_composition`
`insights-composition` 的完整约束见
`docs/time_tracer/core/contracts/stats/insights_composition_contract_v1.md`。

```json
{
  "schema_version": 1,
  "action": "insights_composition",
  "output_mode": "semantic_json",
  "total_duration_seconds": 7200,
  "active_root_count": 1,
  "active_days": 2,
  "range_days": 7,
  "tree": [
    {
      "name": "study",
          "duration_seconds": 7200,
          "occurrence_count": 2,
          "average_duration_seconds": 3600,
          "average_duration_per_occurrence_seconds": 3600,
      "average_occurrence_count": 1.0,
      "average_occurrence_ratio": 1.0,
      "children": [
        { "name": "cpp", "duration_seconds": 3600, "occurrence_count": 1,
          "average_duration_seconds": 1800, "average_duration_per_occurrence_seconds": 3600,
          "average_occurrence_count": 0.5,
          "average_occurrence_ratio": 1.0,
          "children": [] }
      ]
    }
  ]
}
```

说明：
1. `tree` 是唯一的分布数据源，始终保留未折叠的完整加权活动树。
2. 端侧从根节点或当前节点的 `children` 逐层构建分布图和下钻路径；可按 `duration_seconds` 或 `occurrence_count` 统计分布。
3. `average_duration_seconds` 和 `average_occurrence_count` 使用 `active_days` 作为分母；`average_duration_per_occurrence_seconds` 使用节点 `occurrence_count` 作为分母；`average_occurrence_ratio` 是当前层级的记录次数占比。

## 兼容性规则
1. `text` 模式不受此文档约束。
2. 语义 JSON 在 v1 内遵循“字段追加优先”。
3. 对非对象内容的兼容封装允许出现 `raw_content` 或 `data` 字段。
4. 版本升级与兼容窗口规则详见：`docs/time_tracer/core/contracts/stats/semantic_json_versioning_policy.md`。

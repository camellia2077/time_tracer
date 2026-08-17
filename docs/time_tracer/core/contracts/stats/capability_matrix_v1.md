# Core Stats Capability Matrix v1

## 元信息
1. 版本：`v1`
2. 生效日期：`2026-02-22`
3. 适用范围：`DataQuery`（Core/CLI/Android 统一统计契约）

## 能力矩阵（按 action）

| action | 主要输入 | 统计计算归属 | semantic_json 关键字段 | 空数据/fallback 约定 |
| --- | --- | --- | --- | --- |
| `years` | 可选过滤：无 | 无统计计算 | `items`, `total_count` | `items=[]`, `total_count=0` |
| `months` | `year` | 无统计计算 | `items`, `total_count` | `items=[]`, `total_count=0` |
| `days` | `year/month/from/to/reverse/limit` | 无统计计算 | `items`, `total_count` | `items=[]`, `total_count=0` |
| `days-duration` | `year/month/from/to/root/...` | 日汇总（按日聚合） | `rows[].date`, `rows[].duration_seconds`, `total_count` | `rows=[]`, `total_count=0` |
| `days-stats` | `period/period_arg/root/top_n/...` | `stats/day_duration_stats_calculator.*` | `stats.*`, `rows`, `total_count`, `top_*` | `stats.count=0`，其余统计值为 0 |
| `insights-chart` | `lookback_days` 或 `from/to` + 可选 `root` | `stats/insights_chart_stats_calculator.*` | `series`, `total_duration_seconds`, `total_occurrence_count`, `average_duration_seconds`, `average_duration_per_occurrence_seconds`, `mode_duration_seconds`, `median_duration_seconds`, `minimum_duration_seconds`, `maximum_duration_seconds`, `lower_quartile_duration_seconds`, `upper_quartile_duration_seconds`, `coefficient_of_variation`, `mean_absolute_deviation_seconds`, `active_days`, `range_days` | 缺 root/无数据时 `series=[]`；数值统计为 0，`mode_duration_seconds=null` |
| `insights-composition` | `lookback_days` 或 `from/to` | 项目树聚合与层级平均统计 | `total_duration_seconds`, `active_root_count`, `active_days`, `range_days`, `tree[].average_duration_seconds`, `tree[].average_duration_per_occurrence_seconds`, `tree[].average_occurrence_count`, `tree[].average_occurrence_ratio` | `tree=[]` 且统计值为 0 |
| `tree` | `period/period_arg/level/root/...` | 树聚合与节点父级占比计算 | `roots[].duration_seconds`, `roots[].children[].parent_duration_percent`, `root_count`, `max_depth`, `max_available_depth` | `roots=[]`, `root_count=0`, `max_available_depth=0` |
| `search` | `remark/day_remark/project/root/...` | 无统计计算 | `items`, `total_count` | `items=[]`, `total_count=0` |
| `activity-frequent` | `lookback_days/top/prefix/score_mode` | 评分聚合（核心查询层） | `items[].score`, `total_count` | `items=[]`, `total_count=0` |

## 边界说明
1. 统计公式只允许存在于 `libs/tracer_core/src/infra/query/data/stats/`。
2. 时间范围解析与 action 编排只允许存在于 `libs/tracer_core/src/infra/query/data/orchestrators/`。
3. 文本与语义 JSON 渲染只允许存在于 `libs/tracer_core/src/infra/query/data/renderers/`。
4. Core/CLI/Android adapter 只做请求映射、调用转发、结果解码与错误映射。

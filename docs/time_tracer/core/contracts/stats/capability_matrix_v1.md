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
| `report-chart` | `lookback_days` 或 `from/to` + 可选 `root` | `stats/report_chart_stats_calculator.*` | `series`, `total_duration_seconds`, `average_duration_seconds`, `active_days`, `range_days` | 缺 root/无数据时 `series=[]` 且统计值为 0 |
| `tree` | `period/period_arg/level/root/...` | 无统计公式，仅树聚合与渲染 | `roots`, `root_count`, `max_depth` | `roots=[]`, `root_count=0` |
| `search` | `remark/day_remark/project/root/...` | 无统计计算 | `items`, `total_count` | `items=[]`, `total_count=0` |
| `activity-suggest` | `lookback_days/top/prefix/score_mode` | 评分聚合（核心查询层） | `items[].score`, `total_count` | `items=[]`, `total_count=0` |

## 边界说明
1. 统计公式只允许存在于 `apps/time_tracer/src/infrastructure/query/data/stats/`。
2. 时间范围解析与 action 编排只允许存在于 `apps/time_tracer/src/infrastructure/query/data/orchestrators/`。
3. 文本与语义 JSON 渲染只允许存在于 `apps/time_tracer/src/infrastructure/query/data/renderers/`。
4. Core/CLI/Android adapter 只做请求映射、调用转发、结果解码与错误映射。

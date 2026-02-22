# Core Stats Capability Contract v1

## 元信息
1. 版本：`v1`
2. 生效日期：`2026-02-22`
3. 适用范围：`DataQuery` 统计与语义输出（跨 CLI / Android）

## 目标
1. 统一统计口径，避免多端各自计算产生偏差。
2. 将“计算语义”与“文本渲染”分离，便于自动化消费。
3. 在兼容默认 `text` 输出的前提下，提供稳定 `semantic_json` 契约。

## 支持能力
1. 列表查询：`years`、`months`、`days`、`search`
2. 时长序列：`days-duration`
3. 统计汇总：`days-stats`
4. 图表统计：`report-chart`
5. 项目树：`tree`

## 输入语义
1. `output_mode`
   - 默认：`text`
   - 语义模式：`semantic_json`
2. `root` 过滤
   - `root` 非空时优先使用 `root`。
   - 否则回退使用 `project` 的 root 语义。
3. `report-chart` 时间范围
   - 显式范围要求同时提供 `from_date` 与 `to_date`。
   - 范围为闭区间（包含首尾日期）。
   - 要求 `from_date <= to_date`。
   - 未提供显式范围时，使用滚动窗口（默认 `lookback_days=7`）。

## 统计口径定义
1. 通用
   - 时间单位统一为 `seconds`。
   - 空数据时返回 0 或空数组，不返回未定义值。
2. `report-chart`
   - `total_duration_seconds = sum(series[].duration_seconds)`
   - `range_days = 日期范围内的总天数（闭区间）`
   - `active_days = duration_seconds > 0 的天数`
   - `average_duration_seconds = total_duration_seconds / range_days`（`range_days=0` 时为 `0`）
3. `days-stats`（基于过滤后的全部样本，不受 `limit/reverse` 截断）
   - `count`：样本数
   - `mean_seconds`：均值
   - `variance_seconds`：总体方差（分母 `N`）
   - `stddev_seconds`：总体标准差（`sqrt(variance_seconds)`）
   - `median_seconds`：中位数
   - `p25/p75/p90/p95_seconds`：百分位（nearest-rank）
   - `iqr_seconds = p75_seconds - p25_seconds`
   - `mad_seconds`：相对中位数的绝对偏差中位数
4. `days-stats` 的 `top_n`
   - `top_longest_rows`：最长时长前 N
   - `top_shortest_rows`：最短时长前 N

## 错误与边界
1. `report-chart` 仅提供 `from_date` 或仅提供 `to_date`：返回错误。
2. `report-chart` 日期非法或范围反转：返回错误。
3. 无可用数据时：返回结构化空结果（字段存在，值为 0/空数组）。

## 兼容与演进
1. 默认行为兼容：未显式指定 `output_mode` 时仍走 `text`。
2. 语义字段演进原则：优先追加字段，避免删除或重命名既有字段。
3. 若发生破坏性变更，必须升级 schema 版本并在 history 文档说明迁移窗口。
4. 版本升级流程详见：`docs/time_tracer/core/contracts/stats/semantic_json_versioning_policy.md`。

## 相关文档
1. 能力矩阵：`docs/time_tracer/core/contracts/stats/capability_matrix_v1.md`
2. JSON 字段：`docs/time_tracer/core/contracts/stats/json_schema_v1.md`
3. 代码映射：`docs/time_tracer/core/contracts/stats/code_map.md`

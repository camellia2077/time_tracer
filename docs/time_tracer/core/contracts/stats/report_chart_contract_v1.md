# Report Chart Contract v1

## 元信息
1. 版本：`v1`
2. 生效日期：`2026-02-23`
3. 适用 action：`report-chart`
4. 适用输出模式：`output_mode=semantic_json`

## 目的
1. 定义 `report-chart` 的跨端统一数据契约（Core -> Android -> Windows）。
2. 明确 Core 负责“可直接渲染的数据与统计口径”，端侧只做渲染。
3. 作为 `report-chart` 字段约束的专用入口，避免字段语义漂移。

## 顶层字段（v1）
1. `schema_version` (`int`, 必填，当前为 `1`)
2. `action` (`string`, 必填，固定 `report_chart` 或兼容 token `report-chart`)
3. `output_mode` (`string`, 必填，固定 `semantic_json`)
4. `roots` (`string[]`, 必填)
5. `selected_root` (`string`, 必填，可为空字符串)
6. `lookback_days` (`int`, 必填，未提供显式范围时用于窗口语义)
7. `from_date` (`string`, 可选，`YYYY-MM-DD`)
8. `to_date` (`string`, 可选，`YYYY-MM-DD`)
9. `average_duration_seconds` (`int`, 必填)
10. `total_duration_seconds` (`int`, 必填)
11. `active_days` (`int`, 必填)
12. `range_days` (`int`, 必填)
13. `series` (`object[]`, 必填)

## `series[]` 字段（v1）
1. `date` (`string`, 必填，`YYYY-MM-DD`)
2. `duration_seconds` (`int`, 必填，非负)
3. `epoch_day` (`int`, 可选，保留为后续跨端渲染优化字段)

## `report-composition` 展示语义

`report-composition` 返回的加权 `tree` 之外，还必须提供以下展示字段：

1. `display_level` (`int`, 必填，默认 `0`)
2. `display_path` (`string[]`, 必填，默认 `[]`)

Core 根据当前报告日期范围内的有效 parent 根数量计算这两个字段：

1. 只有一个有效 parent 且该 parent 有子节点时，返回 `display_level=1`，并将该 parent
   放入 `display_path`，表示客户端初始显示该 parent 的下一层活动。
2. 存在多个有效 parent 时，返回 `display_level=0`、`display_path=[]`。
3. 整个数据库可以存在多个 parent；如果当前日期范围内只有一个有效 parent，
   仍按第 1 条自动下钻。

Android 等客户端必须直接消费 `display_path` 作为初始展示路径，不根据
TOML 文件名或自身重新判断 hierarchy 深度。

## 口径约束
1. `total_duration_seconds = sum(series[].duration_seconds)`
2. `range_days = 日期范围闭区间天数`
3. `active_days = 至少存在一条活动事实（包括 end-only）的天数`
4. `average_duration_seconds = total_duration_seconds / active_days`（`active_days=0` 时为 `0`；没有活动记录的日期不计入分母）

## 空数据约束
1. 空数据时返回完整结构，不省略关键字段。
2. `series=[]`，统计字段统一为 `0`。
3. `selected_root` 应回显请求值（若无请求则为空字符串）。

## 分层边界
1. Core：负责查询、聚合、统计、契约输出。
2. Android/Windows：负责 UI/HTML 渲染，不重复实现统计公式。
3. 若新增统计字段，应优先在 Core 扩展，并同步文档与测试。

## 兼容策略
1. v1 内采用“字段追加优先”，避免删除/重命名既有字段。
2. 端侧应允许读取未知字段并忽略。
3. 破坏性调整必须升级 schema 版本并遵循：
   - `docs/time_tracer/core/contracts/stats/semantic_json_versioning_policy.md`

## 代码落点
1. Core 映射入口：
   - `libs/tracer_core/src/infra/query/data/repository/query_runtime_service_report_mapping.cpp`
2. Android 解析入口：
   - `apps/android/runtime/src/main/java/com/example/tracer/runtime/NativeRuntimeQueryParsing.kt`
3. Windows 查询适配入口：
   - `apps/cli/windows/src/api/cli/impl/commands/query/data_query_parser.cpp`

## 相关文档
1. Stats 文档索引：`docs/time_tracer/core/contracts/stats/README.md`
2. 通用 schema：`docs/time_tracer/core/contracts/stats/json_schema_v1.md`
3. 能力契约：`docs/time_tracer/core/contracts/stats/capability_contract_v1.md`
4. 能力矩阵：`docs/time_tracer/core/contracts/stats/capability_matrix_v1.md`


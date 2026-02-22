# Adapter Reviewer Checklist

用于评审 `DataQuery` 相关改动是否仍符合“统计计算 / 时间范围编排 / 格式渲染 / 入口适配”边界。

## A. 目录边界
1. 统计公式只在 `query/data/stats/` 修改。
2. 时间范围解析只在 `query/data/orchestrators/` 修改。
3. 文本/语义渲染只在 `query/data/renderers/` 修改。
4. `sqlite_data_query_service*`、CLI、Android adapter 不新增统计公式实现。

## B. 适配层改动检查
1. CLI/Android 改动是否仅为参数映射、调用透传、响应解码、提示文案。
2. 是否出现 `mean/variance/stddev/mad/percentile` 等统计字段的重新计算。
3. 是否出现日期范围推导逻辑重复实现（应复用 core 结果）。
4. 是否在 adapter 层引入 SQL、聚合、排序公式等业务逻辑。

## C. fallback 行为
1. 空数据返回是否保持结构化字段完整（而非空字符串或缺字段）。
2. root 不存在、root 为空、显式范围非法等场景是否有稳定输出/错误。
3. `text` 与 `semantic_json` 是否行为一致（仅表现形式不同）。

## D. 测试与文档
1. `apps/time_tracer/src/infrastructure/tests/data_query_refactor_tests.cpp` 是否同步更新。
2. `test/suites/tracer_windows_cli/tests/commands_query_data.toml` 是否补充关键回归场景。
3. `docs/time_tracer/core/contracts/stats/*.md` 是否同步更新（能力、schema、版本策略）。
4. 版本历史是否补充迁移说明（`docs/time_tracer/history/`）。

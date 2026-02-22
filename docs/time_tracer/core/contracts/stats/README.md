# Core Stats 文档索引

本目录定义 `time_tracer` 内核统计能力的跨端契约，用于 CLI / Android / 脚本工具统一复用。

## 文档列表
1. `docs/time_tracer/core/contracts/stats/capability_contract_v1.md`
   - 统计能力与口径定义（平均值、方差、标准差、root 过滤、空数据行为等）。
2. `docs/time_tracer/core/contracts/stats/json_schema_v1.md`
   - `semantic_json` 输出字段与示例。
3. `docs/time_tracer/core/contracts/stats/capability_matrix_v1.md`
   - 按 action 的能力矩阵、字段契约和 fallback 约定。
4. `docs/time_tracer/core/contracts/stats/semantic_json_versioning_policy.md`
   - `semantic_json` 版本升级与破坏性变更规则。
5. `docs/time_tracer/core/contracts/stats/adapter_code_map.md`
   - Core/CLI/Android adapter 的职责落点图。
6. `docs/time_tracer/core/contracts/stats/adapter_reviewer_checklist.md`
   - 评审清单与边界守卫检查项。
7. `docs/time_tracer/core/contracts/stats/code_map.md`
   - Core/CLI/Android 的统计相关代码落点与职责映射。

## 使用建议
1. 先读能力契约，再接 JSON 字段，最后看代码映射。
2. 新增统计字段时，必须同步更新契约文档和测试基线。
3. 默认输出模式仍为 `text`，跨端集成建议优先使用 `semantic_json`。

# Report Data Consistency Spec v1

状态：`active`  
生效日期：`2026-02-25`  
适用范围：`tracer_core` 的日报/周报/月报/区间报表数据生成路径（含 Android/Windows CLI 共享内核）

## 1. 目标
本规范用于防止“同一份报告内部各区块口径不一致”的逻辑错误。  
本规范是执行标准，不是仅供参考的经验文档。

## 2. 术语
1. `单份报告快照`：一次 report query 过程中用于生成输出的完整数据视图。
2. `路径真相源`：项目路径（如 `study_math_linear-algebra`）的唯一来源。
3. `长生命周期 Runtime`：同一进程内多次 ingest/query，不重启进程（Android 典型）。

## 3. 强制约束（MUST）
1. 同一份报告内，`All Activities` 与 `Project Breakdown` 必须来自同一份语义快照。
2. 报告生成路径中，项目路径只能有一个 `路径真相源`，禁止混用“实时联表路径”和“陈旧缓存路径”。
3. 任何进程级可变缓存（例如项目名缓存）必须定义并实现失效/刷新策略。
4. ingest（含 `single_txt_replace_month`）成功提交后，后续 report query 不得读取旧缓存映射。
5. 代码评审若发现“同一报表多条数据真相链路”，必须阻断合并。

## 4. 实现要求（MUST/SHOULD）
1. 缓存策略：
   1. `MUST` 支持刷新（每次查询刷新或版本化失效二选一）。
   2. `MUST` 在 DB 变更后保证下一次查询可见新映射。
   3. `SHOULD` 避免无版本号的全局单例长期驻留快照。
2. 报告查询：
   1. `MUST` 保证同一次查询里记录列表与树聚合使用同一映射上下文。
   2. `SHOULD` 在 query 层组装统一 payload，再交给 formatter，避免 formatter 再查 DB。
3. 可观测性：
   1. `MUST` 在路径映射缺失时记录明确错误（含 project_id / query 参数）。
   2. `SHOULD` 在调试日志中输出“缓存是否刷新”状态。

## 5. 回归测试契约（Release Gate）
以下场景未覆盖，禁止发布：
1. 同进程连续执行：ingest -> report -> replace_month ingest -> report，验证两次 `Project Breakdown` 都与 `All Activities` 一致。
2. 人为修改 `projects` 映射后再次查询，验证缓存刷新后结果生效。
3. Android 长生命周期路径必须有自动化测试，不得只依赖 CLI 短进程验证。

建议最少落点：
1. `apps/tracer_core/src/infrastructure/tests/android_runtime/android_runtime_business_regression_tests.cpp`
2. `apps/tracer_core/src/infrastructure/tests/android_runtime/android_runtime_smoke_io_tests.cpp`
3. `apps/tracer_core/src/infrastructure/tests/android_runtime/android_runtime_report_consistency_tests.cpp`

## 5.5 跨端 MD 报告一致性保障（Windows CLI / Android）

### 5.5.1 架构前提

Android 和 Windows CLI **共享同一个 `tracer_core` 原生报告内核**：

```
Android: RuntimeReportDelegate.kt → JNI → tracer_core_runtime_report_json()
Windows: cli_runtime_factory_proxy.cpp → tracer_core_runtime_report_json()
```

因此两端生成报告时走的 querier / formatter 代码完全相同，天然保证了逻辑一致性。

### 5.5.2 一致性分层定义

| 层次 | 定义 | 保障等级 | 实现方式 |
|------|------|----------|----------|
| **数据层** | 同一份 txt 导入后，报告的 `total_duration`、`detailed_records` 条数、`project_tree` 节点、各字段值完全一致 | **MUST** | 共享 `DayQuerier` / `BaseQuerier` 同一份 SQL + 映射代码 |
| **结构层** | MD 输出包含相同的 section 标题（`## All Activities`、`## Project Breakdown`、`## Statistics` 等）和标签（`- **Date**:`、`- **Total Time Recorded**:` 等） | **MUST** | 共享 `ReportDtoFormatter` + 同一份 `day.toml` 配置 |
| **格式层** | 文本结构与换行语义一致（UTF-8 + LF + 末尾 LF） | **MUST** | `report_formatter_parity_md_tests.cpp` + `ReportOutputPolicy` |
| **字节层** | 原始字节（含换行符）完全相同，且 `sha256` 一致 | **MUST** | parity/consistency 硬门禁 + 审计脚本 |

### 5.5.3 字节层门禁变更说明（2026-02-28）
> 本条在 `2026-02-28` 废弃。当前发布门禁已升级为原始字节全等。
>
> 变更原因：
> 1. 归一化比较会掩盖链路中的非法文本修补行为（`trim/replace/normalize`）。
> 2. 多端一致性目标已升级为“同输入、同格式、同正文字节”。
> 3. `report_hash_sha256` 已进入 runtime 响应，可直接做跨端对账。

### 5.5.4 测试矩阵

| 测试文件 | 覆盖层次 | 方法 |
|----------|----------|------|
| `android_runtime_report_consistency_tests.cpp` — `TestDataLayerStructuredFieldVerification` | 数据层 | ingest 真实 txt → `RunStructuredReportQuery` → 断言 `total_duration > 0`、records 非空、sum(duration) == total、project_tree 有节点 |
| `android_runtime_report_consistency_tests.cpp` — `TestDataLayerCrossIngestConsistency` | 数据层 + 格式层 | 同 runtime 内 ingest → report → replace_month → report，断言数据字段不变 + 归一化 MD 内容不变 |
| `android_runtime_report_consistency_tests.cpp` — `TestStructureLayerMdSectionIntegrity` | 结构层 | 断言日报/月报 MD 包含预期 section 标题和标签 |
| `report_formatter_parity_md_tests.cpp` | 格式层 + 字节层 | 同一 fixture，CLI/Android 原始字节一致 + sha256 一致 + golden snapshot |
| `android_runtime_business_regression_tests.cpp` | 数据层 | 项目重命名后缓存刷新、项目表变更后映射更新 |
| `scripts/tools/collect_report_markdown_cases.py` + `report_consistency_audit.py` | 字节层 | 固定六类样本（day/month/week/year/recent/range）对 golden 做字节级审计 |
| `scripts/tools/report_markdown_render_snapshot_check.py` | 结构层（渲染） | 固定六类样本做标题/列表/代码块/表格/空行结构快照对比 |


## 6. 代码评审检查单（必须逐项勾选）
1. 是否引入了新的进程级可变缓存？若是，是否定义失效策略与触发点？
2. 同一报表的多区块是否共享同一数据快照？
3. 是否出现“区块 A 直接查 DB、区块 B 走缓存”的双真相路径？
4. ingest/replace 之后是否有明确的缓存刷新保证？
5. 是否新增了对应自动化回归测试？
6. 是否更新了 `docs/time_tracer/history/` 的版本记录？

## 7. 本次事故（2026-02）归档
1. 现象：
   1. `All Activities` 正确。
   2. `Project Breakdown` 出现与当天活动无关的旧项目（如 `douyin/game`），且遗漏应有项目（如 `linear-algebra`）。
2. 根因：
   1. `All Activities` 路径来自日报查询的实时联表。
   2. `Project Breakdown` 路径来自 `project_id -> ProjectNameCache`。
   3. `ProjectNameCache` 为进程级单例且仅首次加载，Android 长生命周期下发生陈旧映射。
3. 修复：
   1. 将 `ProjectNameCache::EnsureLoaded` 改为每次刷新 DB 快照。
   2. 增加“项目表变化后缓存必须刷新”的回归测试。
4. 相关代码：
   1. `apps/tracer_core/src/infrastructure/reports/data/cache/project_name_cache.hpp`
   2. `apps/tracer_core/src/infrastructure/tests/android_runtime/android_runtime_business_regression_tests.cpp`

## 8. 变更要求
后续若修改以下任一模块，必须同步检查本规范：
1. `infrastructure/reports/data/queriers/*`
2. `infrastructure/reports/data/cache/*`
3. ingest -> import -> report 的调用链与生命周期管理

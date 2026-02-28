# Domain 层架构：核心模型与业务规则

本篇文档旨在深入剖析 Time Tracer 的核心 `domain` 领域层。
该层完全与外部世界（文本文件、控制台、SQLite）解耦，专注于表达时间管理的核心概念、计算规则和数据合法性约束。

## 1. 核心领域实体 (Domain Models)

Domain 层的所有实体都存放在 `apps/tracer_core/src/domain/model/` 目录下。

### 1.1 `DailyLog` (日记录实体)
**文件**: `apps/tracer_core/src/domain/model/daily_log.hpp`

由于我们的文本输入是按天（例如 `0101`、`0102`）组织的，`DailyLog` 自然成为了数据转换期最大的容器。
它的生命周期贯穿了整个数据的清洗变形期：
1. **Raw 生成期**：只包含 `rawEvents`（即文本解析器强塞进来的未处理字符串）。
2. **逻辑补充期**：补全 `getupTime`，判断 `isContinuation`（本日是否是为了续写昨日通宵）。
3. **活动塑型期**：所有的 `rawEvents` 利用时间差被转换为标准的 `processedActivities`。
4. **统计产生期**：基于处理好的活动，最终生成一份当天的宏观统计数据 `stats`。

### 1.2 `BaseActivityRecord` (归一化活动记录)
**文件**: `apps/tracer_core/src/domain/model/time_data_models.hpp`

这是每一个具体的活动（例如打游戏、睡觉、学高数）的最终领域形态。
*   `start_time_str` / `end_time_str` / `duration_seconds`: 精确的时长表达。
*   `project_path`: 抛弃原有杂乱的 `description`，统一归档为 `_` 分隔的标准路径（如 `study_math`）。
*   `remark`: 挂载额外注释。

### 1.3 `ActivityStats` (日度统计快照)
**文件**: `apps/tracer_core/src/domain/model/time_data_models.hpp`

这是经过运算后浓缩的统计面板。在 C++ 结构体中，它被扁平化为各项具体指标（如 `study_time`, `sleep_night_time`, `anaerobic_time`），这些数据最终将被直接投入 SQLite 以供快速提取。

---

## 2. 核心转换与运算边界 (Logic - Converter)

当文本变成 `RawEvent` 后，如何把它们搓揉成最终的 `BaseActivityRecord` 呢？
**核心逻辑在**：`apps/tracer_core/src/domain/logic/converter/convert/core/converter_core.cpp`

### 2.1 DayProcessor (单日运算器)
这是每天必须经历的单线程处理机：
*   **计算时长**：前后时间相减，碰到午夜倒置（如 `23:00` 开始跨到 `02:00`）能够自动按 +24 小时公式解算。
*   **同月跨天睡眠处理机制**：在处理当前日期 (`day_to_process`) 时，若前一日期 (`previous_day`) 末尾存在未结束活动，且当前日期包含起床标识，`DayProcessor` 会在当前日期的起始位置自动注入 `sleep_night` 活动块。

### 2.2 LogLinker (换月黏合剂)
当一月份结束，二月份开始，`DayProcessor` 里的局部“前一天”变量就失效了。此时轮到 `LogLinker` 上场。
*   它是**月与月之间的搭路者**。它拥有纵观全局数据桶 (`data_map`) 的能力，专门拦截检测上个月最后一天的最后一项，去和下个月第一天的 `getupTime` 实施握手连接，彻底实现“年经百战不失一旦”的时间连续性。

---

## 3. 防断与安全验证边界 (Logic - Validator)

引擎不留垃圾，这就是 `validator` 存在的意义。只有被它放行的时间数据才被允许向后续传递。

**核心引擎文件**: `apps/tracer_core/src/domain/logic/validator/structure/structure_validator.cpp`

### 3.1 异常过长拦截防误规则 (`ValidateActivityDuration`)
*   **触发条件**：任何单个活动的时长如果在运算后被判定 > 16 小时（例如错写了 PM 导致倒置计算叠加）。
*   **阻断逻辑**：立即抛出 `DiagnosticSeverity::kError` 级错误，中断当前文档流水线（不会中断其他健康文档）。
*   **特赦标签 `@allow-long`**：程序考虑到了极端边缘化用例（比如真实长途昏睡或挂机长途乘车）。只要该事记的 `remark` 中命中特赦串 `@allow-long`，验证器将主动豁免绕过拦截。

### 3.2 历法漏洞审查 (`ValidateDateContinuity`)
*   **连续性审计**：严查月度记录中是否发生了“物理失联”。借助标准库判断润年大小月，以发现诸如缺失了 `2月28号` 后直接蹦到 `3月1号` 的残缺输入，逼迫用户保持数据的完整连续性追踪。

---

**总结**：
Time Tracer 的 Domain 领域层保持高度纯粹。所有输入均转换为由 `DailyLog` 承载的结构化数据，经过计算层 (`DayProcessor`, `LogLinker`) 的逻辑处理与校验层 (`Validator`) 的规则审查，生成准确的时间记录切片。
只有通过该层验证的数据，才有资格进入下一阶段的写入工序。

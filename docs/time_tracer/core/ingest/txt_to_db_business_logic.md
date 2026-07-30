# TXT To DB Business Logic

本文是 `time_tracer` 关于“规定格式 TXT 如何被解析并写入数据库”的权威业务文档。

它回答的核心问题是：

1. 原始 TXT 的格式合同是什么
2. 每一类行在业务上分别表示什么
3. 活动时长如何由点事件或区间事件转换得到
4. `wake_keywords`、跨日补链、跨月补链分别在什么语义下成立
5. `sleep_night` 在什么条件下自动生成
6. 整体流程如何从 `validate -> parse struct -> insert db`

如果实现与本文不一致，应优先把差异视为实现待调整项，而不是反向修改本文业务口径。

## 1. 总览

程序的基本职责是：

1. 读取符合约定格式的月度 TXT
2. 验证文本结构与业务逻辑
3. 解析为带日语义的结构化数据
4. 在必要时补全跨日/跨月睡眠活动
5. 将最终活动事实写入数据库

可以把整条链路理解为：

`TXT raw text -> validated text -> parsed day struct -> generated activities -> DB rows`

关于新区间事件与点事件混用的目标语义，见：

- [interval_event_and_mixed_timeline_semantics.md](/C:/code/time_tracer/docs/time_tracer/core/ingest/interval_event_and_mixed_timeline_semantics.md)

## 2. 原始 TXT 合同

### 2.1 文件粒度

每个 `.txt` 文件只表示一个月份内的活动记录。

文件头固定为：

1. 第一行：`yYYYY`
2. 第二行：`mMM`

示例：

```text
y2025
m01
```

语义：

1. `y2025` 表示该文件所属年份是 `2025`
2. `m01` 表示该文件所属月份是 `1 月`

### 2.2 日期块

一个月内每天的开始由 `dMMDD` 标记。

示例：

```text
d0101
d0102
```

这里的 `dMMDD` 中：

1. `d` 是固定的日标记前缀，用于把日期行和 `HHMM` / `HHMMSS` 事件行区分开
2. `MM` 表示月份
3. `DD` 表示当天日期

但结构层仍要求它写出来，用于形成清晰的日分块。

### 2.3 空行

空行允许存在，主要用于增强可读性。

它不承载业务语义。

## 3. 行类型

一个日块内部，常见行类型有三类：

1. 日期行：`dMMDD`
2. 备注行：以 `//` 开头
3. 事件行：点事件或区间事件

### 3.1 日备注行

因此：

```text
// beta
// alpha
```

表示这一天存在两行日级备注，内部保存为一个包含 LF 的文本。

注意：

1. 日备注不是活动
2. 日备注不参与 authored event 计数
3. 日备注不参与活动时长计算

### 3.2 事件行

目标口径下，事件行分为两类：

1. 点事件：`HHMMSS + activity token`
2. 区间事件：`HHMMSS-HHMMSS + activity token`

为兼容既有 TXT，读取时仍接受四位 `HHMM`。Core 会在解析入口自动补上
`00` 秒并规范化为 `HHMMSS`；后续校验、持续时间和时间戳计算均以秒精度执行。

示例：

```text
0606w
1353睡觉 //remark
0900-1030概率统计
```

其中：

1. `0606`、`1353` 是兼容格式的点事件时间点，解析后分别为 `060600`、`135300`
2. `0900-1030` 是兼容格式的区间事件显式开始/结束时间，解析后为
   `090000-103000`
3. `w`、`睡觉`、`概率统计` 是活动 token
4. `//remark` 是该事件的行内备注

## 4. 行内备注语义

事件行允许带备注。

唯一支持的备注分隔符为 `//`。活动行之后的物理 `//` 行归入最近活动；
空行不会终止这种归属。备注内容中的两个字符 `\n` 保持普通文本。

示例：

```text
1353睡觉 // remark
// 第二行备注
1847概率统计 // 备注
0231吃饭 // apple
```

语义：

1. 活动名分别是 `睡觉`、`概率统计`、`吃饭`
2. 备注分别是 `remark\n第二行备注`、`备注`、`apple`

## 5. 活动时长的核心规则

本系统最终写入数据库的是标准活动区间（start/end/duration）。

在作者态文本中，区间来源有两种：

1. 点事件：
   - 记录“时间点 + 活动 token”
   - 其持续时间由“最后已知时间边界 -> 当前时间点”推导
2. 区间事件：
   - 记录“开始时间 - 结束时间 + 活动 token”
   - 其持续时间由显式 `start -> end` 直接给出

在只有点事件的传统模式下，核心换算规则仍然可以理解为：

> 上一个已知时间边界与当前时间点共同决定当前活动持续时间。

在引入区间事件后：

1. 区间事件会把最后已知时间边界推进到自己的显式结束时间
2. 后续点事件若继续记录，则应从这个结束时间起算
3. 两个已记录区间之间允许存在未记录时间缺口
4. 未记录时间不自动归属给任何活动，也不参与统计

例如：

```text
0606w
1353睡觉 //remark
```

这里：

1. 当前活动是 `睡觉`
2. 备注是 `remark`
3. `睡觉` 的持续时间来自 `06:06 -> 13:53`

混用示例：

```text
0606w
0900-1030概率统计
1353睡觉
```

这里：

1. `0900-1030概率统计` 是显式区间事件
2. 该区间结束后，最后已知时间边界推进到 `10:30`
3. 后续点事件 `1353睡觉` 的持续时间应来自 `10:30 -> 13:53`

换算为秒：

`(13 - 6) * 3600 + (53 - 6) * 60`

最终写入数据库时，持续时间以秒存储。

## 6. 活动名合法性

普通活动名的合法性基于：

`test/data/activity_hierarchy/*.toml` for test fixtures or
`assets/tracer_core/defaults/activity_hierarchy/*.toml` for distribution defaults
defaults. Generated platform copies are not sources of truth.

这里的核心语义是：

1. alias TOML 使用 canonical-key-first 格式：canonical key 位于左侧，右侧
   是一个或多个 alias 的字符串数组，例如 `"running" = ["跑步", "run"]`
2. canonical key 和 alias 数组中的字符串共同组成普通活动可识别 token 集合
3. 这些 token 在后续转换时映射到项目路径或规范活动名；旧的
   `alias = "canonical"` 格式不再是有效配置

但有一类特殊 token 不属于“普通活动映射”，而属于 wake 语义集合，见下一节。

## 7. Wake 语义

### 7.1 配置来源

wake 语义只来源于运行时的：

`<filesDir>/tracer_core/config/activity_hierarchy/_system.toml`

仓库中的 distribution 默认 seed 位于
`assets/tracer_core/defaults/activity_hierarchy/_system.toml`。

中的：

```toml
[sleep_inference]
wake_keywords = ["起床", "醒", "w", "wake", "新的一天开始了"]
```

这表示：

1. 这些 token 在语义上表示“起床锚点”
2. wake 判定只依赖 `wake_keywords`
3. alias child files 不参与 wake 分类本身

### 7.2 作者态可输入集合

虽然 wake 分类只看 `wake_keywords`，但作者态允许输入的 token 集合定义为：

`authorable_event_tokens = alias_files.keys ∪ wake_keywords`

因此：

1. 普通活动 token 可以直接输入
2. wake token 也可以直接输入
3. “能否输入”与“是否 wake”是两个不同问题

### 7.3 位置约束

wake 相关活动只能是一天中的第一个语义活动。

也就是说：

1. 若某天首个语义活动命中 `wake_keywords`，该天是 wake-start day
2. 若 wake 出现在当天后续活动位置，逻辑校验失败

这条规则属于 ingest 业务要求。

## 8. `sleep_night` 的自动生成

在 `activity_hierarchy/_system.toml` 中有：

```toml
[sleep_inference]
sleep_project_path = "sleep_night"
```

语义：

1. 当系统识别到“这是一次合法的跨日睡眠补链”时
2. 自动生成的睡眠活动名使用 `sleep_night`

### 8.1 什么时候生成

哪个日期命中 wake 语义，就在哪个日期尝试生成 `sleep_night`。

生成条件同时满足时成立：

1. 当天首个语义活动命中 `wake_keywords`
2. 能拿到上一条可用上下文时间点
3. 当前 wake 时间有效

此时使用：

`上一天最后一个活动时间点 -> 当前 wake 时间点`

做差，得到自动补出的睡眠时长。

### 8.2 什么时候不生成

以下情况不生成 `sleep_night`：

1. 该天首个语义活动不是 wake
2. 没有上一天末事件，无法建立可靠跨日时间差
3. wake 出现在当天非首位，这属于非法输入而不是“缺少睡眠补链”

如果某天的第一条活动不是 `wake_keywords` 中的内容，则表示：

1. 这一天不是 wake-start day
2. 这一天不自动生成 `sleep_night`

## 9. 跨日与跨月补链

### 9.1 跨日

当某天首个语义活动是 wake 时：

1. 当前 wake 时间属于当天
2. 其前一段睡眠来自上一天最后一个活动时间点

因此，睡眠时长并不是“当天内部两条事件”的差值，而是：

`previous context -> current wake`

### 9.2 跨月

若当前月份的第一天需要补链，且存在上一个月份的 TXT，则可使用：

1. 上一个月份最后一天的最后一个活动
2. 当前月份第一天的第一个语义活动

来计算跨月首段时间差。

因此，跨月处理本质上是跨日逻辑在“月边界”上的延续，而不是另一套独立规则。

## 10. 阶段职责

整条 ingest 链路可分为四个阶段理解。

### 10.1 结构校验

负责：

1. 检查 `yYYYY` / `mMM` / `dMMDD` / 事件行等格式是否合法
2. 检查头部顺序是否正确
3. 检查文本是否满足基础结构约束
4. 检查事件行属于合法点事件或合法区间事件格式

### 10.2 逻辑校验

负责：

1. 检查活动名是否在允许范围内
2. 检查 wake 是否只出现在当天第一个语义活动位置
3. 检查时长、顺序、跨日语义是否成立
4. 在目标语义下允许未记录时间缺口，但不允许已记录区间重叠

“一天少于 2 条 authored events”不再属于阻断性逻辑错误。

### 10.3 解析为 struct

负责：

1. 将原始文本解析为日级结构
2. 建立 `rawEvents`
3. 根据首个语义活动建立 `getupTime` 或 `isContinuation`
4. 在未来支持区间事件后，同时保留点事件与区间事件的作者态信息

### 10.4 插入数据库

负责：

1. 在 struct 层根据上下文生成 `sleep_night`
2. 将最终活动事实转换为数据库写入行
3. 以秒为单位写入时长

因此，`sleep_night` 的自动补全发生在“解析后、入库前”的业务阶段，而不是原始文本层。

## 11. 示例解释

示例：

```text
y2025
m01

d0101
// beta
// alpha
0606w
1353睡觉 // remark
// 第二行活动备注
1847概率统计 // 备注
0226bilibili
0227洗漱
0228线性代数
0230上厕所
0231吃饭 // apple
0232哔哩哔哩 //banana
0233守望先锋
```

解释：

1. 文件属于 `2025-01`
2. `d0101` 是 `1 月 1 日`
3. `// beta`、`// alpha` 是日备注
4. `0606w` 的 `w` 命中 `wake_keywords`
5. 因为这是当天首个语义活动，所以这一天是 wake-start day
6. 若存在上一天末事件，则系统可生成一条 `sleep_night`
7. `1353睡觉 // remark` 表示活动 token `睡觉`，备注 `remark`
8. 在只有点事件的模式下，`睡觉` 的持续时间来自 `06:06 -> 13:53`

## 12. 最终业务流程

从高层看，最终业务流程是：

1. 读取原始月度 TXT
2. 验证文本格式
3. 根据配置验证活动 token 是否合法
4. 解析为带日语义的 struct
5. 依据 `wake_keywords`、上一天末事件、上月末事件进行跨日/跨月补链
6. 在需要时生成 `sleep_night`
7. 将最终活动事实插入数据库

## 13. 相关文档

更细的专题说明见：

1. [day_bucket_and_wake_anchor_semantics.md](/C:/code/time_tracer/docs/time_tracer/core/ingest/day_bucket_and_wake_anchor_semantics.md)
2. [interval_event_and_mixed_timeline_semantics.md](/C:/code/time_tracer/docs/time_tracer/core/ingest/interval_event_and_mixed_timeline_semantics.md)
3. [record_input_and_day_completeness_semantics.md](/C:/code/time_tracer/docs/time_tracer/core/ingest/record_input_and_day_completeness_semantics.md)
4. [txt_logic.md](/C:/code/time_tracer/docs/time_tracer/core/capabilities/validation/txt_logic.md)
5. [input_format_cn.md](/C:/code/time_tracer/docs/time_tracer/user_manual/input_format_cn.md)

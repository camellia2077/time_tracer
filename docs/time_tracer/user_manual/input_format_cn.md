# 输入文本格式与记录规则

本文描述 TimeTracer 的 `.txt` 原始日志格式、必需约束，以及
`@allow-long` 的使用规则。

关于新区间事件与点事件混用的目标语义，见：

- [../core/ingest/interval_event_and_mixed_timeline_semantics.md](/C:/code/time_tracer/docs/time_tracer/core/ingest/interval_event_and_mixed_timeline_semantics.md)

## 1. 文件结构

- **年份头**: 必须以 `yYYYY` 形式出现，且**一个文件只能有一个年份头**。
  - 示例: `y2021`
- **月份头**: 必须以 `mMM` 形式紧随年份头出现（新建月文件默认写入）。
  - 示例: `m01`
- **日期行**: `MMDD` 四位数字。
  - **首日必须为当月 1 号**（例如 `0101`）。
- **日备注行**: 以 `remark_prefix` 开头（默认 `r `）。
  - 只能出现在日期行之后，且**必须在当天任何事件行之前**。
- **事件行**:
  - 当前实现主路径：`HHMM` + 活动描述（点事件）。
  - 目标语义：同时支持
    - 点事件：`HHMM` + 活动描述
    - 区间事件：`HHMM-HHMM` + 活动描述
  - 时间为 24 小时制，`00:00` ~ `23:59`。
  - 活动描述不能为空。
- **空行**: 会被忽略。

头部顺序约束:
- 必须先出现 `yYYYY`，再出现 `mMM`，之后才允许 `MMDD`/事件行。
- 同一文件最多允许一个 `mMM`。
- `mMM` 必须在 `m01..m12` 范围内。
- 若 `mMM` 与后续 `MMDD` 的月份前两位不一致，结构校验会报错。

## 2. 事件行与行内备注

事件行可以附带行内备注，分隔符支持 `//`、`#`、`;`。
程序会以**最先出现的分隔符**作为备注起点。

示例:
```
0747wake
0806meal
0809oral-hygiene #今天特殊情况
0813drive
0900-1030study-math // chapter 3
```

目标语义下可按如下理解：

- `0747wake`、`0806meal`、`0809oral-hygiene`、`0813drive` 是点事件
- `0900-1030study-math` 是区间事件
- 点事件的开始时间需要由上下文推导
- 区间事件自带显式开始与结束时间

如果两个已记录区间之间存在空白时间，例如：

```text
0900-1030study-math
1401-1900sleep
```

则 `1030-1401` 视为**未记录时间**。

这表示：

- 用户不想记录或暂不记录这段时间
- 系统不自动补成某个未知活动
- 查询与报表只统计已记录区间的总和

## 3. `@allow-long` 标记

为了跳过“单个活动超过 16 小时即报错”的限制，可以在**事件行的行内备注**
里加入 `@allow-long`（顺序不敏感，只要备注里包含该 token 即生效）。

示例:
```
0756oral-hygiene #今天特殊情况 @allow-long
```

**注意**:
- `@allow-long` 只能写在**事件行的行内备注**中；
- 写在**日备注行**（`r ...`）里不会生效。

## 4. 逻辑校验要点

### 4.1 当前实现主路径

- **活动时长必须 > 0**（`start_time == end_time` 视为错误）。
- **单个活动时长不得超过 16 小时**，除非事件备注包含 `@allow-long`。
- **wake 相关活动只能是当天第一条语义活动**。
- **作者态可输入活动名** 采用：
  `authorable_event_tokens = alias_mapping.keys ∪ wake_keywords`。
  也就是说，wake 词本身也属于可直接输入的合法 token。

### 4.2 目标区间语义

引入区间事件后，还应满足以下目标规则：

- **允许未记录时间缺口**，缺口本身不是错误
- **不允许已记录区间重叠**
- **点事件与区间事件混用时**，点事件应从“最后已知时间边界”起算
- **Total Time Recorded** 继续表示“已记录时长之和”，而不是整天跨度
- **区间事件允许跨过 00:00**：如果 `start_time > end_time`，按“结束在下一天”
  解释，再套用单活动时长校验。例如 `2132-0135study` 表示
  `21:32 -> 次日 01:35`；`1030-0900study` 表示 22h30m，默认会因超过
  16 小时被拒绝，除非行内备注包含 `@allow-long`。

## 5. 作者态完整性提醒

- **每天少于 2 条 authored events 不再是硬性错误**。
- 这类情况表示“当天记录暂未补全”，仍然可以保存、继续追加、继续导入。
- `Record Input` 与 `TXT save+sync` 可能提示：
  - `Warning: this day currently has fewer than 2 authored events, so some intervals may not be computable yet.`
  - `Warning: possible overnight continuation; the first event of this day is not wake-related, so no sleep activity will be auto-generated.`
- 第二条 warning 只在“当天不完整且首条 authored event 不是 wake”时出现，并替代第一条泛化提示。

引入区间事件后，这里的口径需要进一步理解为：

- **只有 1 条点事件** 通常表示“合法但未完成”
- **只有 1 条完整区间事件** 也可能已经是“合法且可统计”的记录
- 因此，是否提示 completeness warning，不能再只看 authored event 的数量

## 6. 完整示例

```
y2021
m01
0101
r 今天写代码
0747wake
0806meal
0809oral-hygiene #今天特殊情况 @allow-long
0813book
```

区间事件示例：

```text
y2021
m01
0101
0900-1030study-math
1401-1900sleep
```

这里的 `1030-1401` 是合法未记录时间，不参与统计，也不会被系统自动补成其他活动。


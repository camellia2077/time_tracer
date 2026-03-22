# Day Bucket 与 Wake Anchor 语义说明

本文是 `time_tracer` 当前“日桶 (day bucket)”与“起床锚点 (wake anchor)”语义的权威说明。

它的目标不是介绍所有 ingest 细节，而是专门解释几个最容易被误解的业务规则：

1. 为什么单日 `Total Time Recorded` 可以大于 `24h`
2. `wake_keywords`、`isContinuation`、`getupTime` 三者如何协同
3. `wake_anchor`、自动补出的跨天睡眠活动、任意 `sleep_*` 活动三者有什么区别
4. 为什么 `sleep_project_path` 不参与 day status 判定

## 1. 术语

### 1.1 `wake anchor`

语义：这一天存在有效的“起床锚点”。

当前实现公式：

```text
wake_anchor = !isContinuation && !getupTime.empty() && getupTime != "00:00"
```

说明：

- `wake_anchor` 是**日级元数据 / 状态**
- 它不等于“有 sleep 活动”
- 它也不等于“成功补出 `sleep_night`”

### 1.2 `generated overnight sleep`

语义：系统根据“上一天最后一个活动结束时间 + 当前天起床时间”自动补出的一条跨天睡眠活动。

说明：

- 这是**活动事实补链**
- 它的项目路径由 `generated_activities.sleep_project_path` 控制
- 默认路径是 `sleep_night`
- 它用于活动树、活动列表、时长统计
- 它**不是** `wake_anchor` 的来源

### 1.3 `any sleep activity`

语义：当天活动事实中是否存在任意 `sleep_*` 路径活动，例如：

- `sleep_night`
- `sleep_day`

说明：

- 这是**活动事实层**概念
- 它和 `wake_anchor` 是两个不同维度

## 2. 单日时长口径

### 2.1 `Total Time Recorded`

当前口径：

```text
Total Time Recorded = sum(activity.duration_seconds)
```

不是：

```text
last_activity_end - first_activity_start
```

### 2.2 为什么允许大于 `24h`

`time_tracer` 的“日”不是简单的自然日 `00:00 ~ 24:00` 容器，而是业务上的“当天记录桶”。

在通宵场景下：

- 前一天末尾活动可能延续到当前天
- 当前天内部活动也可能继续跨到次日凌晨
- 这些活动都可以合法归入同一个 day bucket

因此：

- `Total Time Recorded > 24h` **在当前业务设计中是允许的**
- 这不是自动表示 bug

是否合理，应由“活动事实与分桶规则”判断，而不是由 `24h` 这个物理阈值直接判错。

## 3. `wake_keywords` 与 `isContinuation`

### 3.1 `wake_keywords` 的职责

`wake_keywords` 只负责一件事：

> 识别“这一天的第一个语义活动是否是起床锚点”

它不负责：

- 指定自动生成睡眠活动的路径
- 判断 `sleep_night` 应不应该叫什么
- 直接决定睡眠时长统计

### 3.2 首个活动规则

当前规则：

- 只有当天**第一个语义活动**是 `wake_keywords` 之一时，才建立 `getupTime`
- 如果 `wake_keywords` 出现在当天后续活动位置，`validate logic` 直接失败

这样做的原因：

- 避免后续 wake 事件反转整天语义
- 保证 `wake_anchor` 语义稳定
- 保证自动补链与 day status 的判断口径一致

### 3.3 `isContinuation`

如果当天第一个语义活动不是 wake，并且此时还没有 `getupTime`，则：

```text
isContinuation = true
```

它表示：

> 这一天是上一天活动/睡眠片段的延续，而不是以正常起床锚点开始的一天

## 4. `sleep_project_path` 的职责边界

配置：

```toml
[generated_activities]
sleep_project_path = "sleep_night"
```

它只决定：

- 自动补出来的跨天睡眠活动使用什么项目路径

它不决定：

- `wake_anchor` 是否为真
- 报表头部 `Wake Anchor`
- `days.wake_anchor`
- `Wake Anchor Days (True)`

因此，即使未来把：

```toml
sleep_project_path = "sleep_overnight"
```

只要 `wake_keywords` 与 `getupTime` 规则不变：

- `wake_anchor` 语义就不应该发生变化

## 5. 数据真相源

### 5.1 `days.wake_anchor`

真相源：

- `getupTime`
- `isContinuation`

这是入库时写入的日元数据。

### 5.2 报表头部 `Wake Anchor`

真相源：

- `days.wake_anchor`

### 5.3 `Wake Anchor Days (True)`

真相源：

- 范围内 `days.wake_anchor = 1` 的日期数

### 5.4 `Sleep Time`

真相源：

- `time_records` / 活动事实中的 `sleep_night`、`sleep_day` 等 `sleep_*` 活动

注意：

- `Wake Anchor` 与 `Sleep Time` 不是同一个概念

## 6. 当前推荐理解方式

可以把这三个概念记成：

1. `wake_anchor`
   - “这一天有没有有效起床锚点”
2. `generated overnight sleep`
   - “系统有没有自动补出跨天睡眠活动”
3. `any sleep activity`
   - “活动事实里有没有任意 `sleep_*` 片段”

它们彼此相关，但不等价。

最重要的一条是：

> `Wake Anchor` 是 day status；`sleep_night` 是 activity。


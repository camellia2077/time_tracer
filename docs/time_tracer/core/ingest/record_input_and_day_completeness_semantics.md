# Record Input 与 Day Completeness 语义说明

本文是 `time_tracer` 当前关于“单日首条记录”“日记录完整度（day completeness）”“跨日补链”和“阻断性校验边界”的权威业务说明。

它的目标不是解释所有实现细节，而是明确以下最容易混淆的业务规则：

1. 新一天是否允许只写入第一个活动
2. “记录不完整”与“记录非法”有什么区别
3. `wake_keywords`、`isContinuation`、上一天末事件三者如何协同
4. 自动补出的 `sleep_night` 在什么条件下才应该生成
5. 哪些情况应该是 `warning`，哪些情况才应该是 `error`
6. 作者态“可输入活动名”与 wake 语义集合如何分工

若当前实现与本文规则不一致，应将差异视为实现待调整项，而不是反向修改本文语义。

## 1. 术语

### 1.1 首个语义活动

语义：某一天在忽略空行、日期行、日备注行之后，遇到的第一条事件行所代表的活动。

示例：

```text
0102
r note
0656w
0904无氧训练
```

这里首个语义活动是 `w`。

### 1.2 day completeness

语义：某一天的记录是否已经足够完整，足以推导出该天内部的大部分区间。

本文约定：

1. `0` 条活动：空白日
2. `1` 条活动：不完整日，但仍然是合法输入
3. `>=2` 条活动：已具备基本区间计算条件

`day completeness` 只描述“信息是否足够完整”，不直接等价于“能否保存”或“能否入库”。

### 1.3 wake-start day

语义：这一天的首个语义活动命中 `wake_keywords`，因此这一天以“起床锚点”开始。

### 1.4 continuation day

语义：这一天的首个语义活动不是 `wake_keywords`，因此这一天被视为上一段跨日活动的延续。

这表示：

1. 该天不是以 wake anchor 开始
2. 该天首段时长如需计算，应优先依赖“上一天最后一个事件时间”
3. 这一天不应自动生成 `sleep_night`

### 1.5 previous context

语义：当前天之前可用的上一条上下文事件，通常是“上一天最后一个事件时间”。

它可以来自：

1. 同一月份文本中的前一天
2. 上一个月份文本的最后一天
3. 外部补入的上月末事件

### 1.6 authorable event token

语义：作者态允许直接写入 TXT 事件行的原始 token。

最终口径：

1. `alias_mapping.keys` 是普通活动映射集合
2. `wake_keywords` 是 wake 语义集合
3. `authorable_event_tokens = alias_mapping.keys ∪ wake_keywords`

这表示：

1. wake token 可以是作者态合法输入，即使它不参与普通活动映射
2. Android/authoring 不应再维护任何本地硬编码 wake 名单
3. 作者态校验必须与 core 暴露出来的配置查询结果保持一致

## 2. 核心业务结论

最重要的规则是：

> 新一天必须允许先写入第一个活动；“当天少于 2 条活动”只能表示记录暂不完整，不能单独作为阻止记录或阻止导入的依据。

换句话说：

1. “只有 1 条活动”是合法但未完成
2. “格式错误 / 活动名非法 / wake 出现在当天非首位 / 时长不可能成立”才是不合法

## 3. 阶段归属

为避免把 `wake` 语义、合法性校验、自动补睡眠三件事混为一谈，本文约定以下阶段职责：

### 3.1 文本解析阶段

文本解析阶段的职责是：

1. 识别当天首个语义活动
2. 若首个语义活动命中 `wake_keywords`，建立 `getupTime`
3. 若首个语义活动不命中 `wake_keywords`，建立 `isContinuation`

文本解析阶段不负责：

1. 因为当天后续再次出现 wake keyword 而立即判错
2. 生成 `sleep_night`

换句话说，解析阶段只负责建立“这一天是 wake-start day 还是 continuation day”的日级语义。

### 3.2 逻辑校验阶段

逻辑校验阶段的职责是：

1. 检查 wake keyword 是否只出现在当天第一个语义活动位置
2. 将“wake 出现在当天后续位置”判定为非法输入

因此：

1. “wake 只能是当天第一个活动”是 ingest 的业务要求
2. 这条要求的正式判错点应归属于逻辑校验阶段
3. 它不应被误解为“parser 读到第二个 wake 时必须立刻抛异常”

### 3.3 转换与补链阶段

转换与补链阶段的职责是：

1. 根据解析阶段已经建立的 `getupTime / isContinuation` 语义进行区间换算
2. 在满足条件时把自动生成的 `sleep_night` 插入到解析后的 struct 中

因此：

1. 自动补出的 `sleep_night` 是 activity fact
2. 它发生在解析后的 struct 层，而不是原始文本层
3. 它依赖前两个阶段已经建立并验证过的 wake 语义

## 4. Record Input 的业务语义

### 3.1 新一天必须允许先有第一条记录

`Record Input` 的职责是按当前时间快速追加事件，而不是要求用户一次性补齐完整的一天。

因此：

1. 当目标日当前还没有任何活动时，系统必须允许写入第一条活动
2. 不能因为“此时还只有 1 条活动”就把这次记录判成失败
3. 否则会产生业务死锁：
   1. 新的一天天然从 `0` 条活动开始
   2. 若没有第一条活动就无法变成第二条活动
   3. 那么用户永远无法开始当天记录

### 3.2 Record Input 不是完整性担保器

`Record Input` 只保证：

1. 单行活动名合法
2. 写入时间合法
3. 当前追加不会破坏同一天内部的时间顺序约束

它不应该要求：

1. 当天已经存在至少两条活动
2. 上下文睡眠已经可计算
3. 所有跨日区间在当前时刻都已闭合

## 5. 单日首条活动的业务解释

### 4.1 首条活动是 wake keyword

如果当天首个语义活动命中 `wake_keywords`：

1. 这一天是 `wake-start day`
2. 该时间点定义当天的 `getupTime`
3. 如果存在 `previous context`，系统可以根据“上一天最后一个事件时间 -> 当前 wake 时间”自动补出 `sleep_night`
4. 如果不存在 `previous context`，则：
   1. 这一天仍然是合法的 wake-start day
   2. 但不能凭空生成 `sleep_night`
   3. 只能认为“自动补睡眠所需上下文暂时缺失”

### 4.2 首条活动不是 wake keyword

如果当天首个语义活动不命中 `wake_keywords`：

1. 这一天是 `continuation day`
2. 它表示当天开始时仍处于上一段跨日活动的延续中
3. 如果存在 `previous context`，则当天首段时长应使用“上一天最后一个事件时间 -> 当前首条事件时间”来计算，并归属于当前首条非 wake 活动
4. 如果不存在 `previous context`，则：
   1. 该天仍然是合法输入
   2. 但当天最前面的跨日片段暂时无法计算
   3. 系统不应伪造 `sleep_night`
   4. 系统也不应仅因为缺少这段上下文而阻止记录或阻止导入

## 6. `sleep_night` 的生成边界

`sleep_night` 是“自动补出的跨日睡眠活动事实”，不是所有跨日首段的默认兜底名称。

### 5.1 允许生成 `sleep_night` 的条件

只有在以下条件同时成立时，才允许补出 `sleep_night`：

1. 当天首个语义活动是 `wake_keywords` 之一
2. 当前天具备有效的 `getupTime`
3. 能拿到有效的 `previous context`

### 5.2 不允许生成 `sleep_night` 的条件

以下情况都不应生成 `sleep_night`：

1. 当天首个语义活动不是 wake keyword
2. 只有“当天活动数不足 2 条”这一条信息
3. 缺少上一天末事件，无法形成可靠跨日时间差

## 7. day completeness 与合法性的关系

### 6.1 合法但不完整

以下情况属于“合法但不完整”：

1. 新一天目前只有 1 条活动
2. 当天首条活动为 wake，但缺少上一天上下文，暂时无法补 `sleep_night`
3. 当天首条活动非 wake，且缺少上一天上下文，导致最前面的跨日片段暂不可解

这些情况都可以：

1. 保存为 TXT
2. 继续后续追加记录
3. 进入后续的补链或重新导入流程

### 6.2 非法输入

以下情况才属于应阻断的非法输入：

1. 文本头部格式不合法
2. 行格式不合法
3. 活动名不在配置允许范围内
4. `wake_keywords` 出现在当天非首个语义活动位置
5. 活动时长必然为零或形成明确的非法时间关系

## 8. 诊断等级约定

### 7.1 应为 `warning` 的情况

以下情况本身只表示“作者态信息暂未补全”，不再构成 core ingest /
import 的阻断条件：

1. 当天少于 2 条活动
2. 当前天缺少上一天上下文，导致首段无法完整推导
3. 当前时刻还不能生成 `sleep_night`

最终实现口径是：

1. core validate / ingest / processed JSON import 不再因为 `<2 authored events`
   失败
2. completeness warning 只保留在作者态入口：
   1. `Record Input`
   2. `TXT save+sync`
3. authored events 的计数只看 TXT 中用户写下的事件行：
   1. 不含日备注行
   2. 不含自动生成的 `sleep_night`

这些 `warning` 的作用是提醒“统计暂不完整”，而不是阻止用户继续记录。

### 7.2 应为 `error` 的情况

以下情况应报告为 `error`，并可阻断后续流程：

1. 文本结构不成立
2. 配置/别名映射不成立
3. wake 语义位置错误
4. 明确的零时长、负逻辑或不可能时间关系

## 9. 对 Android Record Input 的直接约束

`Record` 页中的 `Record Input` 与 `TXT save+sync` 必须遵守以下业务要求：

1. 当目标日为空日时，允许直接写入第一条活动
2. 若该天在作者态保存后少于 2 条 authored events，可以提示：
   `Warning: this day currently has fewer than 2 authored events, so some intervals may not be computable yet.`
3. 若该天不完整，且首条 authored event 不是 wake-related，则应改为提示：
   `Warning: possible overnight continuation; the first event of this day is not wake-related, so no sleep activity will be auto-generated.`
4. 若命中第 3 条，则只显示通宵 warning，不再叠加泛化 warning
5. 录入/保存失败只应由真正的非法输入触发，而不应由“信息暂时不完整”触发

## 10. 作者态 token 与 wake 语义边界

为消除“alias key”和“wake keyword”之间的歧义，当前实现约定：

1. `listActivityAliasKeys()` 继续只表示 `alias_mapping.toml` 左键
2. `listWakeKeywords()` 只表示 `interval_processor_config.toml` 中的 `wake_keywords`
3. `listAuthorableEventTokens()` 表示 `alias_mapping.keys ∪ wake_keywords`
4. `Record Input`、`Quick Access`、core atomic record 活动名校验统一使用第 3 条口径

这三类集合职责不同：

1. alias keys 解决普通活动映射问题
2. wake keywords 解决“当天首条 wake 语义”问题
3. authorable tokens 解决“用户此刻能不能输入这个原词”问题

## 11. 推荐理解方式

可以把相关概念分成三层：

1. 输入是否合法
   - 能不能写进去、能不能继续保留
2. 当天是否完整
   - 统计和自动补链是否已经拥有足够信息
3. 跨日活动能否推导
   - 是否存在 `previous context`

三者关系是：

1. 合法输入，不等于已经完整
2. 不完整，不等于非法
3. 缺少上下文，不等于必须拒绝记录

最重要的一条仍然是：

> “当天少于 2 条活动”描述的是 completeness，不是 validity。

# 文本解析核心算法 (Text Parsing Algorithm)

本文档面向开发者，揭示 Time Tracer 是如何读取遵循 `input_format_cn.md` 规范的纯文本日志，并在内存中把它们还原为强类型领域模型（`DailyLog`）的。

权威代码入口主要位于 Application 层的 `apps/time_tracer/src/application/parser/text_parser.cpp`。

---

## 1. 核心解析思想：基于流的状态机 (Stream-based State Machine)

由于原始日志文件采用逐行组织格式，`TextParser` 采用单次遍历、逐行读取的状态机模式，旨在保证解析效率。

### 1.1 上下文状态保持 (Context Management)
由于文本内容存在隐含的层级关系（如年份、月份对日记录的制约），解析器需要维护当前处理的上下文状态。

解析器 (`TextParser::Parse`) 在内部维护以下状态：
- `current_year_prefix`: 当前所属年份（例如 "2024"），由读取到 `y2024` 触发设置。
- `current_month_prefix`: 当前所属月份（例如 "01"），由读取到 `m01` 触发设置。
- `current_day`: 一个缓冲区对象 (`DailyLog`)，用于暂存当天收集到的所有事件和备注。

### 1.2 触发与提交流程
当解析器逐行 (`std::getline`) 推进时：
1. **遇到头部标记 (`yYYYY` 或 `mMM`)**: 更新对应的上下文状态。如果遇到月份标记但没有年份，会发出报告。
2. **遇到日期标记 (`MMDD`)**: 
   - 识别到新日期。
   - 解析器会检查缓冲区 (`current_day`) 中是否存在待处理的数据。若存在，则触发回调 `on_new_day(current_day)`，将数据提交至领域逻辑层。
   - 清空缓冲区，利用当前上下文（年、月、日）组装 ISO 格式日期符（如 `2024-01-01`）初始化新的 `current_day`，开始新一轮收集。
3. **遇到普通内容行**: 代理给 `ParseLine` 方法，进行事件拆解或备注归档。
4. **文件 EOF (结尾)**: 主循环结束后，若缓冲区中仍有剩余数据，执行最后一次 `on_new_day` 提交。

## 2. 单行事件拆解逻辑 (`ParseLine`)

当在某一天内读取到一行普通的文本时，`ParseLine` 是如何做到精准切分的？

### 2.1 日备注识别
如果行以配置文件中定义的 `remark_prefix`（通常是 `r `）开头，这行内容会被裁切掉前缀，然后原封不动地推入 `current_day.generalRemarks` 数组。它严格不参与时间解析。

### 2.2 事件切分 (固定宽度提取)
`TextParser` 利用了我们日志规范中“时间头固定 4 位数字”的强假设，直接使用高效的硬编码索引截取，而非正则。
- 前 4 个字符 `line.substr(0, 4)` 强制转换为小时与分钟。
- 后续的字符串则丢入备注解析器。

### 2.3 `Wake` 与 `Continuation` 的上下文试探
为了兼容以前的日志生态，解析器在获得事件描述后，会调用 `ProcessEventContext`：
- **匹配起床关键词**: 如果此事件就是 Config 中定义的 `wake_keywords`，那么它会顺手把这四个数字提取出来作为 `current_day.getupTime`。
- **匹配跨日延续事件**: 如果一天内第一条事件还没有遇到起床标识（`getupTime` 为空，且 `rawEvents` 为空），算法会推断这条日志属于**昨晚跨天持续至今的延续日志（Continuation）**，并将 `isContinuation` 标志置为 `true`。

### 2.4 行内备注剥离 (`ExtractRemark`)
解析器会通过 `ExtractRemark` 方法扫描这行文本，寻找由 `kRemarkDelimiters` (`//`, `#`, `;`) 指定的分隔符：
- 如果找到对应分隔符，则把前面的部分 `Trim` 清除首尾空格后设为**标准活动描述 (Description)**。
- 把后面的部分设为**补充备注 (Remark)**。
- 这不仅使得底层结构清爽，也是上层的 `@allow-long` 标记得以能够正确匹配的基石。

## 3. 异常处理与验证规则

`TextParser` 严格执行格式规范，通过 `ThrowParseError` 处理不符合规范的数据，防止无效数据进入核心逻辑：
- 在未识别到年份标记时识别到月份标记。
- 在缺失月份上下文时出现日期行。
- 时间格式及范围校验（如 `HH > 23`，`MM > 59`）。

这种快速失败（Fail-fast）机制确保了传递给 `DayProcessor`（Domain层）的 `DailyLog` 对象符合数据结构与一致性要求。

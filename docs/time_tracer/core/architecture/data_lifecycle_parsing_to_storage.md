# 数据流转生命周期：从解析到入库 (Data Lifecycle)

本文旨在通过宏观视角，结合具体的输入示例，为您清晰展现 Time Tracer 系统中最核心的“数据流转生命周期”。

当用户输入一个 `.txt` 文本时，系统是如何将其一步步解析为内存领域对象，又是如何最终落盘到 SQLite 数据库中并支撑上层查询统计的？本文将分三步为您揭秘。

---

## 1. 核心架构总览 (Architecture Overview)

Time Tracer 采用了清晰的分层架构（Clean Architecture）来处理复杂的日志解析与导入流程。整个生命周期可以抽象为三个阶段：

### 阶段一：数据解析与领域转换 (Parse & Convert)
**所在层级**：`application/parser` (解析网关) -> `domain` (核心业务规则)
**核心职责**：将无规则的纯文本字符串提取为系统认识的领域模型（Domain Models）。这一步也是整个引擎最关键的阶段，负责处理时间计算、跨天通宵防断链、活动别名映射以及起床判定。
**主要参与者**：
- `TextParser`: 从文本中提取时间戳，剥离出 `description` 和 `remark`，生成原始事件 `RawEvent`。
- `ConverterService` / `DayProcessor`: 执行“滑动窗口”算法，结合前一天的记录来补齐今日的跨夜睡眠；将描述映射为内部标准的 `project_path`（如 `study_english`）。

### 阶段二：数据入库与持久化 (Ingest & Storage)
**所在层级**：`application/importer` (导入代理) -> `infrastructure/persistence` (底层存储)
**核心职责**：将计算和统计完毕、不沾染任何 IO 的纯粹内存对象池（`ParsedData`），转化为关系型数据库（SQLite）中的实体行数据，确保 ACID 安全。
**主要参与者**：
- `MemoryParser`: 作为 Domain 和 Importer 的屏障，剥离业务领域对象，组装成专供入库的结构体 `DayData` 和 `TimeRecordInternal`。
- `importer` (内部组件 `ProjectResolver` & `Writer`): 递归解析项目树路径，创建不存在的 `projects` 节点，并将记录行安全地插入至 `days` 和 `time_records` 数据表中。

### 阶段三：数据查询与聚合统计 (Query & Stats)
**所在层级**：`application/reports` & `infrastructure` -> UI/CLI
**核心职责**：供系统调取历史沉淀的数据进行汇总。不论是日报告、月报告还是终端的可视化树（tree），均从 SQLite 取出结构化结果，而非重新读取文档。

---

## 2. 核心流转实录：以具体数据为例

为了更好地展示架构中各组件是如何协作的，我们拿一段典型的一连串时间日志来进行拆解：

```text
y2024
m01

0101
0636起床
1101zhihu //remark
2152守望先锋 #备注
2240有氧训练 ;apple
0158bilibili //banana
0313Clash Royale
0319overwatch #cherry
0328抖音 ;date

0102
r alpha
r 没有牺牲大到无法接受
r 没有背叛小到可以容忍
0633起床
1108高等数学 //remark
1214bilibili #备注
1752线性代数矩阵 ;apple
0101线性代数矩阵
0133吃饭 //banana
0314数据结构 #cherry
0322cr ;date
```

### 步骤一：数据解析与领域转换 (Parse -> Domain)

当这串文本进入 `TextParser`，引擎首先依靠 `y2024`、`m01` 和 `0101`/`0102` 的年、月、日锚点划定时间边界。

1. **原始事件提取**：
   在解析 `0101` 日志时，遇到 `1101zhihu //remark` 行，`TextParser` 会将该行拆解为：
   - 原始时间：`1101`
   - 描述：`zhihu`
   - 备注提取拦截：利用 `//`，将 `remark` 剥离出来挂载为专属字段。
   从而生成一个 `RawEvent` 对象放入当天的流水线。

2. **核心业务挂载与转换 (`DayProcessor`)**：
   - **时间差计算**：活动 `zhihu` 的持续时间是怎样计算的？`11:01` 到了下一个活动 `21:52` 开始，相减得出该段持续了 651 分钟。
   - **映射改写**：`zhihu`、`bilibili`、`抖音` 基于用户的配置文件被统一归入 `recreation` (娱乐) 下子树。`守望先锋`和`overwatch` 被同化合并映射为 `recreation_gaming_overwatch`。
   - **精妙的跨天“通宵”与防断处理**：
     观察 `0101` 日志，最后记录到了凌晨 `0328抖音 ;date`，**并没有出现代表睡觉的结束动作**。
     当 `0102` 开始，系统捕捉到了 `0633起床` 关键字点。
     此时，引擎内部启动了“滑动窗口”的衔接补偿技术：它知道 `0101` 的最后动作 `03:28` 结束，而 `0102` 是 `06:33` 起床。于是它自动在 `0102` 的头部追加填充了一个 `03:28 - 06:33` 的连续 `sleep`（睡眠）模块片段！这也叫做**同月跨天动态修复**技术。

3. **统计算法发力 (`DayStats`)**：
   它会在 `0102` 里看到 `高等数学`、`线性代数矩阵` 和 `数据结构`，基于规则知道这些都是 `study` 节点，于是立刻累加它们的持续时间放入当天的 `DayStats`（比如 `study_time` 为 10.5 小时），同理核算 `recreation_time` 等等。

### 步骤二：数据入库与持久化 (Domain -> Importer -> SQLite)

完成了前置的高运算耗时后，沉淀出了一批 `std::vector<DailyLog>` 纯内存对象，它们即将交由毫无 IO 包袱的 `importer` 落盘。

1. **洗涤与结构化隔离**：
   `MemoryParser` 取代了早年的临时中转文件，直接在内存池中将 `DailyLog` 降维转化为符合数据库表结构的 `DayData`（以天为尺度的总表快照）和 `TimeRecordInternal`（扁平化明细）。
   - 诸如 `0102` 头部记录的每日评价 `r alpha` 及格言，被直接拍扁成了 `DayData` 结构中的 `remark` 字段（用换行符连接）。

2. **层级路径建树与关系落库 (`ProjectResolver` & `Writer`)**：
   - `0102` 日志里的“1108高等数学”原本在配置文件里被映射为 `study_math_advanced` 路径链。
   - 注入库时，`importer` 首先核对一张叫 `projects` 的表。如果 `study` 这棵树或者其子节点 `math` 不存在，它会从根节点开始依次自上而下安全插入（依靠 SQLite 返回的主键 `ID` 维持父子关系血脉）。
   - 最终，把包含有精确起止时间戳、计算完毕的时长（`duration`）、备注（`remark` 为 `//remark`），以及项目映射引用（外键指向刚才建好的 `study_math_advanced` ID）的一条完整物理数据行写入 `time_records` 宽表面中。

### 步骤三：统计与查询服务 (Database -> App)

最终，数据安稳沉淀在了关系型网架内。

当我们在 CLI 执行类似 `query day 20240102 --day-remark` 的操作时：
- 系统**再也不必从头去读取那份沉重的 txt 文稿**，也不用再执行耗费 CPU 的“计算事件间隔”、“比对跨天修正点”等庞大工序。
- 它只需求助于底层的 `time_records` 和 `days` 表面，利用高效率的 SQL JOIN 联合 `projects` 词库，即可一秒输出 `0102` 这一天的“学习总时长：X 小时”、显示出格言，乃至完美打印具有缩进深度的活动树形图表。

---

## 3. 结语

这一严苛规整的“**解析隔离 -> 内存深计算 -> 无磁盘映射入库 -> 高效读取输出**”生命周期链条，构成了 Time Tracer 的绝对核心，也是在确保数据不随年代积压拉低系统响应能力的最强底座后盾。

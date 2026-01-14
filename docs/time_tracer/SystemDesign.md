# 时间日志系统核心设计与业务逻辑 (System Design & Core Logic)

**版本**: 0.3.25
**更新日期**: 2026-01-01

## 1. 设计哲学 (Design Philosophy)

本系统的核心目标是实现“个人量化生活（Quantified Self）”的高效闭环。

> **Trace your time, log your life.**

设计遵循以下三大原则：

1. **输入极简主义 (Minimalist Input)**:
* 用户的时间是宝贵的。记录日志的过程要简化，不能像latex那样使用符号记录。
* 系统应当承担“补全上下文”的职责。用户只需记录“变化点”，系统自动推导“时间流”。


2. **语义层级分离 (Semantic Separation)**:
* **原始输入**面向人类记忆（如使用简写 `gym`）。
* **存储数据**面向机器统计（如标准化路径 `exercise.anaerobic`）。
* 系统必须内置一个强大的“翻译层”来桥接这两者。


3. **数据仓库化 (Data Warehousing Approach)**:
* 数据库不仅仅是存储原始日志，更应像数据仓库一样，存储“预计算”好的多维指标（如当日总睡眠、总学习时长），以便于下游 BI 工具直接快速查询，而无需并在运行时重新计算。



---

## 2. 预处理子系统：从文本到语义 (The Reprocessing Engine)

该模块负责将混乱、非结构化的文本流转化为逻辑严密的时间轴数据。

### 2.1 上下文状态流机制 (Contextual State Flow)

为了支持极简输入（即用户不需要每行都写年份和日期），解析引擎被设计为一个**状态保持机**。

* **年份锚点 (Year Anchor)**: 当读取到年份标记（如 `y2025`）时，系统锁定全局上下文年份。后续所有日期均归属该年份，直到遇到新年份标记。
* **日期锚点 (Date Anchor)**: 当读取到日期标记（如 `0101`）时，系统开启一个新的“时间桶（Time Bucket）”。
* **归属原则**: 所有在该日期锚点之后、下一个日期锚点之前的事件，自动继承该日期属性。

### 2.2 跨天时间连续性与睡眠推导 (Time Continuity & Sleep Derivation)

这是本系统最复杂的业务逻辑之一。系统认为**睡眠不是一个主动记录的活动，而是两个清醒周期之间的时间真空**。

#### 逻辑模型

1. **触发条件**: 系统识别到当日的 `Wake Event`（通过配置的起床关键词，如 "起床"）。
2. **回溯链接**: 系统回溯**上一日 (Previous Day)** 的最后一条活动记录。
3. **合成计算**:
* 如果存在上一日数据，且时间逻辑合理（上一日结束时间 < 今日起床时间），系统自动生成一个虚拟的 `sleep_night` 活动。
* **时长 = (今日.起床时间) - (上一日.结束时间)**。
* 如果涉及跨越午夜（24:00），算法需自动处理时间进位（+24小时）。



> **设计意义**: 这种设计防止了用户忘记记录睡眠，同时强制验证了时间轴的连续性。如果上一日数据缺失，今日就无法计算睡眠，从而暴露出数据断层。

### 2.3 三级语义映射漏斗 (The Semantic Mapping Funnel)

为了将随意的用户输入转化为标准的统计数据，活动描述经过三层处理：

1. **标准化映射 (Alias Normalization)**:
* 解决书写随意性。
* *例*: `workout`, `gym`, `举铁`  统一映射为 `exercise`。


2. **动态时长修正 (Duration-based Reclassification)**:
* 解决语义歧义。某些活动通过时长来定义性质。
* *例*: 输入 `reading`。
* 若时长 < 15分钟  系统重判为 `browsing` (随便翻翻)。
* 若时长  15分钟  保持为 `reading` (深度阅读)。




3. **层级路径构建 (Hierarchical Path Construction)**:
* 构建分类树。
* *逻辑*: 将扁平的字符串（如 `study_cpp`）解析为路径 `study` -> `coding` -> `cpp`。这为数据库中的树状索引打下基础。



---

## 3. 存储子系统：关系型建模 (The Storage Model)

该模块负责将内存对象持久化到 SQLite 数据库。设计上采用了**维度建模 (Dimensional Modeling)** 的思想。

### 3.1 实体关系图概念 (Conceptual ER Diagram)

系统并非简单地存储一张大表，而是拆分为三个核心实体，以支持灵活的统计分析。

```mermaid
erDiagram
    DAYS ||--o{ TIME_RECORDS : contains
    PROJECTS ||--o{ TIME_RECORDS : classifies
    PROJECTS ||--o{ PROJECTS : parent_of

    DAYS {
        string date PK "主键：YYYYMMDD"
        int sleep_time "预聚合指标"
        int study_time "预聚合指标"
        string remark "当日总结"
    }

    PROJECTS {
        int id PK
        string name "项目名称"
        int parent_id FK "指向父项目 (邻接表)"
    }

    TIME_RECORDS {
        int logical_id PK "逻辑主键"
        timestamp start_ts
        timestamp end_ts
        int duration
        int project_id FK "指向具体项目"
        string date FK "指向归属天"
    }

```

### 3.2 动态分类树 (Dynamic Taxonomy Generation)

系统不要求用户预先在数据库中配置“项目分类表”。分类树是在数据通过时**动态生长**的。

* **机制**: 当入库引擎遇到一个路径 `study_cs_algo` 时：
1. 检查 `study` 节点是否存在？不存在则创建。
2. 检查 `study` 下是否有子节点 `cs`？不存在则创建，并挂载到 `study` 下。
3. 检查 `cs` 下是否有子节点 `algo`？不存在则创建，并挂载到 `cs` 下。
4. 最终，将时间记录关联到 `algo` 的 ID 上。


* **优势**: 用户的分类体系可以随时演进，无需手动维护数据库字典。

### 3.3 预聚合策略 (Pre-aggregation Strategy)

为了优化查询性能（例如：快速绘制“过去一年的睡眠趋势图”），系统在写入 `DAYS` 表时，不仅仅写入元数据，还写入了**聚合后的统计值**。

* **事实表 (Time Records)**: 存储原子级数据（几点几分做了什么）。
* **汇总表 (Days)**: 存储汇总级数据（当天睡了多久，玩了多久）。
* **写入逻辑**: 解析器在内存中完成一天的处理后，会遍历当天所有活动，根据 `StatsRules` 累加出关键指标（Study Time, Sleep Time, etc.），并在插入 `DAYS` 表时直接写入这些数值。
* **读写分离思想**: 虽然写入变慢了（需要计算），但读取（生成报表）变得极快，因为不需要每次都 `SUM(duration) WHERE date=...`。

---

## 4. 核心校验规则 (Core Validation Rules)

为了保证进入数据库的数据是“干净”的，系统在两个阶段实施了严格的校验。

### 4.1 结构完整性 (Structural Integrity)

* **文件头**: 必须以年份标记开始。
* **块序**: 日期必须包含在年份块内，事件必须包含在日期块内。
* **备注位置**: 当日的 `Remark` 必须位于所有具体活动之前（作为元数据存在）。

### 4.2 逻辑一致性 (Logical Consistency)

* **时间流单调递增**: 同一天内的事件时间戳必须严格递增。不允许“时光倒流”（如 10:00 的活动后紧接着 09:00 的活动）。
* **日期连续性 (Date Continuity)**:
* **连续模式**: 检查从当月1号到当前最新日期的连续性，中间不允许断天。
* **完整模式**: 检查整月数据是否完整（如1月必须有31天数据），用于月末归档检查。


* **最小活动集**: 一天至少包含2个时间点（否则无法计算任何时长）。

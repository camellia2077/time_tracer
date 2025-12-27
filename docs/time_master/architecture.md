# 架构与模块 (Architecture & Modules)

## 1. 系统概览与数据流 (System Overview & Data Flow)

本系统的核心工作流由两个单向流动的阶段组成：**数据入库 (Data Ingestion)** 与 **报告生成 (Report Generation)**。

### 1.1 第一阶段：数据入库 (Ingestion Phase)

此阶段负责将用户用txt记录的的非结构化内容，转换为结构化数据(json)，并持久化存储到关系型数据库中。

流程：`txt` -> `json` -> `Structs` -> `SQLite`

* **原始解析 (Raw Processing)**:
* **输入**: 用户编写的原始时间日志文本文件 (`.txt`)。
* **处理**: 系统按行读取文本，应用解析规则识别时间戳、活动名称及备注。在此过程中，系统会自动处理业务逻辑（如跨天睡眠的分割、活动时长的计算），并将每日数据标准化。
* **中间产物**: 生成包含完整元数据和统计信息的标准化 **JSON** 对象。


* **持久化 (Persistence)**:
* **处理**: 系统反序列化中间层的 JSON 数据，将其映射为关系模型。
* **存储**: 将结构化数据（日期、项目关系、时间记录）写入 **SQLite** 数据库。项目层级关系在此处以“邻接表”形式存储（父子节点 ID 关联）。



### 1.2 第二阶段：报告生成 (Reporting Phase)

此阶段负责从数据库中提取数据，重建业务模型，并渲染为用户所需的最终格式。

流程：`SQLite` -> `Flat Records` -> `ProjectTree (Memory)` -> `Formatted String(md tex typ)`

* **数据查询与重建 (Query & Reconstruction)**:
* **输入**: 用户的查询请求（如“生成某月月报”）。
* **处理**: 系统根据时间范围查询 SQLite 数据库，获取扁平化的记录列表。随后，系统在内存中将这些零散的记录**动态聚合**，重建为一棵带有统计数据的 **项目树 (Project Tree)**。这棵树在逻辑上等同于一个多层嵌套的 JSON 对象。


* **渲染输出 (Rendering)**:
* **处理**: 渲染引擎遍历内存中的项目树。
* **策略应用**: 根据用户指定的格式（Markdown, Typst, LaTeX），系统加载对应的**格式化策略**。在遍历树节点的同时，策略模块将节点数据转换为目标格式的字符串。
* **输出**: 生成最终的可视化报告文本。



## 1 预处理模块(reprocessing)
这个模块的时间计时由action_handler来实现
## 2. 报表模块 (Reports)

### 2.1 基础设施层
* **配置加载 (ConfigUtils)**
    * **路径**: `reports/shared/utils/config/ConfigUtils`
    * **职责**: 基于 `nlohmann/json` 封装，负责从磁盘读取并解析 JSON 配置文件，为格式化器提供参数。

### 2.2 核心业务层
* **渲染引擎 (ProjectTreeFormatter)**
    * **路径**: `src/reports/shared/formatters/base/ProjectTreeFormatter.cpp`
    * **职责**: 负责将内存中抽象的 `ProjectTree` 数据结构转换为最终的格式化文本。
    * **机制**: 采用策略模式（Strategy Pattern），解耦了“树的遍历逻辑”与“具体格式（Markdown/Typst/LaTeX）的生成逻辑”。
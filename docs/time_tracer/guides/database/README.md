# Database & Algorithm Guides (数据库与核心算法指南)

本目录包含了 Time Tracer 引擎中最底层、也是极具技术含量的模块：**数据库模型**、**SQL 查询构建引擎**以及**数据结构与构树算法**。

如果您正准备阅读或修改 Time Tracer 从持久层（SQLite）中抽取数据并呈现在界面上（如 Tree 视图）的相关代码，**强烈建议您按照以下顺序阅读本目录下的文档**。

---

## 推荐阅读顺序

### 阶段一：存储基座 (Storage & Schema)
在了解怎么“取”数据之前，必须先了解数据在磁盘和内存中长什么样。

1. **[数据库物理表定义 (Schema)](database_schema.md)**
   - **必读**。这里记录了最核心的 SQLite 数据表字典：`days`、`time_records`、`projects` 以及它们之间的外键联系。
2. **[内存数据结构映射 (Data Structures)](storage/data_structures.md)**
   - 了解物理表中的列，是如何被一对一映射为 C++ 结构体的（如 `TimeRecordInternal`），以及为查询期准备的数据实体长什么样。

### 第一部分：核心数据提取 (Data Query Pipeline)
**核心职责：** **“只管从 SQLite 高效、安全地解析与提取数据”**
这套流水线完全不知道什么是“报告”、“画树”或“界面”。它只负责把带 `--filter` 和日期的时间范围命令，转化为底层的拉取操作，并吐出拍扁的数据。

3. **[查询引擎总览 (Parsing README)](parsing/README.md)**
   - **必读**。`DataQuery` 子系统（大满贯查询机）的宏观介绍与四大核心流水线骨架图。
4. **[01 周期参数归一化 (Period Normalization)](parsing/01_period_normalization.md)**
   - 如何把人类理解的“近7天”、“今年”转换成数据库认识的绝对时间戳（`from_timestamp` / `to_timestamp`）。
5. **[02 动态安全 SQL 组装 (Filter SQL Build)](parsing/02_filter_sql_build.md)**
   - 防注入的动态参数绑定魔法！如何根据用户的 `--filter study` 以及日期边界，拼装出包含 JOIN 的强悍 WHERE 语句。
6. **[03 执行与拆包解码 (SQL Execution & Row Decode)](parsing/03_sql_execution_row_decode.md)**
   - 连接数据库，执行并用 Cursor / Statement 把裸字节解码为 C++ 内存对象的机制。
7. **[04 语义降维投射 (Semantic Projection)](parsing/04_semantic_projection.md)**
   - 将扁平的数据库明细对象集，提炼、聚合为真正服务于业务的上层结构 `DataQueryData`（统计面板等）。

### 第二部分：报告格式化与树状图实现 (Reports & Tree Generation)
**核心职责：** **“只管数据的聚合计算与华丽呈现”**
这部分代码接手了第一部分吐出的 `DataQueryData` 扁平结果集。它不碰任何 SQL 执行，只负责在内存里捏造出立体的结构，甚至渲染出字符画。

8. **[树图生成与渲染核心算法 (Tree Algorithms)](generation/tree_algorithms.md)**
   - **核心必读**。拿到阶段二吐出的一堆扁平路径（比如 `/study/math` 包含了 2 个小时），通过什么样的算法才能把它们合并成一棵携带节点权重的、完美的关联树？
   - 并且，如何利用深度遍历把这棵内存树绘制成带有 `├──` 与 `└──` 分支的字符画艺术？
9. **[报告格式化引擎 (Report Formatting)](generation/report_formatting.md)**
   - 记录了在生成了 `DailyReportData` 之后，底层代码是如何通过 `ReportDtoFormatter` 和各个细分语言包（Markdown/LaTeX/Typst），高效利用 C++ 的纯字符流拼接，将冰冷的时长结构体转化为供用户直接外发阅览的 `.md`、`.tex` 的完整实现机理。

---

## 结语

按照 `Schema -> Parsing Flow -> Tree Generation` 的路线学习，您将能毫无阻力地看懂整个 Time Tracer 查询生态的核心血脉。这也是所有在 UI 层看到数据之前，引擎在毫秒级内跑完的所有黑盒操作。

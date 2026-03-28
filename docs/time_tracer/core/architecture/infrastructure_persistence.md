# Infrastructure 层架构：持久化与外部服务基石

## Migration Note
1. Capability-first persistence routing now starts at
   `docs/time_tracer/core/capabilities/persistence/README.md`.
2. This document remains the long-form architecture explanation for persistence
   internals during the migration.

本篇文档解析 Time Tracer 架构中最外围、也是最“重”的物理防守层 —— `infrastructure`（基础设施）层。

根据 Clean Architecture 的原则，内部的 `domain`（业务模型）和 `application`（用例调度）都是不包含任何具象存储或外部依赖的乌托邦。所有的底层 I/O 实现——例如读写文件、连接 SQLite 数据库、调用系统的时钟或终端彩色打印——全部被隔离在这层执行。

## 1. 核心职责分解

`infrastructure` 层完全实现了定义在 `application/ports` 中的虚接口契约。
其目录位于：`libs/tracer_core/src/infra/`

它的两大最核心命脉板块为：
- **`persistence` (持久化存储)**：负责将清洗后的数据行写入本地数据库，并承接复杂的多维聚合查询。
- **`platform` / `io` (跨平台与底层服务)**：封装系统时间获取、日志捕获、原生控制台 API 的调用。

### 1.1 `importer/sqlite/writer` (纯数据的高速入库机)
**核心基建**：`libs/tracer_core/src/infra/persistence/importer/sqlite/writer.cpp`

我们在 `application` 篇中提到过，`domain` 层产生的 `DailyLog` 对象会被转换为不包含业务逻辑的 `ParsedData` 结构，然后传递给 Writer。这体现了应用逻辑与持久化实现之间的解耦：

- **简单高效的插入逻辑**：`writer.cpp` 的本职工作非常直接，拿到 `DayData`，绑定参数给 SQLite 的 `Statement`，然后执行 `INSERT OR IGNORE INTO days`。
- **事务与并发防坠**：通过 RAII (Resource Acquisition Is Initialization) 风格封装的 `Transaction` 对象，确保无论这几百条明细中途哪个数据违反了 SQLite 的唯一约束（如日期主键冲突），或者是意外崩溃，整批数据的写入能够安全滚回。它保证了：“要么这几十天的 TXT 被完整安全地吃进去，要么不留一点残渣”。

### 1.2 `ProjectResolver` (树形节点的动态织梦者)
**核心基建**：`libs/tracer_core/src/infra/persistence/importer/sqlite/project_resolver.cpp`

Time Tracer 在持久化 `time_records` 时，并未简单地把 `study_math_advanced` 存成一个孤立文本，而是依靠一套严格的层级主外键树关系 (`projects` 表)。

- **项目路解决流程**：`writer` 在接收到数据时，若遇到 `project_path`（如 `sleep_night`），会调用 `ProjectResolver` 进行路径解析。
- `ProjectResolver` 维护了一个层级缓存。它会解析下划线分隔的路径，并同步数据库中的 `projects` 表级别：
  - 首先查询根项目 `sleep`，若不存在则创建。
  - 接着处理子项目 `night`，将其关联至父项目 ID。
- 最终，解析器返回叶子节点的唯一 ID，由 `writer` 作为外键存入明细记录中。

### 1.3 `persistence.runtime` 的边界
`persistence.runtime` 现在只保留稳定的数据库运行时支撑，不再承载 query-owned 的主读服务入口。

当前稳定 owner 包括：

- `libs/tracer_core/src/infra/persistence/repositories/sqlite_project_repository.*`
- `libs/tracer_core/src/infra/persistence/sqlite_database_health_checker.*`
- `libs/tracer_core/src/infra/persistence/sqlite/db_manager.*`

`query data` 的主读服务入口已经对齐到 `query` owner 路径，由：

- `libs/tracer_core/src/infra/query/data/repository/query_runtime_service.*`

承接请求入口、action 分发与 query-specific read orchestration。

## 2. 数据库连接管理核心

**核心基建**：`db_manager.hpp`

- 实现了对 `sqlite3` 裸指针生命周期的安全 RAII 封装。
- 对表结构的升级和迁移管理（`schema/` 目录）。所有的应用从一开始都是不知道如何建表的，当系统首次在某台新电脑上启动，是 `db_manager` 读取内部的 Schema，从无到有地为用户建立起 `days`, `time_records`, `projects` 等坚不可摧的底层基座。

## 3. 为什么要有这种庞大的跨层屏障？

很多小型工具可能会直接在业务计算的时候引入 `#include <sqlite3.h>` 去建表。但对于 Time Tracer：
1. **测试替身 (Testability)**：因为有了这一层，我们的单元测试可以瞬间编写一个挂载于 `application` 层的 `FakeProcessedDataStorage`（即写在内存 `std::vector` 里）。不需要在 CI 服务器上真配一个数据库，依然能让整个引擎满载狂奔进行单元测试。
2. **底层引擎多平台支持**：核心计算逻辑在 C++ 层实现，避免了在不同平台（如 Android）重复实现复杂的时间处理算法。Android 端可以通过 JNI 调用 `application` 层的入库接口，底层自动映射至 `sqlite::Writer`。这种架构确保了核心逻辑在桌面端与移动端的统一性。

---

## 4. 总结

在架构体系中：
- `domain` 层定义了时间度量与业务校验的核心规则。
- `application` 层定义了数据流转与用例调度的逻辑路径。
- `infrastructure` 层负责将抽象的业务数据进行持久化存储及物理媒介交互。

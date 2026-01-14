# 开发设计模式与工程实践 (Design Patterns & Engineering Practices)

本文档记录了系统的核心设计哲学与代码实现准则，旨在指导后续功能的开发，确保系统架构的解耦性与可扩展性。

## 1. 关注点分离：业务、IO 与编排 (Separation of Concerns)

系统的设计核心在于将“做什么（逻辑）”、“怎么存（IO）”与“何时做（编排）”彻底解耦。

### 1.1 业务逻辑模块的纯净化 (Pure Logic Units)

* **设计准则**：具体的业务 Service（如 `ConverterService`）被设计为**纯粹的计算单元**。
* **实现方式**：
* **禁止直接 IO**：业务模块不直接操作文件系统或数据库，而是通过标准输入输出流（如 `std::istream`）或数据对象进行交互。
* **所有权移交**：通过回调函数（如 `std::function<void(DailyLog&&)>`）将生成的数据对象移交给外部，而不关心数据最终是写入磁盘还是存入数据库。


* **优势**：极大地提高了可测试性。可以在不创建物理文件的情况下，通过内存流对核心逻辑进行单元测试。

### 1.2 IO 模块的工具化 (Utility IO)

* **设计准则**：所有的文件读写、路径检查和目录管理统一托管在 `io` 命名空间下。
* **实现方式**：通过 `FileReader`、`FileWriter` 和 `FileSystemHelper` 提供原子化的操作接口。

### 1.3 Core 层的编排化 (Orchestration)

* **设计准则**：`core` 模块（如 `WorkflowHandler`, `PipelineManager`）作为系统的“大脑”，负责调度业务逻辑与 IO 设施。
* **实现方式**：Core 层负责读取文件、启动 Service、接收 Service 产出的数据并调用存储逻辑。

---

## 2. 模型与配置的耦合策略

系统采用“**外周解耦，核心耦合**”的策略，利用 `common` 模块作为全系统的通信语言。

### 2.1 Common 层的中心化

* **设计准则**：将全局通用的数据模型（如 `DailyLog`）和配置定义（如 `AppConfig`）提取至 `common` 层。
* **解耦逻辑**：具体业务模块（如 `Converter`）仅依赖 `common` 中的模型定义，不依赖具体的配置文件路径或复杂的 Core 环境。
* **强耦合逻辑**：Core 层与 `common` 模型保持**强耦合**。这是设计使然，因为编排者必须深刻理解配置细节和数据结构，才能完成复杂的业务调度。

---

## 3. 配置分发设计 (Configuration Injection)

程序采用了**自顶向下、引用注入**的模式来管理全局配置。

### 3.1 加载与持有

* **加载**：在 `main_cli.cpp` 中，通过 `ConfigLoader` 确定执行路径并解析 `config.json`，生成唯一的 `AppConfig` 实例。
* **生命周期**：配置对象由 `main` 函数或 `CliController` 持有，其生命周期贯穿整个程序运行过程。

### 3.2 注入路径

1. **控制层注入**：`main` 将配置传递给 `CliController`。
2. **编排层注入**：`CliController` 根据命令实例化 `WorkflowHandler`，并将配置作为**常量引用 (`const AppConfig&`)** 注入。
3. **任务层分发**：`WorkflowHandler` 在执行具体流水线（`PipelineManager`）时，会将配置信息（或其子集）同步到 `PipelineContext` 中，供各个执行步骤使用。

### 3.3 设计优势

* **内存高效**：使用 `const&` 传递避免了大型配置对象的频繁拷贝。
* **安全性**：常量约束确保了下游模块只能读取配置，无法在运行时非法篡改全局参数，保证了系统状态的一致性。

---

## 4. 后续开发准则

* **新增业务功能时**：应当定义在 `Service` 模块中，接受数据流输入，返回/回调结构化模型。
* **涉及配置扩展时**：应先在 `common/config/AppConfig.hpp` 中扩展结构，在 `ConfigLoader` 中实现解析逻辑，最后由 Core 层进行分发。
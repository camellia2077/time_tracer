# 配置文件加载架构说明

## 1. 核心流程概述

系统采用“集中加载、逐层分发、按需解析”的模式处理 JSON 配置。主配置文件 `config/config.json` 定义所有子模块配置文件的物理路径。

## 2. 模块职责划分

系统在配置处理上实现了严格的职责分离：

| 模块 | 核心类 | 职责描述 |
| --- | --- | --- |
| **底层加载层** | `ConfigLoader` | **唯一直接读取 `config.json` 的模块**。负责将相对路径转换为绝对路径，并填充 `AppConfig` 结构体。 |
| **逻辑控制层** | `FileController` | **配置持有者**。调用加载器，并在系统启动阶段触发 `ConfigFacade` 对所有路径进行合法性校验。 |
| **业务分发层** | `ActionHandler` | **分发器**。如 `ReportHandler` 会从全局配置中提取特定路径（如 `day_typ_config_path`），传递给具体的业务 Service。 |
| **执行消费层** | `reprocessing` / `reports` | **消费者**。不关心主配置，只接收由分发层传来的特定文件路径，并加载业务所需的详细参数。 |

## 3. 数据流向图

配置数据的生命周期如下：

1. **初始化**：`ConfigLoader` 定位可执行文件目录并加载 `config/config.json`。
2. **校验**：`FileController` 获取 `AppConfig` 后，立即验证所有 JSON 路径是否存在且符合 Schema。
3. **分发**：根据用户指令，`FileHandler` 或 `ReportHandler` 提取所需的子配置路径。
4. **解析**：底层工具类（如 `ConverterConfig` 或 `ConfigUtils`）读取具体的业务配置。

## 4. 关键配置参数说明

在 `config.json` 中定义的参数主要分为三类：

* **系统环境**：如 `error_log_path` 和 `export_path`。
* **预处理引擎**：`interval_processor_config_path` 指向数据转换核心配置。
* **报表模板**：涵盖 `day` / `month` / `period` 三种周期，以及 `tex` / `md` / `typ` 三种格式的路径映射。

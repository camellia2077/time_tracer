# 配置文件加载架构说明

## 1. 核心流程概述

系统采用“集中加载、逐层分发、按需解析”的模式处理 **TOML** 配置。主配置文件
`config/config.toml` 定义各子模块配置文件路径与默认参数。`ConfigLoader`
负责解析该文件并填充 `AppConfig`，同时加载 Converter 与报表配置（支持
`style_source` 合并）。

## 2. 模块职责划分

| 模块 | 核心类 | 职责描述 |
| --- | --- | --- |
| **主配置加载层** | `ConfigLoader` | 读取 `config.toml`，解析可执行文件目录并将相对路径转换为绝对路径，填充 `AppConfig`。 |
| **配置解析层** | `ConfigParserUtils` | 解析 `[system]`/`[defaults]`/`[commands]`/`[converter]`/`[reports]` 并写入 `AppConfig`。 |
| **转换配置加载层** | `ConverterConfigLoader` | 从 `interval_config` 读取并合并 Converter 配置，注入 `initial_top_parents`。 |
| **报表配置加载层** | `ReportConfigLoader` + `TomlLoaderUtils` | 读取报表 TOML，合并 `style_source` 并解析为结构体，填入 `AppConfig.loaded_reports`。 |
| **应用分发层** | `CliApplication` / `AppContext` | 持有 `AppConfig`，计算默认 `db_path`/`output_root` 并注入 `WorkflowHandler` 与 `ReportHandler`。 |
| **业务消费层** | `PipelineManager` / `ReportGenerator` | 运行时按需加载 Converter/Report 配置并执行转换、查询与导出。 |

## 3. 数据流向图

1. **初始化**：`CliApplication` 获取可执行文件路径并创建 `ConfigLoader`。
2. **解析主配置**：`ConfigLoader` 读取 `config/config.toml`，调用
   `ConfigParserUtils` 解析基础字段与路径。
3. **加载 Converter 配置**：`ConverterConfigLoader` 读取
   `converter.interval_config`，并将 `initial_top_parents` 注入到
   `ConverterConfig`。
4. **加载报表配置**：`ReportConfigLoader` 读取各格式的报表 TOML，
   `TomlLoaderUtils::read_toml` 负责合并 `style_source`。
5. **分发**：`AppConfig` 存入 `AppContext`，注入 `WorkflowHandler` 和
   `ReportHandler`。
6. **运行时使用**：
   - `PipelineManager` 在转换/验证时根据 `interval_config` 再次加载
     Converter 配置，确保运行时一致性。
   - `GenericFormatterFactory` 根据 `AppConfig` 中的报表路径读取 TOML，
     将合并后的配置内容传递给 DLL 格式化器。

## 4. 关键配置参数说明

`config/config.toml` 中的参数主要分为以下几类：

* **系统参数**（`[system]`）：`error_log`、`export_root`、
  `save_processed_output`、`date_check_continuity`。
* **默认值**（`[defaults]`）：`db_path`、`output_root`、`default_format`。
* **命令默认值**（`[commands.*]`）：`convert`/`ingest`/`validate-logic`/
  `export`/`query` 的默认 `format`、`date_check`、`save_processed_output` 等。
* **转换配置**（`[converter]`）：`interval_config` 与
  `initial_top_parents`。
* **报表配置路径**（`[reports]`）：`markdown`/`latex`/`typst` 的
  `day`/`month`/`period`/`week`/`year` 路径，报表文件内部可声明
  `style_source` 以复用公共样式。

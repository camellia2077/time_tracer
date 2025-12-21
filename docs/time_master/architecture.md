# 架构与模块

## 1. 报表模块 (Reports)

### 1.1 基础设施层
* **配置加载 (ConfigUtils)**
    * **路径**: `reports/shared/utils/config/ConfigUtils`
    * **职责**: 基于 `nlohmann/json` 封装，负责从磁盘读取并解析 JSON 配置文件，为格式化器提供参数。

### 1.2 核心业务层
* **渲染引擎 (ProjectTreeFormatter)**
    * **路径**: `src/reports/shared/formatters/base/ProjectTreeFormatter.cpp`
    * **职责**: 负责将内存中抽象的 `ProjectTree` 数据结构转换为最终的格式化文本。
    * **机制**: 采用策略模式（Strategy Pattern），解耦了“树的遍历逻辑”与“具体格式（Markdown/Typst/LaTeX）的生成逻辑”。
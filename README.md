[English Version](README.en.md) | [中文版本](README.md)

# time tracer ![License](https://img.shields.io/badge/License-Apache%202.0-blue.svg)

**time tracer** - 基于 C++23 构建的个人时间追踪与分析系统。

这是一套功能强大的个人时间管理工具集，采用 **Clean Architecture** (整洁架构) 设计，旨在提供极致的输入效率、稳健的数据存储以及多维度的可视化分析。

### 设计理念（简要）

1. **数据归用户所有**：记录以可读文本保存，用户可长期持有、备份、迁移，不被单一 App 绑定。  
2. **支持快速修改数据**：可直接编辑文本（改活动名、加备注等），再同步更新数据库与报告。  
3. **跨平台同一输入**：CLI、Android 等平台使用同一种文本数据作为输入，减少格式切换成本。  

### 核心组件

* **`time_tracer_cli` (C++23)**: 核心命令行程序。采用管道模式处理原始文本日志，提供基于 SQLite 的高效查询及多格式（Markdown, LaTeX, Typst）报表导出。
* **`graph_generator` (Python)**: 数据可视化工具。读取数据库并生成动态图表（如时间线、热力图）。
* **`log_generator` (C++)**: 辅助工具。用于生成符合规范的测试日志数据。
---

## 🚀 快速开始

### 1. 环境依赖

* **C++ 组件 (`time_tracer_cli`)**:
    * **MSYS2 UCRT64** (Windows 推荐)
    * **CMake** >= 3.25 (C++23 支持)
    * **编译器**: Clang 16+ 或 GCC 13+
    * **核心库**: SQLite3, nlohmann/json, toml++
* **Python 组件 (`graph_generator`)**:
    * **Python** >= 3.8, Matplotlib

### 2. 构建指南

我们提供了全自动的构建脚本，可一键完成核心程序与运行时交付物的编译。

项目统一使用 Python 工具链命令进行构建和测试，不建议直接调用 `cmake`/`ninja` 或自定义 `ps1`。
➡️ **Python 工具链修改定位图（Agent/开发者）**：[docs/toolchain/python_command_map.md](docs/toolchain/python_command_map.md)
➡️ **clang-tidy 标准流程 SOP**：[docs/toolchain/clang_tidy_sop.md](docs/toolchain/clang_tidy_sop.md)

```bash
# 构建（time_tracer）
python tools/run.py build --app tracer_core --profile release_safe --build-dir build

# 快速验证（构建 + 测试）
python tools/run.py verify --app tracer_core --quick

# 代码行数扫描（开发辅助工具，可选）
python tools/scripts/devtools/loc/scan_cpp_lines.py apps/cli/windows apps/tracer_core_shell libs/tracer_core -t 350
```

➡️ **详细步骤请参考：[构建指南](docs/time_tracer/guides/build_guide.md)**

### 3. 基本使用示例

**示例 1：自动化摄取流水线 (Blink)**
（校验、转换、链接逻辑、持久化一步到位）

```bash
# 处理 target_logs 目录下的所有原始日志
time_tracer_cli blink -a "path/to/target_logs"
```

**示例 2：查询数据清单**

```bash
# 查询 2026年 的所有日期记录
time_tracer_cli query data days --year 2026
```

**示例 3：导出格式化报表**

```bash
# 导出 2026-W05 的周报为 Markdown 格式
time_tracer_cli export week 2026-W05 -f md
```

---

## 📚 详细文档

项目文档已按照 **设计、指南、流程** 进行重新组织，以便于快速查阅：

```text
docs/time_tracer/
├── design/                 # 架构设计与核心逻辑
│   ├── architecture.md     # Clean Architecture 分层说明
│   └── system_design.md    # 设计哲学与数据流
├── guides/                 # 操作手册与配置指南
│   ├── build_guide.md      # 环境搭建与构建步骤
│   └── cli_query_guide.md  # 详细的 CLI 查询命令参考
└── workflows/              # 执行流程展示
    └── workflow.md         # 核心命令的全链路流程图
```

---

## 开发者与致谢

### 核心开发者
* **[camellia2077](https://github.com/camellia2077)**: 项目发起者。

### AI 合作开发者
在此感谢以下模型在重构与架构优化中提供的协助：
* **Google**: Gemini 2.5 Pro, 3 Pro, 3 Flash, 3.1 Pro
* **Anthropic**: Claude 4.5 Opus, 4.6 Opus
* **OpenAI**: GPT-5.2 Codex, 5.3 Codex ,5.4

---

## 免责声明 (Disclaimer)

本软件仅作为个人效率管理工具使用，严禁用于任何违反所在地法律法规的行为。开发者不认同、不参与、不承担任何第三方利用本软件进行政治宣传的后果。

---

## 许可证与开源库

本仓库自有源码使用 **Apache License 2.0** 许可证（见 `LICENSE`）。
第三方依赖保持各自原始许可证。

### 核心与工具链

* **[SQLite](https://www.sqlite.org/)**: 嵌入式数据库 (Public Domain)。
* **[nlohmann/json](https://github.com/nlohmann/json)**: JSON 解析 (MIT)。
* **[tomlplusplus](https://github.com/marzer/tomlplusplus)**: TOML 配置处理 (MIT)。
* **[libsodium](https://github.com/jedisct1/libsodium)**: 加密基础库（计划用于 `tracer_core` 导出/分享文件加密能力）(ISC License)。
* **[Apache ECharts](https://echarts.apache.org/)**: Windows CLI `report-chart` 单文件 HTML 图表渲染（Line/Bar/Pie/Heatmap-Year/Heatmap-Month）(Apache License 2.0)。
* **[Matplotlib](https://matplotlib.org/)**: 绘图引擎 (BSD 风格许可证)。

### Windows Rust CLI (`apps/cli/windows/rust`)

* **[clap](https://github.com/clap-rs/clap)**: Rust CLI 参数解析与子命令框架（MIT 或 Apache License 2.0）。
* **[thiserror](https://github.com/dtolnay/thiserror)**: Rust 错误类型派生（MIT 或 Apache License 2.0）。
* **[libloading](https://github.com/nagisa/rust_libloading)**: 动态库加载（如 runtime DLL）(ISC License)。
* **[serde](https://github.com/serde-rs/serde)**: 序列化/反序列化框架（MIT 或 Apache License 2.0）。
* **[serde_json](https://github.com/serde-rs/json)**: JSON 处理（MIT 或 Apache License 2.0）。
* **[toml](https://github.com/toml-rs/toml)**: TOML 解析（MIT 或 Apache License 2.0）。

依赖版本清单位于：
* `apps/cli/windows/rust/Cargo.toml`
 
### Android 应用 (`apps/android`)

* **[AndroidX / Jetpack Compose 系列](https://github.com/androidx/androidx)**  
  包含本项目 Android 端使用的 `core-ktx`、`lifecycle-*`、`activity-compose`、`compose-*`、`datastore-preferences` 以及 AndroidX 测试库。  
  **许可证**: Apache License 2.0。
* **[Material Components for Android](https://github.com/material-components/material-components-android)** (`com.google.android.material:material`)  
  **许可证**: Apache License 2.0。
* **[Multiplatform Markdown Renderer](https://github.com/mikepenz/multiplatform-markdown-renderer)** (`com.mikepenz:multiplatform-markdown-renderer-m3`)  
  **许可证**: Apache License 2.0。
* **[JUnit 4](https://github.com/junit-team/junit4)** (`junit:junit`，仅测试依赖)  
  **许可证**: Eclipse Public License 1.0 (EPL-1.0)。

依赖版本清单位于：
* `apps/android/gradle/libs.versions.toml`

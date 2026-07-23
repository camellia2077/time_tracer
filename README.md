[English Version](README.en.md) | [中文版本](README.md)

# Time Tracer ![License](https://img.shields.io/badge/License-Apache%202.0-blue.svg) [![Windows Build Matrix](https://github.com/camellia2077/time_tracer/actions/workflows/windows-build-matrix.yml/badge.svg)](https://github.com/camellia2077/time_tracer/actions/workflows/windows-build-matrix.yml) [![Android CI](https://github.com/camellia2077/time_tracer/actions/workflows/android-ci.yml/badge.svg)](https://github.com/camellia2077/time_tracer/actions/workflows/android-ci.yml)

<p align="center">
  <img src="ui/branding/master/time_tracer_brand_master_symbol.svg" alt="Time Tracer Logo" width="120" height="120">
  <br>
  <em>Icon designed for camellia2077/time_tracer</em>
</p>

**Time Tracer** - 一个以 Android 为主入口的层级时间记录系统。它通过可配置 alias mapping（别名映射），将用户快速输入的活动 token 自动归一到多层级活动目录中，并在父节点与子节点之间自动聚合持续时间，用于细粒度的个人时间复盘与行为分析。

Time Tracer 的目标不是简单记录“今天学习了多久”或“今天运动了多久”，而是帮助用户在不增加明显输入负担的前提下，进一步看清这些时间具体流向了哪些下级活动。例如，“学习”下面可以继续拆分为“计算机”“数学”，其中“计算机”可以继续拆分为“算法”“计算机组成”“计算机网络”，“数学”可以继续拆分为“微积分”“线性代数”；“健身”下面也可以继续拆分为“力量训练”“有氧训练”等子项。

系统采用 **Android + Core Engine** 的结构：Android 负责日常快速记录与交互，核心引擎负责解析、归一、聚合、查询与报表生成。原始记录以纯文本日志保存，SQLite 主要作为查询与统计层，用于提升检索、聚合和生成报告的效率。

### 设计理念（简要）

1. **低成本记录**：用户可以用中文、英文、缩写或自定义别名快速输入活动，不需要每次手动选择完整层级路径。  
2. **层级活动目录**：活动不是平铺的标签，而是类似文件夹的层级结构；每个活动节点都可以继续拥有子节点。  
3. **父子节点自动聚合**：一段时间记录到子节点后，会自动计入其所有父节点，从而同时支持细粒度统计与高层级汇总。  
4. **文本作为事实来源**：原始记录以可读 TXT 保存，用户可长期持有、备份、迁移；SQLite 与报告均由文本记录派生。  
5. **跨平台同一数据模型**：Android、CLI 与后续报表使用同一套活动映射和统计语义，减少格式与平台切换成本。  

### 文本输入与别名映射示例

Time Tracer 的日常使用入口是 Android，但底层记录可以落到简单、可读的 TXT 文本中。TXT 更像“时间字幕”：每一行描述某个时间点或时间区间发生了什么。

例如，事件行可以采用 `HHMM + 活动 token` 或 `HHMM-HHMM + 活动 token` 的形式：

```text
0613起床
0634吃早饭
0640-1038算法
1038-1224线性代数
1443-1922重积分 // 习题训练
1930-2030力量训练
```

这里的 `起床`、`算法`、`线性代数`、`重积分`、`力量训练` 都是用户实际输入的活动 token。它们可以是中文、英文、缩写或任意自定义写法。系统会通过 alias mapping 将这些 token 解析到规范活动路径中。

#### 日常活动示例

对应的 alias child file 可以写成：

```toml
parent = "routine"

[aliases]
"洗漱" = "oral-hygiene"
"刷牙" = "oral-hygiene"
"o" = "oral-hygiene"
```

上面的配置表示：

* `洗漱`、`刷牙`、`o` 都会被解析为同一个规范活动路径 `routine_oral-hygiene`
* 左键是用户实际输入的活动 token，可以尽量短，方便快速记录
* 右键是系统用于统计和归类的规范活动名
* 多个输入 token 可以映射到同一个活动节点，用于统一统计口径

#### 多层级活动示例

更复杂的层级示例如下：

```toml
parent = "study"

[aliases]
"cs" = "computer-science"
"计算机" = "computer-science"
"math" = "math"
"数学" = "math"

[aliases.computer-science]
"算法" = "algorithm"
"algo" = "algorithm"
"计组" = "computer-organization"
"计算机组成" = "computer-organization"
"网络" = "computer-network"
"计算机网络" = "computer-network"

[aliases.math]
"微积分" = "calculus"
"线代" = "linear-algebra"
"线性代数" = "linear-algebra"

[aliases.math.calculus]
"重积分" = "multiple-integral"
"二重积分" = "multiple-integral"
```

```toml
parent = "fitness"

[aliases]
"力量训练" = "strength-training"
"力量" = "strength-training"
"有氧训练" = "cardio"
"有氧" = "cardio"
```

这些配置会展开成类似下面的规范活动路径：

* `算法` / `algo -> study_computer-science_algorithm`
* `计组` / `计算机组成 -> study_computer-science_computer-organization`
* `网络` / `计算机网络 -> study_computer-science_computer-network`
* `线代` / `线性代数 -> study_math_linear-algebra`
* `重积分` / `二重积分 -> study_math_calculus_multiple-integral`
* `力量训练` / `力量 -> fitness_strength-training`
* `有氧训练` / `有氧 -> fitness_cardio`

这些路径不仅用于“把活动 token 变成长名字”，更重要的是定义了该活动在层级目录中的位置。

例如，`重积分 -> study_math_calculus_multiple-integral` 可以表示“学习中，数学分类下，微积分方向的重积分训练”。当一段时长被记到叶子节点 `study_math_calculus_multiple-integral` 时，聚合统计会同时把这段时间计入它的所有父节点，包括：

* `study_math_calculus`
* `study_math`
* `study`

同理，`力量训练 -> fitness_strength-training` 会被计入：

* `fitness_strength-training`
* `fitness`

因此，同一份记录既可以支持非常细粒度的统计，也可以支持更高层级的汇总。

这里用学习和健身举例，是因为它们能直观展示 Time Tracer 的核心能力：用户只需要输入一个短 token，系统就能把它归入完整的多层级活动目录，并自动完成向上聚合。

从统计与查询语义上，可以把这套映射理解为一棵带权活动树：

* 节点是规范活动路径中的各级活动名
* 权重来自最终归属于该节点及其子节点的持续时间
* 用户输入可以很短、很自由，但统计、查询和报表都基于归一后的规范路径进行
* 子节点时间会自动累加到父节点，因此可以在不同层级查看同一份时间数据

需要注意的是，这是一种统计语义模型，而不是 ingest 主流程本体的完整描述。程序的主流程是先把文本或 Android 输入解析为标准化活动记录并持久化，再在查询或报表阶段按需要将这些记录投影为树状聚合结果。

当前 alias child file 的展开规则是：

* `parent` 是顶层路径段
* `[aliases.xxx.yyy]` 是中间层级
* 右值字符串是叶子路径段
* 最终规范活动路径采用 `_` 连接，例如 `study_math_calculus_multiple-integral`


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
➡️ **Python 工具链修改定位图（Agent/开发者）**：[docs/tools/toolchain/command_map/python.md](docs/tools/toolchain/command_map/python.md)
➡️ **clang-tidy 标准流程 SOP**：[docs/tools/toolchain/tidy/sop.md](docs/tools/toolchain/tidy/sop.md)

```bash
# 构建（time_tracer）
python tools/run.py build --app tracer_core --profile release_safe --build-dir build

# 快速验证（构建 + 测试）
python tools/run.py verify --app tracer_core --quick

# 代码行数扫描（开发辅助工具，可选）
python -m tools.devtools.loc_scanner --lang cpp apps/cli/windows apps/tracer_core_shell libs/tracer_core --over 350
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

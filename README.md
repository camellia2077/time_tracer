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

### 记录什么

每条记录包含一个时间点或时间区间，以及用户输入的活动名称。例如：

```text
0613起床
0640-1038算法
1930-2030力量训练
```

活动名称可以是中文、英文、缩写或自定义 alias。系统会把它解析为规范活动路径，记录实际持续时间，并保留可读的 TXT 原始日志。SQLite 是由 TXT 派生的查询与统计数据，不是另一套需要手工维护的记录来源。

### 活动层级与记录方式

活动目录类似文件夹树。普通 alias 用于把输入名称映射到叶子活动；一个 group 也可以通过 `group_aliases` 直接成为可记录活动，同时继续拥有子活动：

```toml
parent = "exercise"

[canonical.strength-training]
group_aliases = ["无氧训练"]

[canonical.strength-training.squat]
group_aliases = ["蹲"]
"深蹲" = "squat"
```

因此可以分别记录到：

* `无氧训练` → `exercise_strength-training`
* `蹲` → `exercise_strength-training_squat`
* `深蹲` → `exercise_strength-training_squat_squat`

子活动的时间会自动向上聚合到所有父节点。例如记录到 `exercise_strength-training_squat`，查询 `exercise_strength-training` 和 `exercise` 时也会包含这段时间。这样同一份数据既能看具体动作，也能看分类和总览。

### 活动是否可以移动或改名

可以通过 CLI 或 Android 的配置能力修改活动结构：

* 可以把原本的叶子 alias 提升为可记录 group；提升本身不改变已有规范路径。
* 可以把叶子 alias 移入已有层级。移动会改变规范路径，例如从 `exercise_running` 变为 `exercise_cardio_running`。
* 可以为已有 group 重命名记录别名或增加新的记录别名。

移动叶子或重命名 group 记录别名会影响历史数据。系统会同步更新 canonical TOML、TXT 中的规范活动 token，并重建数据库；所有步骤成功后才替换正式数据。增加 group 记录别名只影响今后的输入，不需要修改历史 TXT 或重建数据库。当前不能把一个 group 及其全部子孙作为整体一次移动。

### 数据如何展示

查询和报告使用规范活动路径进行统计，而不是按用户输入的原始写法分别统计。报告会展示：

* 时间范围内的总记录时长、记录天数和活动数量；
* 按活动树展开的时长明细，父节点包含子节点的聚合时长；
* 日、周、月、年和自定义范围等不同时间尺度的汇总；
* Markdown、LaTeX 和 Typst 格式的导出结果，其中 Markdown 支持中文、英文和日文文本。

因此，输入 `力量训练`、`力量` 或其他映射到同一规范路径的 alias，最终会在查询和报告中作为同一个活动节点展示。TOML 主要用于定义 alias 与层级结构；具体配置规则和迁移约束见 `docs/time_tracer/core/capabilities/config/`。

### Android 中的查询展示

Android 目前是主要开发入口，查询结果有以下几种面向用户的展示方式：

* **Timeline**：日查询可以按时间顺序查看当天的活动。每项显示开始时间、结束时间、活动路径、持续时长和备注；路径会按“顶层分类 > 下级活动”的形式展示。Timeline 还支持修改当天备注和活动备注。
* **Tree**：按选定的日、周、月、年、最近或自定义范围查询活动树。节点默认展开，点击有子节点的节点可以展开或收起；节点显示规范名称、相对路径、聚合时长，并用进度条显示占整个当前树结果的比例（子节点比例按其父节点计算）。结果可以按时长升序或降序排列。
* **活动构成图表**：从同一棵活动树生成 Pie、Horizontal Bar 和 Treemap 三种图表。图表支持按时长或活动次数统计。

活动构成图表支持沿路径逐层查看：点击一个有子节点的活动后，会进入该活动对应的下一级，交互方式类似打开文件夹；界面会显示当前路径，并提供返回上一级操作。进入某个层级后，图表只输出该节点的直接子节点，并计算它们在当前层级中的时长或次数比例，因此显示的是“当前节点内部的构成比例”，不是固定的全局比例。图例和选中项同时显示对应数值与百分比。


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

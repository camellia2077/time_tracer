[English Version](README.en.md) | [中文版本](README.md)

# Time Tracer ![License](https://img.shields.io/badge/License-Apache%202.0-blue.svg) [![Windows Build Matrix](https://github.com/camellia2077/time_tracer/actions/workflows/windows-build-matrix.yml/badge.svg)](https://github.com/camellia2077/time_tracer/actions/workflows/windows-build-matrix.yml) [![Android CI](https://github.com/camellia2077/time_tracer/actions/workflows/android-ci.yml/badge.svg)](https://github.com/camellia2077/time_tracer/actions/workflows/android-ci.yml)

<p align="center">
  <img src="ui/branding/master/time_tracer_brand_master_symbol.svg" alt="Time Tracer Logo" width="120" height="120">
  <br>
  <em>Icon designed for camellia2077/time_tracer</em>
</p>

**Time Tracer** - 基于 C++23 构建的个人时间追踪与分析系统，以纯文本日志为事实来源，并通过可配置 alias mapping（别名映射）将被记录的活动 token 归一为可统计的规范活动路径。


这是一套功能强大的个人时间管理工具集，采用 **Clean Architecture** (整洁架构) 设计，旨在提供极致的输入效率、稳健的数据存储以及多维度的可视化分析。
它也是一套以纯文本日志为事实来源的个人时间账本系统。用户可用任意语言、缩写或别名书写活动 token，系统通过可配置映射将其转换为规范活动语义，并用于统计、查询和报表。

### 设计理念（简要）

1. **数据归用户所有**：记录以可读文本保存，用户可长期持有、备份、迁移，不被单一 App 绑定。  
2. **支持快速修改数据**：可直接编辑文本（改活动名、加备注等），再同步更新数据库与报告。  
3. **跨平台同一输入**：CLI、Android 等平台使用同一种文本数据作为输入，减少格式切换成本。  
4. **作者态活动 token 与统计语义分离**：用户可使用任意语言、缩写或别名书写活动 token，系统通过可配置 alias mapping 将其归一为规范活动路径，再进行查询、统计与报表。  

### 文本输入与别名映射示例

记录文本是事实来源，用户在文本里写下的是作者态活动 token；系统会在导入、查询和报表阶段把这些 token 解析为规范活动路径。

例如，当前 TXT 中的事件行采用 `HHMM + 活动 token` 形式：

```text
0813o
0406r
0622rda // rank dva academy skins
```

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
* 左键是用户实际输入的活动 token，因此可以是中文、英文、缩写或其他自定义写法
* 多个左键映射到同一个右键是合法的，用于统一统计口径

#### 游戏活动示例

更复杂的层级示例如下：

```toml
parent = "others"

[aliases]
"r" = "rest"
```

```toml
parent = "games"

[aliases]
"ow" = "overwatch"
"mc" = "minecraft"

[aliases.overwatch]
"owr" = "rank"
"owrank" = "rank"
"owq" = "quickplay"

[aliases.overwatch.rank]
"dva" = "dva"
"tr" = "tracer"

[aliases.overwatch.rank.dva]
"rda" = "academy"
```

这些配置会展开成类似下面的规范活动路径：

* `r -> others_rest`
* `ow -> games_overwatch`
* `mc -> games_minecraft`
* `owr` / `owrank -> games_overwatch_rank`
* `owq -> games_overwatch_quickplay`
* `dva -> games_overwatch_rank_dva`
* `tr -> games_overwatch_rank_tracer`
* `rda -> games_overwatch_rank_dva_academy`

这些路径不仅用于“把活动 token 变成长名字”，还定义了活动树中的落点。

例如，`rda -> games_overwatch_rank_dva_academy` 可以表示“守望先锋中 rank 模式下，使用 D.Va 的 academy 皮肤所花费的时间”。当一段时长被记到叶子节点 `games_overwatch_rank_dva_academy` 时，聚合统计会同时把这段时间计入它的所有父节点，包括：

* `games_overwatch_rank_dva`
* `games_overwatch_rank`
* `games_overwatch`
* `games`

因此，同一份文本事实数据既可以支持非常细粒度的统计，也可以支持更高层级的汇总。

这里用 D.Va 和 academy 皮肤举例，不是因为 Overwatch 本身缺少角色游玩时间统计，而是因为这个例子层级更深，适合演示 alias mapping 如何把作者态活动 token 挂到活动树的深层叶子节点上。

从统计与查询语义上，可以把这套映射理解为一棵带权活动树：

* 节点是规范活动路径中的各级活动名
* 权重来自最终归属于该节点及其子节点的持续时间
* 用户写法可以自由变化，但统计、查询、报表都基于归一后的规范路径进行

需要注意的是，这是一种统计语义模型，而不是 ingest 主流程本体的完整描述。程序的主流程是先把文本解析为标准化活动记录并持久化，再在查询或报表阶段按需要把这些记录投影为树状聚合结果。

当前 alias child file 的展开规则是：

* `parent` 是顶层路径段
* `[aliases.xxx.yyy]` 是中间层级
* 右值字符串是叶子路径段
* 最终规范活动路径采用 `_` 连接，例如 `games_overwatch_rank_dva_academy`

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

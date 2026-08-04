# Usage

## 基本命令

```bash
python -m tools.devtools.loc_scanner (--lang <cpp|kt|py|rs> [paths ...] | --profile <name>) [--over N | --under [N] | --dir-over-files [N]] [--dir-max-depth N] [--log-file <path>]
```

## 示例

```bash
# Python 大文件
python -m tools.devtools.loc_scanner --lang py --over 200

# Kotlin 小文件（使用配置默认阈值）
python -m tools.devtools.loc_scanner --lang kt --under

# C++ 指定目录（当该语言 path_mode 允许命令行路径时）
python -m tools.devtools.loc_scanner --lang cpp libs --over 300

# 目录文件密度扫描
python -m tools.devtools.loc_scanner --lang py --dir-over-files --dir-max-depth 2

# 按组件聚合扫描；tests 会独立显示
python -m tools.devtools.loc_scanner --profile core_family --over 300
python -m tools.devtools.loc_scanner --profile workspace --over 500

# Android 专项扫描；只覆盖 apps/android
python -m tools.devtools.loc_scanner --profile android --over 350

# Windows Rust CLI 专项扫描；只覆盖 apps/cli/windows/rust，并评估 CLI 边界信号
python -m tools.devtools.loc_scanner --profile windows_cli --over 350

# 只扫描 tidy / clang 适配层中的 Python 代码
python -m tools.devtools.loc_scanner --profile tidy --over 200

# 分开查看 tests、tidy、clang adapters 和其他 toolchain glue
python -m tools.devtools.loc_scanner --profile python_tooling --over 200
```

单语言扫描会按源码集分组，避免把生产代码和测试代码混在一个热点列表中：

- `PRODUCTION`：路径中出现 `src/main`
- `TESTS`：路径中出现 `src/test`、`src/androidTest`、`test`、`tests` 或 `testFixtures`
- `OTHER`：不属于上述源码集的文件

分类结果会写入每个扫描路径的 `category_counts`，并写入每个文件的 `category`。

profile 报告中的 `Module Baseline Summary` 是全量源码基线，不受 `--over` 阈值限制，包含模块文件数、代码行数、生产/测试规模、源码目录数和 Top 文件；`hotspots` 仍表示超过本次阈值的文件数量。
模块摘要中的标签是可解释的统计信号：`LARGE_FILE` 表示存在超过阈值的文件，`MANY_LARGE_FILES` 表示至少 5 个文件超过阈值，`MANY_SOURCE_FILES` 表示至少 100 个源码文件，`TEST_HEAVY`/`TEST_LIGHT` 表示已识别测试证据的文件或代码行占比达到 40%/低于 10%，`HIGH_HOTSPOT_CONCENTRATION` 表示热点行数占模块总行数至少 40%，`OTHER_SOURCE_SET_PRESENT` 表示存在未归入生产/测试的源码集。Rust 文件中的 `#[cfg(test)]` 模块会作为估算的 inline test evidence 计入；标签用于辅助 Agent 规划，不替代代码语义判断。
模块综合排序只在当前 profile 内比较：架构优先级占 60%，模块规模占 20%，热点数量占 15%，边界信号占 5%。P0–P3 使用固定优先级基线，分数用于决定 Agent 的初始阅读顺序，不是代码质量评分。

`windows_cli` 还会生成组件专项评估：跨 CLI ownership zone 的热点、多个
command-handler family 的热点、Runtime ABI seam 热点、Rust 内嵌 `#[cfg(test)]` 测试
以及 test-support 文件。报告区分 path-based tests、inline tests 和 test-support；
只有未发现任何这些证据时才报告 `CLI_NO_DISCOVERED_TEST_EVIDENCE`。Windows CLI 的
黑盒 suite 和 Core/runtime contract tests 仍是测试证据来源，评估代码不会仅凭这些
信号决定重构。

重构前后对比：

```bash
# 首次扫描时保存 baseline
python -m tools.devtools.loc_scanner --profile core_family --over 350 \
  --save-baseline temp/loc_scanner/baselines/core_family_before.json

# 重构后进行比较
python -m tools.devtools.loc_scanner --profile core_family --over 350 \
  --compare-baseline temp/loc_scanner/baselines/core_family_before.json
```

`--save-baseline` 和 `--compare-baseline` 只适用于 `--profile`。输入必须是
之前生成的 profile JSON 报告，差异报告写入 `temp/loc_scanner/reports/`。

## 按组件聚合

profile 定义在 `config/scan_lines.toml` 的 `[profiles.*]` 节点中。当前 profile 包括：

- `core_family`：`libs/tracer_core`、`libs/tracer_adapters_io`、`libs/tracer_core_bridge_common`、`libs/tracer_transport`
- `presentation`：`apps/tracer_core_shell`、`apps/android`、`apps/cli`、`apps/server`
- `android`：仅 `apps/android`
- `windows_cli`：仅 `apps/cli` 中的 Rust CLI
- `tests`：顶层 `test` 和 `tools/tests`
- `tidy`：`tools/toolchain/commands/tidy` 和 `tools/toolchain/commands/clang`
- `python_tooling`：tests、tidy、clang adapters、toolchain glue 和 LOC Scanner
- `workspace`：以上三组组件

profile 输出首先按优先级分组，同一优先级内按代码行数从高到低排序；每条结果保留 `category/component/language` 信息。当前默认优先级为：`tracer_core=P0`、其他 libs 为 `P1`、presentation 为 `P2`、tests 为 `P3`。组件根目录中命中 `test`、`tests`、`androidTest` 或 `testFixtures` 的路径会归入 `tests`。

profile 模式暂不支持 `--dir-over-files`；目录密度扫描仍使用 `--lang` 模式。

当前默认 `over` 阈值为：C++ 350、Kotlin 350、Python 200、Rust 350。profile 未显式传入 `--over` 时，按文件语言使用对应阈值。

profile 模式的命令行只输出优先级、文件、行数和一句话上下文提示。模块阅读候选顺序只是建议，不代表重构顺序；完整报告会自动写入 workspace 根目录的 `temp/loc_scanner/reports/`：

- `profile_<name>.md`：完整扫描明细和对应 guidance 内容快照
- `profile_<name>.json`：结构化扫描结果、建议、guidance 路径和验证入口

扫描日志统一写入 `temp/loc_scanner/logs/`。

架构 guidance 的统一重构入口是
`docs/time_tracer/architecture/refactoring_guidance.md`；模块文档只补充
具体 ownership、contract 和 validation 细节。本地报告用于 agent 和开发者
按本次扫描结果集中阅读。

扫描结果必须按以下流程进入重构判断：

```text
LOC Scanner finds candidates
    ↓
Agent analyzes responsibilities, coupling, and reasons for change
    ↓
Confirm that a real boundary exists
    ↓
Split only by responsibility
    ↓
Check whether dependencies decreased and cohesion improved
```

扫描器不判断单一职责、业务规则重复、依赖方向、Runtime protocol 风险、测试
覆盖是否足够，或重构收益是否高于风险；这些判断由 Agent 基于代码、契约和测试
证据完成。拆分的目的应是降低职责混杂、明确 ownership、简化依赖，并提高内聚性
和独立测试能力，而不是单纯降低 LOC。

不要机械拆分：不要为每个方法或 CRUD 操作创建小类，不要只搬移方法而不改变职责
归属，不要因为文件较大就拆分已有高内聚代码，也不要为了降低 LOC 引入透传包装类、
重复状态、重复模型或第二套 Runtime protocol。

## Windows bat 入口

```bat
tools\devtools\loc_scanner\scripts\run_py.bat
tools\devtools\loc_scanner\scripts\run_kt.bat
tools\devtools\loc_scanner\scripts\run_cpp.bat
tools\devtools\loc_scanner\scripts\run_rs.bat
```

可追加参数透传给 `run.py`。

profile 模式入口位于 `scripts/profile/`：

```bat
tools\devtools\loc_scanner\scripts\profile\run_core_family.bat --over 300
tools\devtools\loc_scanner\scripts\profile\run_presentation.bat --over 300
tools\devtools\loc_scanner\scripts\profile\run_tests.bat --over 200
tools\devtools\loc_scanner\scripts\profile\run_workspace.bat --over 500
tools\devtools\loc_scanner\scripts\profile\run_tidy.bat --over 200
```

注意：若语言配置中 `path_mode = "toml_only"`，则命令行 `paths`（包括 bat 透传的位置参数）会被忽略，仅使用 TOML 里的 `default_paths`。

## 配置文件

默认配置文件：`tools/devtools/loc_scanner/config/scan_lines.toml`

配置字段说明：`tools/devtools/loc_scanner/docs/toml_config.md`


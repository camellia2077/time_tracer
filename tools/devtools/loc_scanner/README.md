# LOC Scanner

通用代码行数扫描工具，采用独立子项目目录组织。

This README covers scanner usage and result interpretation only. For the
refactoring decision standard, read
[`docs/time_tracer/architecture/refactoring_guidance.md`](../../../docs/time_tracer/architecture/refactoring_guidance.md).
The scanner implementation layout is only relevant when changing the scanner
itself; see [docs/architecture.md](docs/architecture.md).

## Agent Refactoring Entry Point

When changing LOC Scanner itself, use the local
[`AGENTS.md`](AGENTS.md). It defines the scanner-specific refactoring workflow,
required documents, decision rules, and validation commands. This README remains
the usage and result-interpretation guide.

## 快速使用

从仓库根目录执行：

```bash
python -m tools.devtools.loc_scanner --lang py --under
```

按架构组件聚合扫描：

```bash
# 扫描 core_family 中的 libs，结果会按 libs / tests 分类，并在组件下继续按语言展示
python -m tools.devtools.loc_scanner --profile core_family --over 300

# 同时扫描 core_family 和 presentation；tests 仍单独归类
python -m tools.devtools.loc_scanner --profile workspace --over 500

# 只扫描 Android；结果按 P2 presentation / P3 tests 输出
python -m tools.devtools.loc_scanner --profile android --over 350

# 只扫描 Windows Rust CLI，并输出 CLI ownership-zone 评估信号
python -m tools.devtools.loc_scanner --profile windows_cli --over 350

# 只扫描 tidy / clang 适配层中的 Python 代码
python -m tools.devtools.loc_scanner --profile tidy --over 200

# 分开查看 tests、tidy、clang adapters 和其他 toolchain glue
python -m tools.devtools.loc_scanner --profile python_tooling --over 200
```

对比重构前后的 profile JSON：

```bash
# 首次扫描时保存 baseline
python -m tools.devtools.loc_scanner --profile core_family --over 350 \
  --save-baseline temp/loc_scanner/baselines/core_family_before.json

# 重构后进行比较
python -m tools.devtools.loc_scanner --profile core_family --over 350 \
  --compare-baseline temp/loc_scanner/baselines/core_family_before.json
```

对比报告会写入 `temp/loc_scanner/reports/profile_<name>_delta.md` 和对应
JSON，展示新增热点、消失热点以及行数变化。baseline 必须是之前生成的
`profile_<name>.json`，避免只用当前行数判断重构是否有效。

Windows bat 入口：

```bat
tools\devtools\loc_scanner\scripts\profile\run_core_family.bat --over 300
tools\devtools\loc_scanner\scripts\profile\run_presentation.bat --over 300
tools\devtools\loc_scanner\scripts\profile\run_tests.bat --over 200
tools\devtools\loc_scanner\scripts\profile\run_workspace.bat --over 500
tools\devtools\loc_scanner\scripts\profile\run_loc_scanner.bat
```

`--lang` 和 `--profile` 是两种互斥模式：

- `--lang`：按单一语言扫描；`scripts/lang/` 提供对应的 bat 入口。
- `--profile`：按 TOML 中定义的组件聚合扫描所有语言；显式阈值对所有语言生效，未传阈值时使用各语言默认阈值。

`windows_cli` 是面向 `apps/cli` 的专项 profile，只扫描 Rust CLI，并额外报告跨
`src/cli`、`src/commands`、`src/core/runtime`、`src/error` ownership zone 的热点证据。
Rust `#[cfg(test)]` 内嵌测试会单独计入 test evidence，CLI test-support 文件也不会
混入 production 统计。评估信号用于决定阅读顺序和验证重点，不自动判定必须拆分。

`tidy` 和 `python_tooling` profile 通过 `languages = ["py"]` 限制为 Python；组件之间使用 `exclude_roots` 避免父级 toolchain glue 重复统计 tidy 和 clang adapters。

profile 模式按 `libs`、`presentation` 和 `tests` 分类，并在组件内部继续按语言展示。命令行只输出优先级、文件、行数和简要上下文提示；完整内容写入本地报告。
单语言模式也会按源码集分类：`src/main` 归入 `PRODUCTION`，`src/test`、`src/androidTest`、`test` 等测试目录归入 `TESTS`，其余路径归入 `OTHER`。分类会同时写入控制台和 JSON 日志。
profile 模式还会为每个组件生成模块级基线摘要：全量文件数、全量代码行数、生产/测试规模、源码目录数和 Top 文件；热点数量仍按当前阈值单独统计。
基线摘要会附加证据标签，例如 `LARGE_FILE`、`MANY_LARGE_FILES`、`MANY_SOURCE_FILES`、`TEST_HEAVY`、`TEST_LIGHT`、`HIGH_HOTSPOT_CONCENTRATION` 和 `OTHER_SOURCE_SET_PRESENT`，每个标签都会带触发原因，不代表扫描器已经完成架构判断。Rust profile 还会区分路径型 tests、`#[cfg(test)]` 内嵌测试和 test-support 文件。
profile 还会输出当前 profile 内的模块综合排序。分数由架构优先级（60%）、模块规模（20%）、热点数量（15%）和边界信号（5%）组成，并保留分项分数和排序原因。P0–P3 使用固定优先级基线，避免大模块规模直接压过核心边界。

报告和日志输出位置：

```text
temp/loc_scanner/reports/profile_<name>.md    # 完整扫描明细和 guidance 快照
temp/loc_scanner/reports/profile_<name>.json   # 结构化扫描结果和上下文提示
temp/loc_scanner/logs/scan_<name>.json        # 扫描日志
```

报告中的架构 guidance 统一指向
`docs/time_tracer/architecture/refactoring_guidance.md`；模块文档只补充
各自的 ownership、contract 和 validation 细节。本地 Markdown 只是本次
扫描的内容快照。

组件配置与语言配置分离。组件根目录内命中 `test`、`tests`、`androidTest` 或 `testFixtures` 目录的文件，会统一归入 `tests` 类别，不会混入 `libs` 或 `presentation`。

profile 模式会进一步按优先级输出结果：当前 `tracer_core` 为 P0，其他 libs 为 P1，presentation 为 P2，tests 为 P3。同一优先级内按代码行数从高到低排序。

当前 Python 默认大文件阈值为 200 行；C++、Kotlin、Rust 默认大文件阈值保持 350 行。

常用参数：

- `--lang`：`cpp | kt | py | rs`
- `--profile`：按配置中的组件 profile 聚合扫描，例如 `core_family`、`presentation`、`windows_cli`、`workspace`
- `paths`：可选，待扫描目录；默认可覆盖配置中的 `default_paths`（若该语言 `path_mode = "toml_only"`，则忽略命令行 `paths`）
- `--workspace-root`：相对路径解析根目录，默认当前目录
- `--config`：配置文件路径，默认 `tools/devtools/loc_scanner/config/scan_lines.toml`
- `--log-file`：日志输出路径；不传时写入 `<workspace-root>/temp/loc_scanner/logs/scan_<lang>.json`
- `--over N` / `--under [N]` / `--dir-over-files [N]`

Android profile 的快捷入口为 `scripts/profile/run_android.bat`，Windows CLI profile
的快捷入口为 `scripts/profile/run_windows_cli.bat`；LOC Scanner
自身 Python 工具的 profile 入口为 `scripts/profile/run_loc_scanner.bat`。

更多示例见：[docs/usage.md](docs/usage.md)，配置字段说明见：[docs/toml_config.md](docs/toml_config.md)


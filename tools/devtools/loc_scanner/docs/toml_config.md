# LOC Scanner TOML 配置说明

默认配置文件位置：`tools/devtools/loc_scanner/config/scan_lines.toml`

## 0. 组件与 profile

语言配置决定扩展名、忽略目录和阈值；组件配置决定代码属于哪个架构范围。两者相互独立。

```toml
[components.tracer_core]
display_name = "libs/tracer_core"
root = "libs/tracer_core"
category = "libs"
priority = 0
exclude_roots = []
test_roots = []

[profiles.core_family]
display_name = "Core Family"
components = ["tracer_core"]

[test_classification]
directory_names = ["test", "tests", "androidTest", "testFixtures"]
priority = 3
```

profile 扫描时，优先级输出层级为：

```text
priority -> matched files
```

每条结果仍保留 `category`、`component` 和 `language` 信息。同一优先级内按代码行数降序、路径升序排列。

优先级数字越小越高。组件通过 `components.<name>.priority` 配置；测试文件命中测试目录后使用 `test_classification.priority`，不继承组件优先级。

组件根目录内命中 `test_classification.directory_names` 的目录，会被归入 `tests` 类别，覆盖组件原本的 `libs` 或 `presentation` 类别。

profile 报告中的模块基线摘要统计组件根目录下全部可读代码文件；阈值只影响 `matched_files`/热点结果，不影响模块总文件数和总行数。

模块基线还会生成解释性标签及 `label_reasons`。标签来自固定统计规则：5 个以上热点文件为 `MANY_LARGE_FILES`，100 个以上源码文件为 `MANY_SOURCE_FILES`，已识别测试证据的文件或代码行占比达到 40% 为 `TEST_HEAVY`，低于 10% 为 `TEST_LIGHT`，热点行数占比达到 40% 为 `HIGH_HOTSPOT_CONCENTRATION`。Rust 的 `#[cfg(test)]` 内嵌测试会作为估算 evidence 计入，CLI 的 `commands/testing.rs` 会归入测试 support。这些标签用于缩小 Agent 的阅读范围，不直接表示架构缺陷。

同一 profile 还会生成 `module_reading_candidates`。阅读分数按架构优先级 60%、模块规模 20%、热点数量 15%、边界信号 5% 计算；P0–P3 使用固定优先级基线，并保留 `reading_score_breakdown` 与 `reasons`。该分数只用于当前 profile 内建议 Agent 先阅读哪些模块，不代表重构顺序或代码质量。

组件可配置 `exclude_roots`，用于避免父组件和子组件重复统计。排除路径按仓库根目录解析，扫描父组件时会跳过这些目录。profile 可配置 `languages`，例如 `languages = ["py"]`，只扫描指定语言；未配置时扫描全部支持语言。

组件还可配置 `test_roots`，用于声明位于组件根目录之外的配套测试目录。测试文件会计入该组件的测试统计，并标记为 `TESTS_EXTERNAL`，避免把外置测试误判为 `NO_TESTS`。该字段按仓库根目录解析。

当前 profile：

- `core_family`
- `presentation`
- `android`
- `windows_cli`
- `tests`
- `workspace`
- `python_tooling`
- `tidy`

当前默认 `over` 阈值为：C++ 350、Kotlin 350、Python 200、Rust 350。profile 未显式传入阈值时，使用对应语言的默认值。

## 1. 基本结构

配置按语言分段，每个语言一个 section：

```toml
[py]
display_name = "Python"
default_paths = ["."]
path_mode = "cli_override"
extensions = [".py", ".pyw"]
ignore_dirs = [".git", "__pycache__", "venv"]
ignore_prefixes = [".", "site-packages"]
default_over_threshold = 250
default_under_threshold = 120
default_dir_over_files = 10
over_inclusive = true
```

支持的语言 section：

- `[cpp]`
- `[kt]`
- `[py]`
- `[rs]`

## 2. 字段说明

- `display_name`:
  - 用途：报告标题显示名。
  - 类型：字符串。
- `default_paths`:
  - 用途：默认扫描路径。
  - 类型：字符串数组。
  - 规则：相对路径按 `--workspace-root` 解析。
- `path_mode`:
  - 用途：控制命令行 `paths` 与 `default_paths` 的关系。
  - 类型：字符串。
  - 可选值：
    - `cli_override`：有命令行 `paths` 时覆盖 `default_paths`（默认）。
    - `toml_only`：始终只使用 `default_paths`，忽略命令行 `paths`。
    - `merge`：先用 `default_paths`，再追加命令行 `paths`（自动去重）。
- `extensions`:
  - 用途：纳入统计的文件扩展名。
  - 类型：字符串数组。
  - 示例：`[".cpp", ".hpp"]`。
- `ignore_dirs`:
  - 用途：目录名精确匹配忽略。
  - 类型：字符串数组。
- `ignore_prefixes`:
  - 用途：目录名前缀匹配忽略（不区分大小写）。
  - 类型：字符串数组。
- `default_over_threshold`:
  - 用途：`over` 模式默认阈值（大文件阈值）。
  - 类型：正整数。
- `default_under_threshold`:
  - 用途：`under` 模式默认阈值（小文件阈值）。
  - 类型：正整数。
- `default_dir_over_files`:
  - 用途：`--dir-over-files` 不带值时的默认目录文件数阈值。
  - 类型：正整数。
- `over_inclusive`:
  - 用途：控制 `over` 判定是否包含等号。
  - 类型：布尔值。
  - `true` 表示 `>=`，`false` 表示 `>`。
- `exclude_roots`:
  - 用途：从当前组件中排除已由其他组件独立统计的子目录。
  - 类型：字符串数组。
- `profiles.<name>.languages`:
  - 用途：限制 profile 扫描的语言。
  - 类型：字符串数组；可选值为 `cpp`、`kt`、`py`、`rs`。

单语言扫描复用 `[test_classification]` 的 `directory_names`。此外，路径中相邻出现
`src/main` 时归入 `production`，不命中源码集规则的文件归入 `other`；测试目录规则优先于
`src/main`，避免测试 fixture 被误归类。

## 3. 常见配置场景

- 只扫描某个子目录：
  - 在目标语言的 `default_paths` 写入该目录，例如 `["apps/android"]`。
- 强制只能用 TOML 控制扫描目录（忽略 bat/CLI 的路径参数）：
  - 设置 `path_mode = "toml_only"`。
- 提高大文件告警门槛：
  - 调大 `default_over_threshold`。
- 只统计源码，不统计头文件：
  - 调整 `extensions`，移除不需要的扩展名。
- 避免扫描产物目录：
  - 在 `ignore_dirs` 加入 `build`、`out`、`target` 等目录名。

## 4. 常见错误

- `default_paths` 不是数组，或数组中有空字符串。
- `path_mode` 不是 `cli_override` / `toml_only` / `merge`。
- `default_*_threshold` 不是正整数。
- 漏写语言 section（例如没有 `[py]`）。
- 扩展名未带点（建议写成 `.py`、`.cpp` 形式）。

当配置不合法时，命令会输出 `[ERROR] 配置加载失败` 并返回非 0 状态码。

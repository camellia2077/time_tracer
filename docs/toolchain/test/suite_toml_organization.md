# Suite TOML Organization

本文档约定 `tools/suites/` 下 suite TOML 的分层职责、命名方式与推荐 include 关系。

## 1. 目标

1. 让 suite 入口一眼可读，先看聚合入口，再看 capability 入口，最后看实际命令表。
2. 让文件名表达职责，而不是表达底层 TOML table 细节。
3. 避免 `command_groups*.toml`、`commands_*.toml` 并存造成搜索和维护分叉。

## 2. 推荐分层

每个 suite 目录推荐保持三层：

1. 聚合入口层
   - `config.toml`
   - `tests.toml`
2. capability / profile 入口层
   - `config_cap_<capability>.toml`
   - `config_<profile>.toml`
3. 基础装配与职责命令层
   - `env.toml`
   - `tests/base*.toml`
   - `tests/commands_*.toml`

## 3. 各层职责

### `config.toml`

- suite 默认入口。
- 只负责拼装 `env.toml` 与 `tests.toml`，不直接承载大段命令定义。

### `tests.toml`

- suite 默认命令聚合入口。
- 只负责按执行顺序 include `tests/base*.toml` 与 `tests/commands_*.toml`。
- 不建议在这里内联大量 `[[commands]]` 或 `[[command_groups]]`。

### `config_cap_<capability>.toml`

- capability 子入口。
- 只拼 capability 需要的 `base` 与对应 `commands_<capability>.toml`。
- 用于快速执行某一职责面，例如 reporting、query、exchange。

### `config_<profile>.toml`

- profile / 场景子入口。
- 只表达 profile 差异，不重复复制整套命令表。
- 常见于 Android 这种会按 `style`、`ci`、`device` 切分验证入口的 suite。

### `env.toml`

- 只承载路径、环境、部署输出位置。
- 不混入业务命令。

### `tests/base*.toml`

- 只承载跨职责共享的基础装配。
- 例如 runtime bootstrap、公共准备步骤、统一前置检查。
- 如果不同 capability 的基础装配不同，可以保留 `base.toml`、`base_no_pipeline.toml` 这类变体。

### `tests/commands_*.toml`

- 这是职责命令层。
- 一个文件对应一个明确职责域，例如：
  - `commands_reporting.toml`
  - `commands_version.toml`
  - `commands_query_tree.toml`
  - `commands_query_data.toml`
  - `commands_failure_cli.toml`
  - `commands_failure_runtime.toml`
  - `commands_pipeline.toml`
  - `commands_generate.toml`
- 文件名统一使用 `commands_` 前缀，即使内部实际使用的是 `[[command_groups]]` table。
- 不再新建 `command_groups*.toml` 这类以底层 table 命名的文件。

## 4. 命名规则

1. 聚合入口：
   - `config.toml`
   - `tests.toml`
2. capability 入口：
   - `config_cap_<capability>.toml`
3. profile 入口：
   - `config_<profile>.toml`
4. base：
   - `tests/base.toml`
   - `tests/base_<variant>.toml`
5. 职责命令：
   - `tests/commands_<responsibility>.toml`

## 5. include 关系示例

默认 suite 入口：

```toml
includes = ["env.toml", "tests.toml"]
```

默认命令聚合入口：

```toml
includes = [
  "tests/base.toml",
  "tests/commands_reporting.toml",
  "tests/commands_query_data.toml",
]
```

capability 入口：

```toml
includes = [
  "env.toml",
  "tests/base.toml",
  "tests/commands_reporting.toml",
]
```

## 6. 现有仓库约定

当前仓库推荐这样理解：

1. `config.toml` / `tests.toml`
   - 默认聚合入口
2. `config_cap_*.toml` / `config_<profile>.toml`
   - capability 或 profile 子入口
3. `tests/base*.toml`
   - 基础装配层
4. `tests/commands_*.toml`
   - 按职责拆分的命令层

## 7. 迁移建议

1. 发现 `command_groups*.toml` 时，优先迁成 `commands_*.toml`。
2. 如果某个旧文件未被任何入口 include，优先删除，不保留同义副本。
3. 新增测试职责时，优先新增 `tests/commands_<responsibility>.toml`，再决定是否需要新的 `config_cap_<capability>.toml`。

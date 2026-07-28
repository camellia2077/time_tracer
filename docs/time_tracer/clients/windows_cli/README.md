# Windows CLI 文档

本文档按主命令组织 Windows Rust CLI。先看下面的命令索引，再进入对应主命令
文档查看用途、数据影响范围和示例。

## 命令索引

| 主命令 | 用途 | 文档 |
|---|---|---|
| `alias` | 编辑 alias TOML、查看层级、迁移 canonical | [alias.md](commands/alias.md) |
| `pipeline` | TXT/config 校验、转换、导入、入库 | [pipeline.md](commands/pipeline.md) |
| `query` | 查询数据库数据和树 | [query.md](commands/query.md) |
| `report` | 生成报告、导出结果、生成图表 | [report.md](commands/report.md) |
| `exchange` | tracer 数据包导入、导出、检查 | [exchange.md](commands/exchange.md) |
| `txt` | 查看和编辑 TXT 日块 | [txt.md](commands/txt.md) |
| `system` | 检查 CLI/Core 运行环境 | [system.md](commands/system.md) |
| `about` | 查看版本、许可和项目信息 | [about.md](commands/about.md) |

## 全局参数

- `--db <PATH>`：覆盖数据库路径。
- `--output <PATH>`：覆盖输出路径；部分导出命令要求提供。
- `--version`：显示版本。

```powershell
time_tracer_cli --db <db_path> <command> <subcommand> [options]
```

## 实现入口

- 命令模型：`apps/cli/windows/rust/src/cli/`
- 命令路由：`apps/cli/windows/rust/src/commands/`
- Core runtime host：`apps/cli/windows/rust/src/core/runtime/`

## 数据边界

CLI 负责参数解析、文件操作和命令编排；Core 负责 TXT 语义、alias/config 校验、
canonical 解析、入库和报告业务逻辑。具体命令的 TOML、TXT、DB 修改范围见各命令文档。

## 兼容说明

已移除的参数或命令包括：`--database`、`--out`、`--project`、`blink`、`zen`、
`remark-day`、`sensitive`。

## Validation

```powershell
python tools/run.py verify --app tracer_core --concise
```

如需显式构建 Windows runtime 和 Rust CLI：

```powershell
python tools/run.py build --app tracer_core --profile release_bundle --build-dir build --runtime-platform windows
python tools/run.py build --app tracer_windows_rust_cli --profile release_bundle --build-dir build --runtime-platform windows
```

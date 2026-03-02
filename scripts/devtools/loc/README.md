# LOC Scanner (for Agent)

本目录提供统一的代码行数扫描工具，入口只有一个：

- `python scripts/devtools/loc/run.py`

## 1. 基本用法

从仓库根目录执行：

```bash
python scripts/devtools/loc/run.py --lang <cpp|kt|py|rs> [paths ...] [--over N | --under [N] | --dir-over-files [N]] [--dir-max-depth N] [--log-file <path>]
```

参数说明：

- `--lang`：语言类型，必填，支持 `cpp` / `kt` / `py` / `rs`。
- `paths`：可选，扫描目录列表，支持相对路径和绝对路径。
- `--over N`：扫描“大文件”。
- `--under [N]`：扫描“小文件”；不传 `N` 时使用 TOML 里的默认小文件阈值。
- `--dir-over-files [N]`：扫描“目录内代码文件数超过 N”的目录；不传 `N` 时使用 TOML 的 `default_dir_over_files`。
- `--dir-max-depth N`：目录扫描最大深度（相对输入根目录，`0` 表示仅根目录）；仅对 `--dir-over-files` 生效。
- `-t/--threshold N`：兼容参数，等价 `--over N`。
- `--log-file`：可选，自定义日志文件路径（相对路径相对仓库根目录）。
- `--config`：可选，指定配置文件路径，默认 `scripts/devtools/loc/scan_lines.toml`。

## 2. 默认行为

如果不传 `paths`，会自动读取 `scan_lines.toml` 中该语言的 `default_paths`：

- `cpp` -> `apps/tracer_core`
- `kt` -> `apps/tracer_android`
- `py` -> `test`, `scripts`
- `rs` -> `apps/tracer_cli/windows/rust_cli/src`

目录文件数扫描默认阈值（`--dir-over-files` 不带 `N` 时生效）：

- `cpp` -> `20`（包含声明与实现文件，按 `extensions` 统计）
- `kt` -> `15`
- `py` -> `10`
- `rs` -> `12`

如果传多个路径，输出会按路径分段打印（每个路径一个 `[SCAN]` 区块）。

## 3. 日志输出

每次执行都会写日志，便于 agent 回看完整输出。

默认日志路径：

- `scripts/devtools/loc/logs/scan_cpp.json`
- `scripts/devtools/loc/logs/scan_kt.json`
- `scripts/devtools/loc/logs/scan_py.json`
- `scripts/devtools/loc/logs/scan_rs.json`

可用 `--log-file` 覆盖，例如：

```bash
python scripts/devtools/loc/run.py --lang py --under 120 --log-file temp/loc_scan_py.json
```

## 4. 常用命令模板（Agent）

扫描 Python 大文件（默认路径）：

```bash
python scripts/devtools/loc/run.py --lang py --over 200
```

扫描 Kotlin 小文件（默认小文件阈值）：

```bash
python scripts/devtools/loc/run.py --lang kt --under
```

扫描 C++，指定绝对路径：

```bash
python scripts/devtools/loc/run.py --lang cpp "C:/abs/path/to/project" --over 300
```

扫描多个路径（分开打印）：

```bash
python scripts/devtools/loc/run.py --lang py test scripts --under 80
```

扫描目录中文件过多的热点目录（按 Python 扩展名过滤）：

```bash
python scripts/devtools/loc/run.py --lang py --dir-over-files 25 --dir-max-depth 2
```

扫描目录中文件过多的热点目录（使用语言默认阈值）：

```bash
python scripts/devtools/loc/run.py --lang py --dir-over-files
```

Windows 快捷入口（bat 与 `run.py` 同目录）：

```bat
scripts\devtools\loc\run_py.bat
scripts\devtools\loc\run_kt.bat
scripts\devtools\loc\run_cpp.bat
scripts\devtools\loc\run_rs.bat
```

可追加参数透传给 `run.py`，例如：

```bat
scripts\devtools\loc\run_py.bat --dir-over-files --dir-max-depth 2
```

目录文件数量扫描专用 bat（默认使用 `--dir-over-files`）：

```bat
scripts\devtools\loc\run_py_files.bat
scripts\devtools\loc\run_kt_files.bat
scripts\devtools\loc\run_cpp_files.bat
scripts\devtools\loc\run_rs_files.bat
```

## 5. 配置文件

配置文件：`scripts/devtools/loc/scan_lines.toml`

建议只在这里维护：

- 默认扫描路径（`default_paths`）
- 文件扩展名（`extensions`）
- 忽略目录和前缀（`ignore_dirs` / `ignore_prefixes`）
- 默认阈值（`default_over_threshold` / `default_under_threshold`）
- 目录文件阈值（`default_dir_over_files`）
- `over_inclusive`（`over` 是否按 `>=`）
- 目录扫描也复用 `extensions`、`ignore_dirs`、`ignore_prefixes`

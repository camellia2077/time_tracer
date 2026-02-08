# CLI Export Guide (export)

用于导出报表（`md/tex/typ`）。

---

## 用法

```
time_tracker_cli.exe export <type> [argument] [options]
```

## type 说明

需要 argument：
- `day`：单日
- `month`：单月
- `week`：单周（ISO 周）
- `year`：单年
- `recent`：最近 N 天
- `all-recent`：多个最近区间（列表）

不需要 argument：
- `all-day` / `all-month` / `all-week` / `all-year`

## argument 格式

- `day`：`YYYYMMDD`
- `month`：`YYYYMM`
- `week`：`YYYY-Www`（例如 `2026-W05`）
- `year`：`YYYY`
- `recent`：`N`
- `all-recent`：`N1,N2,...`（例如 `7,30,90`）

## 选项

- `-f` / `--format`：输出格式，支持 `md,tex,typ`（逗号分隔）。
- `-o` / `--output`：导出目录。
- `--db` / `--database`：数据库路径。

## 示例

```
time_tracker_cli.exe export day 20260101 --format md
time_tracker_cli.exe export month 202601 --format md,typ
time_tracker_cli.exe export week 2026-W05 --format tex
time_tracker_cli.exe export recent 7 --format md
time_tracker_cli.exe export all-recent 7,30 --format md
time_tracker_cli.exe export all-month --format md
```

## 全局选项

- `-h` / `--help`
- `-v` / `--version`

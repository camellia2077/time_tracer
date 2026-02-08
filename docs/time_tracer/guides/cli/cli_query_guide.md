# CLI Query Guide (query)

本指南覆盖 `query` 命令的全部功能：**报表查询**与**数据查询**。

---

## 1. 命令总览

```
time_tracker_cli.exe query <type> <argument> [options]
```

`type` 支持：
- 报表查询：`day` / `month` / `week` / `year` / `recent`
- 数据查询：`data`

---

## 2. 报表查询 (day/month/week/year/recent)

用于生成日报/月报/周报/年报/最近 N 天报表（md/tex/typ）。

### 2.1 格式选项
- `-f` / `--format`：`md` / `tex` / `typ`，支持逗号分隔组合。

示例：
```
time_tracker_cli.exe query day 20260101 --format md,typ
time_tracker_cli.exe query month 202601 --format md
```

### 2.2 参数格式
- `day`: `YYYYMMDD`（自动规范化为 `YYYY-MM-DD`）
- `month`: `YYYYMM`
- `week`: ISO 周格式 `YYYY-Www`（例如 `2026-W05`）
- `year`: `YYYY`
- `recent`: 数字列表（例如 `7` / `7,30`）

示例：
```
time_tracker_cli.exe query day 20260101
time_tracker_cli.exe query week 2026-W05
time_tracker_cli.exe query year 2026
time_tracker_cli.exe query recent 7
```

---

## 3. 数据查询 (query data ...)

数据查询面向数据库（days/time_records），用于筛选日期集合或统计。

### 3.1 列表类命令

- 年份列表：
  `time_tracker_cli.exe query data years`
- 月份列表：
  `time_tracker_cli.exe query data months --year 2021`
- 日期列表：
  `time_tracker_cli.exe query data days --year 2021 --month 1`

### 3.2 按日时长排序

- 按每日总时长排序（默认升序）：
  `time_tracker_cli.exe query data days-duration --from 202101 --to 202101`
- 降序：
  `time_tracker_cli.exe query data days-duration --from 202101 --to 202101 -r`

### 3.3 日统计 (均值/方差/分位数等)

```
time_tracker_cli.exe query data days-stats --from 202101 --to 202101
```

可选输出 Top N（最长/最短）：
```
time_tracker_cli.exe query data days-stats --from 202101 --to 202101 --top 5
```

> `days-stats` 忽略 `-n/--numbers`，因为统计需要完整数据集。

---

## 4. 过滤条件 (Filters)

- 日备注：
  `--day-remark` / `--remark-day`
- 活动备注：
  `--remark`
- 项目路径（前缀匹配）：
  `--project`
- 通宵：
  `--overnight`
- 运动标记：
  `--exercise 0|1`
- 状态标记：
  `--status 0|1`

示例：
```
time_tracker_cli.exe query data days --day-remark overwatch
time_tracker_cli.exe query data days --remark 备注
time_tracker_cli.exe query data days --project sleep_night
time_tracker_cli.exe query data days --year 2021 --remark 自助 --exercise 1
```

---

## 5. 时间范围 (Date Range)

`--from/--to` 支持：
- `YYYY`（全年度）
- `YYYYMM`（整月）
- `YYYYMMDD`（单日或区间）

示例：
```
time_tracker_cli.exe query data days --from 2026 --to 2026
time_tracker_cli.exe query data days --from 202605 --to 202605
time_tracker_cli.exe query data days --from 20260101 --to 20260131
```

---

## 6. 输出控制 (Output Control)

- 限制条数：`-n` / `--numbers`
- 逆序：`-r` / `--reverse`
  - 对 `days-duration`：按“时长降序”输出
  - 对 `days`/`search`：按“日期降序”输出

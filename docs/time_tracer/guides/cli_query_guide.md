# CLI Query Guide (query data)

## List

- List years:
  `time_tracker_cli.exe query data years`
- List months (optional year filter):
  `time_tracker_cli.exe query data months --year 2021`
- List days (optional year/month filter):
  `time_tracker_cli.exe query data days --year 2021 --month 1`

## Filters

- Day remark:
  `time_tracker_cli.exe query data days --day-remark alpha`
  `time_tracker_cli.exe query data days --remark-day overwatch`
- Activity remark:
  `time_tracker_cli.exe query data days --remark banana`
  `time_tracker_cli.exe query data days --remark 备注`
- Project path (mapping result, supports partial match):
  `time_tracker_cli.exe query data days --project sleep_night`
- Overnight (getup_time is null/empty/00:00):
  `time_tracker_cli.exe query data days --overnight`
- Exercise flag:
  `time_tracker_cli.exe query data days --exercise 1`
- Status flag:
  `time_tracker_cli.exe query data days --status 0`
- Combined filters:
  `time_tracker_cli.exe query data days --year 2021 --remark 自助 --exercise 1`

## Date Range

`--from/--to` accept YYYY, YYYYMM, YYYYMMDD.

- `--from 2026` means 2026-01-01
- `--to 2026` means 2026-12-31

Example: query 2026 (full year) day-remark contains "overwatch", reverse order, limit 10.

```
time_tracker_cli.exe query data days --from 2026 --to 2026 --day-remark overwatch -r -n 10
```

Example: query 2026-05 days with exercise=1.

```
time_tracker_cli.exe query data days --from 202605 --to 202605 --exercise 1
```

Example: list all days in 2021-01 (ascending).

```
time_tracker_cli.exe query data days --from 202101 --to 202101
```

## Output Control

- Limit count: `-n` / `--numbers`
- Reverse order: `-r` / `--reverse`

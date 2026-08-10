# `insights`

生成文本报告、导出报告结果和图表数据。

## 子命令

- `insights render`：直接渲染报告。
- `insights export`：将报告导出到指定目录。
- `insights chart`：生成 line、bar、pie、heatmap 等图表结果。

```powershell
time_tracer_cli --db <db_path> insights render recent 7 --as-of 2026-03-07 --format md
time_tracer_cli --db <db_path> --output <out_dir> insights export recent 7 --format md
```

`recent` 支持 `--as-of YYYY-MM-DD`。`pie` 表示整个时间段的 root composition，
不接受 `--root`；`line`、`bar` 和 `heatmap-*` 使用趋势/日序列报告数据。

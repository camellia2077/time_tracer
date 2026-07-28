# `txt`

查看和编辑共享的月度 TXT 文件。

## 子命令

- `txt view-day`：查看指定日期的 day block，保留 point/interval 原文。
- `txt append-event`：向指定日期追加一个 authored event。

```powershell
time_tracer_cli txt view-day --in <month_txt> --day 0101
time_tracer_cli txt append-event --in <month_txt> --day 0101 --time 0900 --activity 学习
```

Core 支持的活动行包括 `HHMMtoken` 和 `HHMM-HHMMtoken`。TXT 中的活动 token 必须能通过当前 alias 配置解析。

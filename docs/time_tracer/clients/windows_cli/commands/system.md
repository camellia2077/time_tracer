# `system`

检查 CLI、Core runtime、配置和数据库路径是否可用。

## 子命令

```powershell
time_tracer_cli --db <db_path> system doctor
```

`doctor` 适合在导入、入库或查询失败时确认运行时 DLL、配置目录、数据库文件和输出目录。
该命令主要用于诊断，不负责修复业务数据。

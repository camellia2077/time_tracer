# `query`

从数据库查询活动数据或树形结构。

## 子命令

- `query data`：查询原始或聚合活动数据。
- `query tree`：按 weighted tree 形式查看活动层级与秒数。

```powershell
time_tracer_cli --db <db_path> query data [options]
time_tracer_cli --db <db_path> query tree [options]
```

该主命令只读数据库，不修改 TXT、TOML 或数据库。

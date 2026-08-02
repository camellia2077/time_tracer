# `activity`

管理可记录的 leaf activity 合并及其历史 TXT、数据库迁移。

## 合并 activity

```powershell
time_tracer_cli --db <db_path> activity merge `
  --file config/activity_hierarchy/exercise.toml `
  --from cardio.running.treadmill `
  --into cardio.running.track-running `
  --input <txt_dir>
```

`activity merge` 只支持将一个非 group 的 leaf activity 合并到另一个非 group
leaf。Core 会删除 source activity 及其 aliases，保留目标 activity，并生成
canonical/alias replacement；CLI 使用 replacement 替换 TXT 中旧的 canonical 和
aliases，随后重建数据库。候选数据库成功后才替换原数据库。

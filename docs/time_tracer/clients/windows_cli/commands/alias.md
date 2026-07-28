# `alias`

管理 alias TOML 的 canonical 层级和 alias，用于配置编辑、层级查看，以及在需要时同步旧 TXT 和数据库。

## 子命令

| 子命令 | 用途 | TOML | TXT | DB |
|---|---|---:|---:|---:|
| `add` | 给 canonical leaf 添加 alias | 改 | 不改 | 不改 |
| `promote` | 把 leaf 提升为 group | 改 | 不改 | 不改 |
| `tree` | 输出基础树或带 alias 树 | 只读 | 不改 | 不读 |
| `move-config` | 只移动 TOML 中的 canonical leaf | 改 | 不改 | 不改 |
| `move` | 移动并同步旧 canonical | 改 | 改 | 重建 |
| `rename-group` | 重命名 group 并同步所有后代 canonical | 改 | 改 | 重建 |
| `add-group-alias` | 添加 group alias | 改 | 不改 | 不改 |
| `rename-group-alias` | 重命名 group alias | 改 | 不改 | 不改 |

## 查看层级

```powershell
time_tracer_cli alias tree --file config/aliases/study.toml
time_tracer_cli alias tree --file config/aliases/study.toml --show-aliases
```

默认只显示 canonical 树；`--show-aliases` 会在节点旁显示 `aliases` 和
`group_aliases`。输出是 plaintext，不会转换或重写 TOML。

## 只改 TOML

```powershell
time_tracer_cli alias move-config `
  --file config/aliases/study.toml `
  --alias 二重积分 `
  --to math.calculus.multiple-integral
```

该命令适合先整理配置，TXT 中的旧 canonical 不会被自动修改。

## 同步 TXT 和数据库

```powershell
time_tracer_cli --db <db_path> alias move `
  --file config/aliases/study.toml `
  --alias 二重积分 `
  --to math.calculus.multiple-integral `
  --input <txt_dir>
```

`alias move` 会修改 TOML，替换 TXT 中的旧 canonical，并通过 Core 重建候选数据库；
候选数据库成功后才替换原数据库。`--input` 是 TXT 根目录，`--db` 是全局数据库路径覆盖。

重命名 group 使用 `rename-group`。它会同时迁移当前 group、嵌套 group 和叶子活动
的 canonical；`--input` 指定 TXT 根目录，`--db` 指定要替换的数据库路径：

```powershell
time_tracer_cli --db <db_path> alias rename-group `
  --file config/aliases/exercise.toml `
  --group cardio `
  --name conditioning `
  --input test/data
```

## 其他编辑命令

```powershell
time_tracer_cli alias add --file <alias_toml> --alias <alias> --canonical <canonical>
time_tracer_cli alias promote --file <alias_toml> --alias <alias>
time_tracer_cli alias add-group-alias --file <alias_toml> --group <group> --alias <group_alias>
time_tracer_cli alias rename-group-alias --file <alias_toml> --group <group> --old-alias <old> --alias <new>
```

这些命令只修改 alias TOML；canonical 路径不变时，不需要迁移 TXT 或数据库。

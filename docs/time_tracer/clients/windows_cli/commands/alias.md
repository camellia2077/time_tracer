# `alias`

管理 alias TOML 的 canonical 层级和 alias，用于配置编辑、层级查看，以及在需要时同步旧 TXT 和数据库。

## 子命令

| 子命令 | 用途 | TOML | TXT | DB |
|---|---|---:|---:|---:|
| `add` | 给 canonical leaf 添加 alias | 改 | 不改 | 不改 |
| `promote` | 把 leaf 提升为 group | 改 | 不改 | 不改 |
| `tree` | 输出基础树或带 alias 树 | 只读 | 不改 | 不读 |
| `move-config` | 移动 TOML 中的 canonical leaf，可跨 TOML | 改 | 不改 | 不改 |
| `move` | 移动并同步旧 canonical，可跨 TOML | 改 | 改 | 重建 |
| `rename-group` | 重命名 group 并同步所有后代 canonical | 改 | 改 | 重建 |
| `rename-parent` | 同步重命名 TOML 文件、parent 和所有 canonical | 改名+改 | 改 | 重建 |
| `add-group-alias` | 添加 group alias | 改 | 不改 | 不改 |
| `rename-group-alias` | 重命名 group alias | 改 | 不改 | 不改 |

## 查看层级

```powershell
time_tracer_cli alias tree --file config/activity_hierarchy/study.toml
time_tracer_cli alias tree --file config/activity_hierarchy/study.toml --show-aliases
```

默认只显示 canonical 树；`--show-aliases` 会在节点旁显示 `aliases` 和
`group_aliases`。输出是 plaintext，不会转换或重写 TOML。

tree 先调用 Core 的 describe_activity_hierarchy 获取统一的 ActivityHierarchyTree，
再由 CLI 负责字符树展示。Core 模型中的每个可选节点都有 canonical_key、
相对于 [aliases] 的 canonical path、kind、aliases 和递归 children；是否显示
alias 只影响 CLI 展示，不改变模型。

## 只改 TOML

```powershell
time_tracer_cli alias move-config `
  --file config/activity_hierarchy/study.toml `
  --to-file config/activity_hierarchy/meal.toml `
  --alias 二重积分 `
  --to root
```

`--to-file` 存在时，Core 会读取 alias 目录中的文档集合，校验跨文件 alias
唯一性，并同时返回/提交 source 和 destination 两个 TOML。`--to` 可以是
`root` 或目标文件中已有的 group 路径。省略 `--to-file` 时保留同一 TOML 内
移动的兼容行为。该命令适合先整理配置，TXT 中的旧 canonical 不会被自动修改。

## 同步 TXT 和数据库

```powershell
time_tracer_cli --db <db_path> alias move `
  --file config/activity_hierarchy/study.toml `
  --to-file config/activity_hierarchy/meal.toml `
  --alias 二重积分 `
  --to root `
  --input <txt_dir>
```

`alias move` 会通过 Core 生成 source/destination TOML 和 canonical replacements，
再替换 TXT 中的旧 canonical，并通过 Core 重建候选数据库；候选数据库成功后才
替换原数据库。`--input` 是 TXT 根目录，`--db` 是全局数据库路径覆盖。

移动完整 group 子树：

```powershell
time_tracer_cli --db <db_path> alias move `
  --file config/activity_hierarchy/exercise.toml `
  --to-file config/activity_hierarchy/meal.toml `
  --group cardio.running `
  --to root `
  --input <txt_dir>
```

重命名 group 使用 `rename-group`。它会同时迁移当前 group、嵌套 group 和叶子活动
的 canonical；`--input` 指定 TXT 根目录，`--db` 指定要替换的数据库路径：

```powershell
time_tracer_cli --db <db_path> alias rename-group `
  --file config/activity_hierarchy/exercise.toml `
  --group cardio `
  --name conditioning `
  --input test/data
```

重命名活动层级 parent 使用 `rename-parent`。TOML 文件名 stem 和顶层
`parent` 被视为同一个值，因此命令会把 `<old-parent>.toml` 改为
`<new-parent>.toml`，同步更新 `meta/bundle.toml` 中的 required file 引用，
替换所有 TXT 中的 canonical，并在候选数据库成功后提交整个迁移：

```powershell
time_tracer_cli --db <db_path> alias rename-parent `
  --file config/activity_hierarchy/exercise.toml `
  --name training `
  --input test/data
```

目标 TOML 已存在、parent 名称不安全或候选构建失败时，旧 TOML 路径、TOML
内容、TXT 和数据库都会保留/恢复。`_system.toml` 不允许重命名。

## 其他编辑命令

```powershell
time_tracer_cli alias add --file <alias_toml> --alias <alias> --canonical <canonical>
time_tracer_cli alias promote --file <alias_toml> --alias <alias>
time_tracer_cli alias add-group-alias --file <alias_toml> --group <group> --alias <group_alias>
time_tracer_cli alias rename-group-alias --file <alias_toml> --group <group> --old-alias <old> --alias <new>
```

这些命令只修改 alias TOML；canonical 路径不变时，不需要迁移 TXT 或数据库。

当前 `move`/`move-config` 支持移动 leaf activity 或完整 group 子树。使用
`--alias` 移动 leaf，使用 `--group` 移动 group；group 的 nested groups、
group aliases 和 leaf aliases 会随子树一起移动。group 移动必须提供
`--to-file`，目标可以是 `root` 或目标文件中已有的 group。

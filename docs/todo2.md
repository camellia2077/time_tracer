# Tree 命令想法评估

## 可行性
可行。当前数据库里有 `projects` 表（包含 `id/name/parent_id`），而且已有递归路径拼接逻辑（`query-data --project` 里使用的 CTE）。因此新增一个 `tree` 命令，遍历任意层级并输出树形结构是可实现的。

## 当前程序如何做到
最小可行路径：
1. 新增 CLI 命令 `tree`（或 `query-tree`），放在 `cli/impl/commands/`。
2. 在命令中通过 SQLite 递归 CTE 拉平项目树，按层级排序输出。
3. 支持常用参数：
   - `--root <path>`：从某个项目路径开始输出子树（如 `study_math`）。
   - `--depth N`：限制输出深度。
   - `--contains <keyword>`：过滤名称或路径包含关键词的节点。
   - `--format`：`text/json`（可选）。

参考 SQL（递归 CTE）：
```
WITH RECURSIVE project_tree(id, name, parent_id, depth, path) AS (
  SELECT id, name, parent_id, 0 AS depth, name AS path
  FROM projects
  WHERE parent_id IS NULL
  UNION ALL
  SELECT p.id, p.name, p.parent_id, pt.depth + 1,
         pt.path || '_' || p.name
  FROM projects p
  JOIN project_tree pt ON p.parent_id = pt.id
)
SELECT id, name, depth, path FROM project_tree ORDER BY path;
```

## 输出示例
```
study
  study_math
    study_math_calculus
      study_math_calculus_double-integral
    study_math_linear-algebra
```

## 价值
- 快速查看“映射后项目树结构”，对维护 `mapping_config.toml` 和查询路径更直观。
- 与 `query-data --project` 搭配，用 `--root` 或 `--contains` 快速定位路径。

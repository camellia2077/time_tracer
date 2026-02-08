# CLI 命令总览

本页汇总所有 CLI 命令，并链接到对应的独立说明。

---

## Query

- `query`：查询统计（day/month/week/year/recent/data）
  - 说明文档：`cli_query_guide.md`

## Export

- `export`：导出报表（day/month/week/year/recent/all-*）
  - 说明文档：`cli_export_guide.md`

## Pipeline

- `convert`：转换 TXT 为 JSON（不入库）
  - 说明文档：`cli_convert_guide.md`
- `import`：导入 JSON 到数据库
  - 说明文档：`cli_import_guide.md`
- `ingest`：完整流水线（结构验证 -> 转换 -> 逻辑验证 -> 入库）
  - 说明文档：`cli_ingest_guide.md`
- `validate-logic`：只做逻辑校验（不保存、不入库）
  - 说明文档：`cli_validate_logic_guide.md`
- `validate-structure`：只做结构校验（TXT 语法）
  - 说明文档：`cli_validate_structure_guide.md`

## General

- `tree`：项目树展示
  - 说明文档：`cli_tree_guide.md`

---

## 全局选项

- `-h` / `--help`
- `-v` / `--version`

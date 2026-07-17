# Toolchain History Index

本目录记录 `tools/`、`tools/tests/`、`tools/test_framework/`、suite 资产迁移与相关
Python 工程工作流的演进历史。

## 口径

1. 同一天只保留一份日期文件。
2. `test/framework/**`、`test/run.py`、`test/suites/**` 等历史迁移也并入
   `docs/toolchain/history/`，不再单独维护 `test` 侧历史目录。
   这些旧路径仅用于描述当时的迁移背景，不代表当前仓库入口。
3. 写作格式遵循：
   - `.agent/guides/docs/history-style-guide-python.md`

## 当前日期文件

1. `2026-04-11.md`
   - suite / test framework 从 `test/` 迁到 `tools/`
2. `2026-04-04.md`
3. `2026-03-26.md`
   - 根测试索引与 agent 导航收口
4. `2026-03-24.md`
   - `validate` / `verify` / `tidy` 重构与 `test/framework/core` 扁平化

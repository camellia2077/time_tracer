# Toolchain Docs Index

`docs/toolchain/` 是 Python 工具链、测试契约与工程执行规则的权威文档目录。

根目录 `tools/` 保留薄层 `README.md` / `AGENTS.md` 索引；`test/` 只保留共享测试资产目录，详细说明统一下沉到本目录。

## 主文档

1. [tools/README.md](tools/README.md)
   - `tools/run.py`、`tools/test.py`、`tools/lint_suites.py` 入口，以及
     `tools/` / `test/` 当前目录职责索引
2. [docs/toolchain/test/README.md](test/README.md)
   - `verify` / `validate` / suite / result contract 与 `tools/` / `test/`
     分工入口

## 专项文档

1. [command_map/README.md](command_map/README.md)
   - 工具链命令入口与修改路由导航
2. [validate/README.md](validate/README.md)
   - `validate --plan` 的 TOML 结构和结果契约入口
3. [tidy/README.md](tidy/README.md)
   - clang-tidy 文件架构、工作流、SOP 与 auto-fix 规则入口
4. [workflows/README.md](workflows/README.md)
   - 跨命令执行流程与节奏约定
5. [notes/README.md](notes/README.md)
   - 补充说明、命令备忘与迁移笔记
6. [history/README.md](history/README.md)
   - toolchain 相关文档与流程演进历史

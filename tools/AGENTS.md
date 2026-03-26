# AGENTS Guide (tools)

本文件只保留 `tools/` 范围的阅读顺序与索引；详细规则统一维护在 `docs/toolchain/`。

## 先读哪些文档

1. 总索引：
   - `docs/toolchain/README.md`
2. 工具链主文档：
   - `docs/toolchain/tools/README.md`
3. 测试与结果契约：
   - `docs/toolchain/test/README.md`
4. 命令改动定位：
   - `docs/toolchain/command_map/README.md`
5. clang-tidy 改动：
   - `docs/toolchain/tidy/README.md`
6. workflow 约定：
   - `docs/toolchain/workflows/README.md`
7. 补充 notes：
   - `docs/toolchain/notes/README.md`
8. 演进 history：
   - `docs/toolchain/history/README.md`

## 工作约束

1. `tools/run.py` 仍是官方入口。
2. 改动影响协议、状态文件、结果契约时，同步更新 `docs/toolchain/` 文档。
3. 不要把 `tools/README.md`、`tools/AGENTS.md` 扩写回厚文档；新增细节优先写到 `docs/toolchain/`。

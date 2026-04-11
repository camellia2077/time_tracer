# AGENTS Guide (tools)

本文件只保留 `tools/` 范围的阅读顺序、目录边界与工作约束；
详细规则统一维护在 `docs/toolchain/`。

## 先读哪些文档

1. 总索引：
   - `docs/toolchain/README.md`
2. 工具链主文档：
   - `docs/toolchain/tools/README.md`
3. 测试与结果契约：
   - `docs/toolchain/test/README.md`
4. 测试分层约定：
   - `docs/toolchain/test/test_layering.md`
5. 命令改动定位：
   - `docs/toolchain/command_map/README.md`
6. validate：
   - `docs/toolchain/validate/README.md`
7. workflow：
   - `docs/toolchain/workflows/README.md`
8. clang-tidy：
   - `docs/toolchain/tidy/README.md`
9. suite 资产层：
   - `tools/suites/README.md`
10. 工具链历史：
   - `docs/toolchain/history/README.md`

## 目录边界

1. `tools/run.py`
   - build / verify / validate / tidy 官方入口
2. `tools/test.py`
   - suite / runtime-guard 官方入口
3. `tools/lint_suites.py`
   - suite schema lint 官方入口
4. `tools/suites/**`
   - suite 定义、command matrix、suite-local 脚本
5. `tools/test_framework/**`
   - test runner / suite orchestration / runtime guard / suite config loader
6. `tools/tests/**`
   - 工具链自身测试，不是产品黑盒 suite
7. `test/**`
   - 只保留共享 data、fixtures、golden 等测试资产；不再承载 suite 或 runner 实现

## 改动路由

1. 改 `build / verify / validate / tidy` 命令：
   - 优先看 `tools/toolchain/**`
2. 改 suite 入口、runtime-guard、suite output contract：
   - 优先看 `tools/test.py`、`tools/lint_suites.py`、`tools/test_framework/**`
3. 改 suite TOML、命令矩阵、suite-local 脚本：
   - 优先看 `tools/suites/**`
4. 改共享输入/基线：
   - 优先看 `test/data/**`、`test/golden/**`
   - 小型专用 fixture 看 `test/fixtures/**`
5. 改测试目录职责边界：
   - 先看 `docs/toolchain/test/test_layering.md`

## 工作约束

1. 不要把 `tools/suites/**` 或 `tools/test_framework/**` 再放回 `test/`。
2. 改动影响 suite 入口、result contract、runner 行为时，同步更新：
   - `docs/toolchain/test/README.md`
   - `tools/suites/README.md`
   - 需要时更新 `docs/toolchain/test/test_layering.md`
3. 改动影响工具链入口或目录职责时，同步更新：
   - `tools/README.md`
   - `docs/toolchain/README.md`
4. 不要把 `tools/README.md`、`tools/AGENTS.md` 扩写回厚文档；新增细节优先写到
   `docs/toolchain/`。

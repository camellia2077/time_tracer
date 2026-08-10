# Tools Index

`tools/` 现在承载三类内容：

1. 项目官方工具链入口
2. suite 资产
3. 测试工具链实现

共享测试数据、fixtures 与 golden 仍留在 `test/`。
根目录 `test/README.md` 保持薄索引，详细规则统一下沉到
`docs/tools/toolchain/test/README.md`。

## 入口

1. 主工具链入口：
   - `tools/run.py`
2. suite / runtime-guard 入口：
   - `tools/test.py`
3. suite schema lint 入口：
   - `tools/lint_suites.py`

## 目录职责

1. `tools/toolchain/`
   - build / verify / tidy / result contract 的官方实现
2. `tools/suites/`
   - suite 定义、command matrix、suite-local 脚本
3. `tools/test_framework/`
   - suite runner、runtime guard、suite config loader、output contract 等测试工具链实现
4. `tools/tests/`
   - 工具链自身的 Python 单元 / 组件测试
5. `tools/platform_config/`
   - 平台配置同步与校验
6. `tools/devtools/`
   - 开发辅助工具
7. `tools/scripts/`
   - 辅助脚本索引与开发脚本入口

## 不在这里的内容

1. `test/data/`
   - 共享测试输入数据
2. `test/fixtures/`
   - 小型专用 fixture
3. `test/golden/`
   - golden / snapshot 基线
4. `out/test/`
   - 每次运行的结果、日志与临时产物

这些目录属于共享测试资产层：

1. 改输入数据：
   - 看 `test/data/**`
2. 改小型 fixture：
   - 看 `test/fixtures/**`
3. 改 golden / snapshot：
   - 看 `test/golden/**`
4. 改运行结果目录契约：
   - 看 `docs/tools/toolchain/test/README.md`

## 先读哪些文档

1. 总索引：
   - `docs/tools/toolchain/README.md`
2. 工具链主文档：
   - `docs/tools/toolchain/tools/README.md`
3. 测试与结果契约：
   - `docs/tools/toolchain/test/README.md`
4. 命令改动定位：
   - `docs/tools/toolchain/command_map/README.md`
5. workflow 约定：
   - `docs/tools/toolchain/workflows/README.md`
6. clang-tidy 专项：
   - `docs/tools/toolchain/tidy/README.md`
7. suite 资产层说明：
   - `tools/suites/README.md`
8. 工具链历史：
   - `docs/tools/toolchain/history/README.md`

## 维护约束

1. `tools/run.py` 仍是 build / verify 官方入口。
2. `tools/test.py` 仍是 suite / runtime-guard 官方入口。
3. `tools/lint_suites.py` 仍是 suite schema lint 官方入口。
4. 改动影响工具链协议、suite 入口、结果契约、目录职责时，同步更新
   `docs/tools/toolchain/` 文档。
5. 本文件保持薄索引，不扩写成详细设计文档。

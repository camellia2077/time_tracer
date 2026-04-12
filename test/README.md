# Test Assets Index

`test/` 只承载仓库共享的静态测试资产，不承载 suite runner、工具链实现或
库级 / app 级测试源码。

对本项目而言，主链路是：

1. `apps/tools/log_generator` 生成符合 canonical contract 的 TXT 测试数据
2. `test/data/**` 保存跨 CLI / shell / Android 复用的 TXT 输入资产
3. core / shell / Android 基于这些 TXT 执行 validate / convert / ingest
4. query / report / export 的稳定对账结果进入 `test/golden/**`
5. 每次运行的临时输出统一进入 `out/test/**`

## 目录职责

1. `test/data/`
   - canonical 原始输入数据
   - 例如供 `tracer_core_shell` pipeline / query / reporting 复用的 TXT 月日志
2. `test/fixtures/`
   - 小型专项 fixture
   - 适合精准测试某一个规则、异常路径或兼容场景
3. `test/golden/`
   - 最终输出基线
   - 例如 report/export/query artifact 的 golden / snapshot

## `test/fixtures` 结构

1. `test/fixtures/text/minimal_month/`
   - 极小但合法的 TXT 月文件
   - 适合测月头解析、单日块解析、空月、少量记录、跨天边界
2. `test/fixtures/text/invalid/`
   - 非法或损坏的 TXT 输入
   - 适合测结构校验、逻辑校验、convert / ingest 失败路径
3. `test/fixtures/config/legacy/`
   - 旧版或兼容场景配置
   - 适合 legacy config / migrate / fallback / Android compat
4. `test/fixtures/config/custom/`
   - 小型专用配置样本
   - 适合只测某个 converter/config 行为，不污染 canonical config
5. `test/fixtures/exchange/`
   - exchange/import/export 专项小样本

边界：

1. 不把整年 canonical TXT 数据放进 `test/fixtures`
2. 不把长期对账输出放进 `test/fixtures`
3. 不把运行生成的 sqlite、logs、临时导出放进 `test/fixtures`
4. 不把可由 `test/data` 正常 ingest 生成的 DB 产物 checked in 到
   `test/fixtures`

## 不放在这里的内容

1. `libs/**/tests`
   - 库级 C++ 单测、语义、codec、contract 测试源码
2. `apps/**/tests`
   - host / CLI / bridge / runtime 入口级测试源码
3. `tools/suites/**`
   - suite 定义、command matrix、suite-local 脚本
4. `tools/test_framework/**`
   - runner、result contract、runtime guard 等工具链实现
5. `out/test/**`
   - 每次运行的临时输出、日志、当前产物
6. `test/output/**`
   - 不再作为有效目录使用；新测试不要再写入这里

## 与 `log_generator` 的关系

1. `apps/tools/log_generator`
   - 是生成测试数据的独立工具 app
2. `test/data/**`
   - 是主程序消费的 canonical 测试输入资产
3. `tools/**`
   - 可以放刷新 `test/data` / `test/golden` 的编排脚本

结论：

1. `log_generator` 不迁入 `test/`
2. `test/` 保存资产
3. 生成、编排、消费三层分离

## 进一步阅读

1. `docs/toolchain/test/README.md`
2. `docs/toolchain/test/test_layering.md`
3. `tools/suites/README.md`

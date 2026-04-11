# Test Layering Convention

本文档定义仓库测试目录的职责边界，统一说明 `libs/**/tests`、
`apps/**/tests`、`test/**`、`tools/**` 各自应该承载什么内容。

## 1. 目标

1. 让测试代码按“被测对象”而不是按语言或历史习惯放置。
2. 避免库级语义测试、应用集成测试、suite 资产、工具链测试混在同一层。
3. 为后续迁移提供判断标准，先停止新增错位文件，再做渐进整理。

## 2. 总体规则

1. 先判断“谁是被测对象”，再决定目录。
2. 目录分层优先级：
   - `libs/**/tests`
   - `apps/**/tests`
   - `test/**`
   - `tools/**`
3. 不要因为测试使用了 Python 就默认放入 `test/**`。
   如果被测对象是工具链本身，优先放到 `tools/**`。
4. 不要因为测试使用了 C++ 就默认放入 `libs/**/tests`。
   如果测试依赖 app host、动态库装配、CLI 或跨库集成，优先放到
   `apps/**/tests` 或 `tools/**`。

## 3. `libs/**/tests`

适用对象：

1. 单个 library 的公开 API、模块边界、纯语义逻辑。
2. 该 library 自己拥有的 contract / codec / DTO 映射。
3. 不依赖完整 app host、CLI 产物布局或跨进程运行环境的回归。

典型内容：

1. 单元测试
2. 语义测试
3. module smoke tests
4. transport codec / field / envelope tests

当前仓库中的典型例子：

1. `libs/tracer_core/tests/**`
2. `libs/tracer_transport/tests/**`

## 4. `apps/**/tests`

适用对象：

1. 某个 app、host、bridge、runtime 入口是直接被测对象。
2. 测试覆盖 app 自己拥有的集成边界，而不是 library 内部逻辑。

典型内容：

1. shell / JNI / C ABI / runtime bridge 测试
2. app-specific integration tests
3. host wiring / bootstrap / platform bridge tests

当前仓库中的典型例子：

1. `apps/tracer_core_shell/tests/integration/**`
2. `apps/android/runtime/src/test/**`

## 5. `test/**`

适用对象：

1. 共享测试资产。
2. 跨 suite 复用的输入数据、fixture、golden。
3. 被测对象不是工具链实现本身。

典型内容：

1. `test/data/**`
2. `test/fixtures/**`
3. `test/golden/**`

当前仓库中的典型例子：

1. `test/data/**`
2. `test/golden/**`

## 6. `tools/**`

适用对象：

1. 工具链、suite 资产、测试编排器、结果写出器、质量门禁本身。
2. 平台配置同步、suite launcher、suite 定义、verify/validate orchestration 等基础设施。

典型内容：

1. `tools/run.py`
2. `tools/test.py`
3. `tools/lint_suites.py`
4. `tools/suites/**`
5. `tools/test_framework/**`
6. `tools/tests/**`

当前仓库中的典型例子：

1. `tools/suites/tracer_windows_rust_cli/**`
2. `tools/tests/verify/**`
3. `tools/tests/validate/**`
4. `tools/tests/run_cli/**`

## 7. 决策表

遇到新测试时，按下面顺序判断：

1. 主要在测某个 library 自身边界吗：
   - 是：放 `libs/**/tests`
2. 主要在测某个 app / host / bridge 吗：
   - 是：放 `apps/**/tests`
3. 主要是共享输入数据、fixture、golden 吗：
   - 是：放 `test/**`
4. 主要在测交付产物的 suite 场景、命令矩阵或工具链吗：
   - 是：放 `tools/**`

如果一个测试同时跨两层，默认选择“更靠外”的那层：

1. `library + app host`
   - 优先 `apps/**/tests`
2. `app + suite asset`
   - 优先 `tools/**`
3. `suite + toolchain internals`
   - 优先 `tools/**`

## 8. 迁移原则

1. 先写清规则，再迁移目录。
2. 新增测试从现在开始遵守本约定。
3. 旧测试只在以下情况迁移：
   - 目录已经持续误导读者
   - 改动会反复跨层定位
   - 构建/验证编排已经明显和现目录不匹配
4. 迁移时不要只移动文件路径；同时同步：
   - CMake / build target
   - `verify` / `validate` 路由
   - suite / result contract
   - 相关 `README.md` / 架构文档

## 9. 相关文档

1. [README.md](README.md)
2. [suite_toml_organization.md](suite_toml_organization.md)
3. [../tools/README.md](../tools/README.md)
4. [../../time_tracer/architecture/libraries/tracer_core.md](../../time_tracer/architecture/libraries/tracer_core.md)
5. [../../time_tracer/architecture/libraries/tracer_transport.md](../../time_tracer/architecture/libraries/tracer_transport.md)

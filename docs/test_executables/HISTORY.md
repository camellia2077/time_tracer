## 2026-01-18 更新说明

### 核心变更摘要

本次更新主要修复了测试环境准备阶段的逻辑控制缺陷，重构了架构以实现“业务逻辑”与“结果展示”的分离，并增强了路径加载的健壮性。

### 1. 缺陷修复 (Bug Fixes)

* **修复环境清理失效问题**：
* 修复了 `enable_environment_clean = 1` 配置无法正确清空目标文件夹的问题。现在清理指令能正确从 `Engine` 传递至 `Workspace`。


* **修复清理与部署的逻辑耦合**：
* 解决了清理后立即自动复制文件导致“清理看似无效”的问题。在 `EnvironmentManager.setup` 中增加了 `should_deploy` 参数，允许在配置中独立控制“是否清理”和“是否部署”。


* **修复配置文件路径加载错误**：
* 解决了在不同目录下运行脚本时找不到 `config.toml` 的 `FileNotFoundError`。现在由入口文件 `run.py` 计算绝对路径并向下传递，不再依赖当前工作目录。



### 2. 架构改进 (Architecture Improvements)

* **实现执行与展示分离 (Separation of Execution and Reporting)**：
* **移除耦合**：`BaseTester` 及其子类不再直接进行控制台打印（`print`），改为返回结构化的 `TestReport` 对象。
* **新增组件**：引入 `Reporter` 类专门负责 UI 展示逻辑，由 `Engine` 统一调度。


* **资源懒加载 (Lazy Initialization)**：
* 将日志目录 (`py_output`) 的创建权从 `Engine` 下沉至 `TestLogger`。现在只有在真正需要写日志时才会创建文件夹，避免了在纯清理模式下产生空文件夹的问题。



### 3. 代码重构 (Refactoring)

* **ExportTester 逻辑解耦**：
* 将 `ExportTester` 中臃肿的 `run_tests` 方法拆分为 `_check_preconditions`（前置检查）、`_plan_test_cases`（测试计划生成）和 `_make_cases`（工厂方法）等子函数，显著降低了代码复杂度。


* **增强调试信息**：
* `main.py` 的 `print_header` 函数现在会打印完整的路径配置摘要（Source, Target, DB, Logs 等），便于排查路径配置错误。
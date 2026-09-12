# Test And Result Guide

本文档是当前测试工具链的权威入口，说明：

1. 官方测试命令入口
2. `tools/` 与 `test/` 的职责分工
3. `verify` / suite 的结果产物契约
4. 仓库长期采用的测试资产布局

## 1. 官方入口

1. 项目统一验证入口：
   - `python tools/run.py verify ...`
2. 工具链最小回归：
   - `python tools/run.py self-test`
3. suite / runtime-guard 入口：
   - `python tools/test.py suite ...`
   - `python tools/test.py runtime-guard ...`
   - `python tools/test.py smoke-windows-cli ...`
4. suite schema lint：
   - `python tools/lint_suites.py`
5. Android 模块定向单测：
   - `python tools/run.py android-test --module feature-insights --tests com.example.tracer.QueryInsightsResultDisplayRobolectricTest`

GitHub Actions 按验证边界拆分为三个独立 workflow：

1. `android-ci.yml`：Android 构建与测试
2. `libs-ci.yml`：Ubuntu 上的共享 libs/core 验证（`--scope libs`）
3. `cli-ci.yml`：Windows CLI presentation 验证（`--scope cli`，包含 optimized/LTO matrix）

libs 发生变化时，`libs-ci` 与 `cli-ci` 会同时触发；CLI workflow 会重新在 Windows
工具链中编译其依赖的 libs，但不会重复执行 libs 测试。

Android 定向单测每次运行都会清理旧的 `artifact_android` 结果文件，并将本次
Gradle 输出写入 `out/test/artifact_android/logs/output.full.log`。只有日志明确显示
Kotlin 中间产物锁定或复用异常时，工具链才会清理 `built_in_kotlinc` 并重试一次。

## 2. 目录分工

1. `tools/test.py`
   - suite / runtime-guard 的统一入口
2. `tools/lint_suites.py`
   - suite TOML schema lint 入口
3. `tools/suites/**`
   - artifact-level suite 定义、command matrix、suite-local 脚本
4. `tools/test_framework/**`
   - suite runner、config loader、runtime guard、output contract 等测试工具链实现
5. `tools/tests/**`
   - 工具链自己的 Python 单元 / 组件测试
6. `test/data/**`
   - 共享输入数据
7. `test/fixtures/**`
   - 小型专项 fixture
8. `test/golden/**`
   - golden / snapshot 基线

结论：

1. runner 实现属于 `tools/`
2. suite 资产也属于 `tools/`
3. `test/` 仅保留共享测试资产
4. 运行结果统一写入 `out/test/**`
5. `apps/tools/log_generator` 保持为独立工具 app，不迁入 `test/`
6. `test/output/**` 不再作为新测试的输出落点

## 2.1 仓库长期布局约定

1. `libs/**/tests`
   - 库级语义、模块、codec、contract 测试源码
2. `apps/**/tests`
   - host / CLI / bridge / runtime 入口级测试源码
3. `test/**`
   - 共享静态测试资产
4. `tools/**`
   - suite、verify、资产刷新工具、runner 实现

对当前仓库的具体解释：

1. `apps/tools/log_generator`
   - 是生成测试数据的工具 app
2. `test/data/**`
   - 是主程序消费的 canonical TXT 输入资产
3. `test/fixtures/**`
   - 保存小型专项样本，而不是第二套 canonical 数据库
4. `test/golden/**`
   - 保存最终输出对账基线

当前项目的测试主链路：

1. `log_generator` 生成 canonical TXT 数据
2. `test/data/**` 保存共享输入 TXT
3. core / shell / Android 对这些 TXT 执行 validate / convert / ingest
4. query / insights / export 的稳定对账结果进入 `test/golden/**`
5. 所有运行时生成物进入 `out/test/**`

`test/fixtures/**` 的推荐结构：

1. `text/minimal_month/`
2. `text/invalid/`
3. `config/legacy/`
4. `config/custom/`
5. `exchange/`

## 3. 常见命令

```bash
# Windows CLI verify
python tools/run.py verify --app tracer_core_shell --profile release_bundle_ci_no_pch --build-dir build_release --scope cli --concise

# Shared libs/core verify (does not build or test the Windows Rust CLI)
python tools/run.py verify --app tracer_core_shell --profile libs_ci_no_pch --build-dir build_libs --scope libs --concise

# Compose both scopes explicitly when one environment should run the complete flow
python tools/run.py verify --app tracer_core_shell --profile fast_ci_no_pch --scope libs --scope cli --concise

# toolchain self-test
python tools/run.py self-test
python tools/run.py self-test --group verify-stack --quiet

# Android module test (omit --tests to run the module's full debug unit-test suite)
python tools/run.py android-test --module feature-insights --tests com.example.tracer.QueryInsightsResultDisplayRobolectricTest

# suite run
python tools/test.py suite --suite artifact_windows_cli --agent --build-dir build_fast --concise

# runtime guard
python tools/test.py runtime-guard --build-dir build_fast

# suite lint
python tools/lint_suites.py --suite tracer_windows_rust_cli
```

在 Windows 本机执行最后两条中的 `--scope libs` 时，`tools/run.py` 会自动将该验证转发到 WSL2 的 `Ubuntu` 发行版；默认使用独立的 `build_libs_ubuntu` 目录，避免与 Windows CMake cache 混用。Ubuntu 需要安装 `libsqlite3-dev` 等基础构建依赖。可通过 `TT_LIBS_WSL_DISTRO` 覆盖发行版名称，或设置 `TT_LIBS_UBUNTU_ACTIVE=1` 调试底层命令。

也可以显式指定验证平台：`--test-platform auto|windows|ubuntu`。例如 `--test-platform windows` 强制使用 Windows，`--test-platform ubuntu` 强制使用 WSL2 Ubuntu（目前只支持 `--scope libs`）。

### WSL2 Ubuntu 本机初始化

Windows 本机要让 `--scope libs` 的 Ubuntu 路径可直接运行，先在 Ubuntu 终端执行一次：

```bash
sudo apt update
sudo apt install -y clang cmake ninja-build git libsqlite3-dev python3-venv
cd /mnt/c/code/time_tracer
python3 -m venv .venv
. .venv/bin/activate
python -m pip install -e .
```

项目 Python 工具依赖由仓库根目录的 `pyproject.toml` 声明，目前包括 `markdown-it-py` 和 `pylatexenc`。转发器会优先使用 `/mnt/c/code/time_tracer/.venv/bin/python`；没有该虚拟环境时才回退到 Ubuntu 的 `python3`。

`nlohmann_json`、`libsodium`、`zstd` 等 C++ 依赖由 CMake 在配置阶段自动发现或下载，不需要另外安装。SQLite 使用 Ubuntu 的 `libsqlite3-dev`，因为它提供 CMake 所需的头文件和链接库。

初始化后，从 Windows 仓库终端验证：

```powershell
python tools/run.py verify --app tracer_core_shell --profile libs_ci_no_pch --build-dir build_libs --scope libs --test-platform ubuntu --concise
```

## 4. verify 行为约定

1. `verify` 是项目统一验证入口。
2. `verify` 支持两个可重复组合的执行范围：
   - `--scope libs`：只构建 CMake core，并执行共享 Python verify-stack、native/core
     语义与 contract 测试；不构建 Cargo CLI，不运行 CLI suite 或输出质量门禁。
   - `--scope cli`：构建 CMake core 后继续构建链接它的 Windows Rust CLI，并执行 CLI
     黑盒 suite 与输出质量门禁；不会重复执行 libs/native/共享 verify-stack 测试。
   - core/CLI verify 必须显式传入至少一个 `--scope`；多个 `--scope` 会去重后按一个流程执行。
3. focused capability profile 不只跑黑盒或 smoke，也会带 capability 对应的
   core semantics / C ABI contract 测试：
   - `cap_query`
     - `tt_query_api_tests`
     - `tc_c_api_query_tests`
   - `cap_insights`
     - `tt_insights_api_tests`
     - `tc_c_api_insights_tests`
   - `cap_pipeline`
     - `tt_pipeline_api_tests`
     - `tc_c_api_pipeline_tests`
   - `shell_aggregate`
     - `tc_c_api_shell_aggregate_tests`

## 5. suite 行为约定

1. suite 入口统一走 `tools/test.py`，不要再使用旧 `test/run.py`。
2. suite 资产现在在 `tools/suites/**`。
3. suite runner 实现在 `tools/test_framework/**`，不要再放回 `test/`。

## 6. 结果产物契约

1. test / verify 结果：
   - `out/test/<result_target>/result.json`
   - `out/test/<result_target>/result_cases.json`
   - `out/test/<result_target>/logs/output.log`
   - `out/test/<result_target>/quality_gates/`
2. `test/**` 不保存每次运行的临时输出目录

## 7. result target 映射

1. `tracer_core`
2. `tracer_core_shell`
3. `tracer_windows_rust_cli`
   - 以上三个都映射到 `artifact_windows_cli`
4. `tracer_android`
   - 映射到 `artifact_android`
5. `log_generator`
   - 映射到 `artifact_log_generator`
6. 未映射 app：
   - 保持 `<result_target>=<app>`

## 8. 回归建议

1. 改 CLI handler：
   - 至少补或运行对应 `tools/tests/run_cli/` 回归
   - 如涉及请求映射或 host 输出，也检查 `apps/**/tests`
2. 改 `verify` / result contract：
   - 至少补或运行 `tools/tests/verify/` 对应回归
3. 改 suite runner、runtime guard、suite loader：
   - 至少补或运行 `tools/tests/platform/core/` 与相关 `tools/tests/verify/` 回归
4. 改 suite 资产、命令矩阵、suite-local 脚本：
   - 至少运行对应 `python tools/test.py suite ...` 或 `python tools/lint_suites.py`
5. 改 canonical TXT 输入、fixture、golden：
   - 至少同步更新 `test/README.md` 与对应资产说明

## 9. 测试分层与测试意图

1. 测试目录职责边界见：
   - [test_layering.md](test_layering.md)
2. 共享测试资产目录说明见：
   - [test/README.md](../../../../test/README.md)
3. 库级语义 / contract 测试意图的写法见相关 library docs：
   - `docs/time_tracer/architecture/libraries/tracer_core.md`
   - `docs/time_tracer/architecture/libraries/tracer_transport.md`

## 10. 延伸阅读

1. [../README.md](../README.md)
2. [../tools/README.md](../tools/README.md)
3. [suite_toml_organization.md](suite_toml_organization.md)
4. [history/README.md](../history/README.md)

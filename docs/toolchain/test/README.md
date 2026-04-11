# Test And Result Guide

本文档是当前测试工具链的权威入口，说明：

1. 官方测试命令入口
2. `tools/` 与 `test/` 的职责分工
3. `verify` / `validate` / suite 的结果产物契约

## 1. 官方入口

1. 项目统一验证入口：
   - `python tools/run.py verify ...`
2. 多 track 验证编排：
   - `python tools/run.py validate --plan <plan.toml> ...`
3. 工具链最小回归：
   - `python tools/run.py self-test`
4. suite / runtime-guard 入口：
   - `python tools/test.py suite ...`
   - `python tools/test.py runtime-guard ...`
   - `python tools/test.py smoke-windows-cli ...`
5. suite schema lint：
   - `python tools/lint_suites.py`

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
   - 小型 fixture
8. `test/golden/**`
   - golden / snapshot 基线

结论：

1. runner 实现属于 `tools/`
2. suite 资产也属于 `tools/`
3. `test/` 仅保留共享测试资产

## 3. 常见命令

```bash
# Windows CLI verify
python tools/run.py verify --app tracer_core_shell --build-dir build_fast --concise
python tools/run.py verify --app tracer_core_shell --profile fast --concise

# validate
python tools/run.py validate --plan temp/import_batch01.toml --paths-file temp/import_batch01.paths
python tools/run.py validate --plan temp/import_batch01.toml --paths-file temp/import_batch01.paths --quiet

# toolchain self-test
python tools/run.py self-test
python tools/run.py self-test --group verify-stack --quiet

# suite run
python tools/test.py suite --suite artifact_windows_cli --agent --build-dir build_fast --concise

# runtime guard
python tools/test.py runtime-guard --build-dir build_fast

# suite lint
python tools/lint_suites.py --suite tracer_windows_rust_cli
```

## 4. verify 行为约定

1. `verify` 是项目统一验证入口。
2. `kind = "verify"` 的 validate track 与直接执行 `verify` 的结果语义保持一致。
3. 对 `tracer_core_shell`：
   - 会构建 Windows CLI 对应产物
   - 会执行 Python unit/component checks
   - 会执行 native smoke / runtime tests / suite checks
   - 会产出 artifact 级质量门禁结果
4. focused capability profile 不只跑黑盒或 smoke，也会带 capability 对应的
   core semantics / C ABI contract 测试：
   - `cap_query`
     - `tt_query_api_tests`
     - `tc_c_api_query_tests`
   - `cap_reporting`
     - `tt_reporting_api_tests`
     - `tc_c_api_reporting_tests`
   - `cap_pipeline`
     - `tt_pipeline_api_tests`
     - `tc_c_api_pipeline_tests`
   - `shell_aggregate`
     - `tc_c_api_shell_aggregate_tests`

## 5. validate 行为约定

1. `validate --plan` 适合多 track、多 build_dir、多 profile 的成组验证。
2. `--paths` / `--paths-file` 用于记录本次关注范围。
3. `validate` 默认实时镜像子命令输出；追加 `--quiet` 才只写日志。
4. TOML 结构和示例以 [../validate/plan_toml.md](../validate/plan_toml.md) 为准。

## 6. suite 行为约定

1. suite 入口统一走 `tools/test.py`，不要再使用旧 `test/run.py`。
2. suite 资产现在在 `tools/suites/**`。
3. suite runner 实现在 `tools/test_framework/**`，不要再放回 `test/`。

## 7. 结果产物契约

1. validate 结果：
   - `out/validate/<run_name>/summary.json`
   - `out/validate/<run_name>/logs/output.log`
   - `out/validate/<run_name>/logs/output.full.log`
   - `out/validate/<run_name>/tracks/<track>/<step>.log`
2. test / verify 结果：
   - `out/test/<result_target>/result.json`
   - `out/test/<result_target>/result_cases.json`
   - `out/test/<result_target>/logs/output.log`
   - `out/test/<result_target>/quality_gates/`

## 8. result target 映射

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

## 9. 回归建议

1. 改 CLI handler：
   - 至少补或运行对应 `tools/tests/run_cli/` 回归
2. 改 `verify` / `validate` / result contract：
   - 至少补或运行 `tools/tests/verify/`、`tools/tests/validate/` 对应回归
3. 改 suite runner、runtime guard、suite loader：
   - 至少补或运行 `tools/tests/platform/core/` 与相关 `tools/tests/verify/` 回归
4. 改 suite 资产、命令矩阵、suite-local 脚本：
   - 至少运行对应 `python tools/test.py suite ...` 或 `python tools/lint_suites.py`

## 10. 测试分层与测试意图

1. 测试目录职责边界见：
   - [test_layering.md](test_layering.md)
2. 库级语义 / contract 测试意图的写法见相关 library docs：
   - `docs/time_tracer/architecture/libraries/tracer_core.md`
   - `docs/time_tracer/architecture/libraries/tracer_transport.md`

## 11. 延伸阅读

1. [../README.md](../README.md)
2. [../tools/README.md](../tools/README.md)
3. [../validate/README.md](../validate/README.md)
4. [suite_toml_organization.md](suite_toml_organization.md)
5. [history/README.md](history/README.md)

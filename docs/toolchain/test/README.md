# Test And Result Guide

本文档说明 `verify` / `validate` / `self-test` / suite 运行方式，以及 `out/test` / `out/validate` 的结果产物契约。

## 1. 官方测试入口

1. 工具链回归与产物验证：
   - `python tools/run.py verify ...`
2. 多 track 验证编排：
   - `python tools/run.py validate --plan <plan.toml> ...`
3. 工具链最小回归：
   - `python tools/run.py self-test`
4. suite 直跑入口：
   - `python test/run.py suite --suite <suite_name> ...`

## 2. 常见命令

```bash
# Windows CLI 单命令验证入口
python tools/run.py verify --app tracer_core_shell --build-dir build_fast --concise
python tools/run.py verify --app tracer_core_shell --profile fast --concise

# Android 验证
python tools/run.py verify --app tracer_android --profile android_style --concise
python tools/run.py verify --app tracer_android --profile android_ci --concise

# validate
python tools/run.py validate --plan temp/import_batch01.toml --paths-file temp/import_batch01.paths
python tools/run.py validate --plan temp/import_batch01.toml --paths-file temp/import_batch01.paths --quiet

# toolchain 自检
python tools/run.py self-test
python tools/run.py self-test --group verify-stack --quiet

# suite 直跑
python test/run.py suite --suite artifact_windows_cli --agent --build-dir build_fast --concise
```

## 3. verify 行为约定

1. `verify` 是项目的统一验证入口
2. `kind = "verify"` 的 validate track 与直接执行 `verify` 的结果语义保持一致
3. 对 `tracer_core_shell`：
   - 会构建 Windows CLI 对应产物
   - 会执行 Python unit/component checks
   - 会执行 native smoke / runtime tests / suite checks
   - 会产出 artifact 级质量门禁结果

## 4. validate 行为约定

1. `validate --plan` 适合多 track、多 build_dir、多 profile 的成组验证
2. `--paths` / `--paths-file` 用于记录本次关注范围
3. `validate` 默认实时镜像子命令输出；追加 `--quiet` 才只写日志
4. TOML 结构和示例以 [../validate/plan_toml.md](../validate/plan_toml.md) 为准

## 5. 结果产物契约

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

## 6. result target 映射

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

## 7. 回归建议

1. 改 CLI handler：
   - 至少补或运行对应 `tools/tests/run_cli/` 回归
2. 改 `commands/verify` / `commands/validate`：
   - 至少补或运行 `tools/tests/verify/`、`tools/tests/validate/` 对应回归
3. 改 `commands/tidy`：
   - 至少补或运行 `tools/tests/platform/test_tidy_*.py`
4. 改结果契约或质量门禁：
   - 同步检查 `out/test/.../result.json`、`quality_gates/` 和文档

## 8. 延伸阅读

1. [../README.md](../README.md)
2. [../tools/README.md](../tools/README.md)
3. [../validate/README.md](../validate/README.md)
4. [suite_toml_organization.md](suite_toml_organization.md)
5. [history/README.md](history/README.md)

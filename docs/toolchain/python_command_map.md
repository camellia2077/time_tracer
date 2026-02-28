# Python Toolchain Command Map

本文档用于快速定位 Python 工具链修改点，避免在 `scripts/`、`test/`、wrapper 脚本之间改错层。

## 1. 入口链路

1. `scripts/run.py`
   - 统一入口，构建 `Context`，分发到 CLI 子命令
2. `scripts/toolchain/cli/parser.py`
   - 注册全部子命令与通用参数
3. `scripts/toolchain/cli/handlers/*.py`
   - 每个命令的参数定义与入参组装（build/verify/tidy 等）
4. `scripts/toolchain/commands/**`
   - 真实业务执行逻辑（调用 cmake、test runner、配置同步等）
5. `scripts/toolchain/core/**`
   - 基础设施：配置模型、上下文、进程执行器

## 2. 需求 -> 修改位置速查

1. 调整 `build/verify/tidy` 命令参数（新增/改名/默认值）
   - 先改：`scripts/toolchain/cli/handlers/<command>.py`
   - 再看是否需要改：`scripts/toolchain/commands/**`

2. 调整构建行为（自动 configure、`--target` 注入、build dir、平台分支）
   - 先改：`scripts/toolchain/commands/cmd_build/cmake.py`
   - 关联：`scripts/toolchain/commands/cmd_build/command.py`

3. 调整 profile 默认策略（如 `cmake_args`、`build_targets`、`BUILD_TESTING`）
   - 先改：`scripts/toolchain/config.toml`
   - 字段模型：`scripts/toolchain/core/config.py`
   - 读取适配：`scripts/toolchain/commands/cmd_build/common/profile_backend.py`

4. 调整 verify 流程（build + test 转发规则）
   - 主入口：`scripts/run.py verify`
   - 参数层：`scripts/toolchain/cli/handlers/verify.py`
   - 执行层：`scripts/toolchain/commands/cmd_quality/verify.py`

5. 调整 shell wrapper 参数通道（run.py 参数 vs cmake --build 参数）
   - `apps/tracer_cli/windows/scripts/build.sh`
   - `apps/tracer_cli/windows/scripts/build_fast.sh`
   - `apps/tracer_cli/windows/scripts/build_release.sh`

6. 调整 clang-tidy 第三方头过滤
   - 配置优先：`scripts/toolchain/config.toml` -> `[tidy].header_filter_regex`
   - 默认回退：`scripts/toolchain/commands/cmd_build/cmake.py`

7. 执行 clang-tidy 批次收口（统一入口）
   - 命令：`python scripts/run.py tidy-batch --app tracer_windows_cli --batch-id <BATCH_ID> --strict-clean --run-verify --concise --full-every 3 --keep-going`
   - 参数层：`scripts/toolchain/cli/handlers/tidy_batch.py`
   - 执行层：`scripts/toolchain/commands/tidy/batch.py`

## 3. 最小回归命令

```bash
python scripts/run.py build --app tracer_windows_cli --profile release_bundle -- --target help
python scripts/run.py verify --app tracer_core --quick --concise
python scripts/run.py tidy --app tracer_windows_cli -- --target tidy_all
python scripts/run.py tidy-batch --app tracer_windows_cli --batch-id <BATCH_ID> --strict-clean --run-verify --concise --full-every 3 --keep-going
```

## 4. 相关导航

1. `scripts/AGENTS.md`
2. `scripts/README.md`
3. `scripts/toolchain/README.md`
4. `docs/toolchain/clang_tidy_sop.md`


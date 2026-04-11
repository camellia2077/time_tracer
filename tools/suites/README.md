# Suite Assets Index

`tools/suites/` 保存的是 artifact-level suite 资产，不是 runner 实现。

这里描述的是：

1. suite 入口 TOML
2. command matrix
3. suite-local 配置
4. 少量只服务于该 suite 的辅助脚本

suite 执行入口与 schema lint 入口在 `tools/`：

1. `python tools/test.py suite ...`
2. `python tools/lint_suites.py`

## 逻辑 suite 名与物理目录

1. `artifact_windows_cli`
   - `tracer_windows_rust_cli/`
2. `artifact_android`
   - `tracer_android/`
3. `artifact_log_generator`
   - `log_generator/`

## 当前 suite 目录

1. `tracer_windows_rust_cli/`
   - core + Windows CLI 集成 suite
2. `tracer_android/`
   - Android host-side verification suite
3. `log_generator/`
   - `apps/log_generator` 对应 suite

## 每个 suite 里通常包含什么

1. `config.toml`
   - suite 总入口
2. `env.toml`
   - 环境 / 部署路径
3. `tests.toml`
   - 聚合命令表
4. `config_cap_<capability>.toml`
   - capability / profile 级入口
5. `config_<profile>.toml`
   - profile 级入口
6. `tests/base*.toml`
   - 基础命令表
7. `tests/commands_*.toml`
   - 责任域命令表

## 命名与组织约定

1. `tests/` 子目录当前承载的是 command-table 资产，不是 Python 测试源码目录。
2. 责任域命令文件统一使用 `commands_*.toml`。
3. 不再新增 `command_groups*.toml` 文件名。
4. 即使 TOML 体内使用 `[[command_groups]]`，文件名仍应表达责任边界。

## 路径与变量约定

1. suite TOML 中的路径字段支持相对路径。
2. 相对路径从当前 TOML 所在目录解析。
3. 支持 `${repo_root}` 变量展开。
4. 可选默认构建目录：
   - `paths.default_build_dir`
   - 例如 `build`、`build_fast`、`build_agent`、`build_check`

## 校验与运行

1. schema lint：
   - `python tools/lint_suites.py`
2. 单 suite lint：
   - `python tools/lint_suites.py --suite tracer_windows_rust_cli`
3. suite 执行：
   - `python tools/test.py suite --suite artifact_windows_cli --build-dir build_fast`

schema 校验会检查：

1. 必需 sections / fields
2. 未解析 `${...}` 变量
3. command placeholders 是否属于允许范围

## 维护约束

1. 不要把 suite runner、runtime guard、config loader 实现放进 `tools/suites/`。
2. `tools/suites/` 里的脚本应保持 suite-local；通用逻辑应上移到 `tools/`。
3. 改 suite 目录职责、命名规则或 command-table 组织方式时，同步更新：
   - `tools/README.md`
   - `docs/toolchain/test/README.md`
   - `docs/toolchain/test/suite_toml_organization.md`
   - 需要时更新 `docs/toolchain/test/test_layering.md`

## 延伸阅读

1. `docs/toolchain/test/README.md`
2. `docs/toolchain/test/suite_toml_organization.md`
3. `docs/toolchain/test/test_layering.md`

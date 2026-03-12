# Validate Plan TOML 规范

本文档定义 `python tools/run.py validate --plan <plan.toml>` 使用的 TOML 结构与行为约定。

## 1. 权威来源

- 解析与默认值：`tools/toolchain/commands/cmd_validate/plan.py`
- 执行行为：`tools/toolchain/commands/cmd_validate/command.py`
- 回归样例：`tools/tests/validate/test_validate_plan.py`、`tools/tests/validate/test_validate_command.py`

如果本文档与代码不一致，以代码为准。

## 2. 最小可用模板

```toml
[run]
name = "import phase5 logging patch b1 minimal"
continue_on_failure = false

[defaults]
kind = "verify"
app = "tracer_core_shell"
verify_scope = "batch"
concise = true

[[tracks]]
name = "modules_on"
build_dir = "build_import_phase5_logging_patchB1"
```

执行示例：

```bash
python tools/run.py validate --plan temp/import_phase5_logging_patchB1_minimal.toml --paths libs/tracer_core/src/foo.cpp
```

## 3. CLI 输入约束

- `--plan` 必填，指向计划 TOML。
- `--paths` 或 `--paths-file` 至少提供一个（可同时提供）。
- `--paths-file` 按行读取路径，空行与 `#` 注释行会被忽略。
- 范围路径会做去重并规范为仓库相对路径。

## 4. TOML 结构

### 4.1 `[run]`（可选）

- `name`：运行名（可选）。不填时默认取 plan 文件名（去扩展名）。
- `continue_on_failure`：是否在某个 track 失败后继续后续 track。
  - 默认：`true`

`run.name` 会被规范化后用于输出目录名 `out/validate/<run_name>/`：

- 仅保留 `A-Za-z0-9._-`
- 其他字符替换为 `_`
- 首尾 `._-` 会被裁剪
- 空字符串会回退为 `validate_run`

CLI 传入 `--run-name` 时，会覆盖 `[run].name`。

### 4.2 `[defaults]`（可选）

用于给 `[[tracks]]` 提供默认值。

- `kind`：`configure` | `build` | `verify`（默认 `verify`）
- `app`：应用名（如 `tracer_core_shell`）
- `profile`：构建 profile 名
- `build_dir`：逻辑构建目录名
- `cmake_args`：字符串数组，附加 CMake 参数
- `verify_scope`：`task` | `unit` | `artifact` | `batch`（默认 `batch`）
- `concise`：是否启用简洁输出（默认 `true`）
- `kill_build_procs`：是否启用 build 前工具清理（默认 `false`）

### 4.3 `[[tracks]]`（必填，至少一个）

每个 track 可使用和 `[defaults]` 同名字段，并覆盖默认值。

- `name`：可选；默认 `track_01`、`track_02` ...
- `kind`：可选；默认继承 `[defaults].kind`，再回退 `verify`
- `app`：可选；优先取 track 自身，否则取 `[defaults].app`
  - 若两者都缺失，命令报错并退出
- 其余字段：同理先 track，再 defaults，再代码默认值

## 5. `kind` 与 `verify_scope` 行为

- `kind = "configure"`：只跑 configure 阶段。
- `kind = "build"`：跑 build 阶段（必要时会先完成 configure）。
- `kind = "verify"`：先 build，再按 `verify_scope` 决定后续验证：
  - `task`：任务级检查
  - `unit`：单元/内部检查
  - `artifact`：产物套件检查
  - `batch`：`unit + artifact` 组合

## 6. 输出产物位置

一次 validate 运行写入：

- `out/validate/<run_name>/summary.json`
- `out/validate/<run_name>/logs/output.log`
- `out/validate/<run_name>/logs/output.full.log`
- `out/validate/<run_name>/tracks/<track>/<step>.log`

其中 `summary.json` 关键字段包含：

- `schema_version`
- `command`
- `plan_path`
- `scope_paths`
- `success`
- `exit_code`
- `started_at` / `finished_at` / `duration_ms`
- `tracks`
- `failures`

## 7. Phase5 Logging 示例

### 7.1 B1 Minimal

文件：`temp/import_phase5_logging_patchB1_minimal.toml`

```toml
[run]
name = "import phase5 logging patch b1 minimal"
continue_on_failure = false

[defaults]
kind = "verify"
app = "tracer_core_shell"
verify_scope = "batch"
concise = true

[[tracks]]
name = "modules_on"
build_dir = "build_import_phase5_logging_patchB1"
```

### 7.2 B2 Extension（示例：在最小版基础上扩展第二轨）

文件：`temp/import_phase5_logging_patchB2_extension.toml`

当前最小内容：

```toml
[run]
name = "import phase5 logging patch b2 extension"
continue_on_failure = false

[defaults]
kind = "verify"
app = "tracer_core_shell"
verify_scope = "batch"
concise = true

[[tracks]]
name = "modules_on"
build_dir = "build_import_phase5_logging_patchB2"
```

扩展示例（双轨对比）：

```toml
[run]
name = "import phase5 logging patch b2 extension"
continue_on_failure = false

[defaults]
kind = "verify"
app = "tracer_core_shell"
verify_scope = "batch"
concise = true

[[tracks]]
name = "modules_on"
build_dir = "build_import_phase5_logging_patchB2_on"

[[tracks]]
name = "modules_off"
build_dir = "build_import_phase5_logging_patchB2_off"
cmake_args = ["-DTT_ENABLE_CPP20_MODULES=OFF"]
```

### 7.3 C Reassessment

文件：`temp/import_phase5_logging_patchC_reassessment.toml`

```toml
[run]
name = "import phase5 logging patch c reassessment"
continue_on_failure = false

[defaults]
kind = "verify"
app = "tracer_core_shell"
verify_scope = "batch"
concise = true

[[tracks]]
name = "modules_on"
build_dir = "build_import_phase5_logging_patchC"
```


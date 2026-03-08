# test

Unified test workspace.

## Two-layer model

- Internal test layer (logic-focused, fast):
  - Entry: `python tools/run.py verify --app tracer_core --scope unit`
  - Target: unit/component logic checks.
- Artifact test layer (delivery-focused, executable/runtime contract):
  - Entry: `python tools/run.py verify --app tracer_core --scope artifact --build-dir build_fast --concise`
  - `build_fast` here is the Windows CLI / flexible CMake build-dir example, not an Android recommendation.
  - Target: runtime workspace/output contract + integration/e2e/golden checks.

Suite naming policy:

- Canonical artifact suite names:
  - `artifact_windows_cli`
  - `artifact_android`
  - `artifact_log_generator`

## Canonical structure

- `framework/`: shared Python testing framework and runner internals.
  - `framework/core/`: reusable testing engine package.
  - `framework/runner/service.py`: suite execution orchestrator.
  - `framework/runner/commands/`: `test/run.py` 子命令实现（suite/runtime-guard/smoke）。
- `suites/tracer_windows_rust_cli/`: artifact suite folder for `artifact_windows_cli` (`apps/tracer_cli/windows` build target).
- `suites/tracer_android/`: artifact suite folder for `artifact_android`.
- `suites/log_generator/`: artifact suite folder for `artifact_log_generator`.
- `data/`: canonical integration/e2e input dataset shared by CLI/Android.
- `fixtures/unit/`: minimal unit/component fixtures (small, focused, fast).
- `golden/`: snapshot/golden expected outputs for artifact regression checks.
- `output/`: all generated runtime artifacts.
- `run.py`: test 根目录唯一 Python 入口（`suite` + `runtime-guard` + `smoke-windows-cli`），invoked by `tools/run.py`.
- `run_runtime_guard.bat`: 统一 runtime-guard 入口，使用 `-b build|build_fast` 选择源目录；如缺 `time_tracer_cli.exe` 会自动尝试从 `apps/tracer_cli/windows/rust_cli/<build>/bin`（再回退 `rust_cli/build/bin`）回填。
- `run_runtime_guard_from_build.bat`: 兼容壳，等价于 `run_runtime_guard.bat -b build`。
- `run_runtime_guard_from_build_fast.bat`: 兼容壳，等价于 `run_runtime_guard.bat -b build_fast`。
- `runtime-guard` 场景在执行前会先完整复制源 `bin` 目录，再针对单个场景做“删文件”变异，保证与真实运行时布局一致。

## Output layering

Each artifact suite writes into:

- `output/<suite>/workspace`: copied binaries and runtime workspace.
- `output/<suite>/logs`: per-case logs + python concise log (`output.log`) + python full log (`output_full.log`).
- `output/<suite>/artifacts`: generated report/output files.
- `output/<suite>/result.json`: machine-readable summary.
- Runner enforces this contract strictly; non-canonical result path overrides are ignored.

Canonical output directories:

- `output/artifact_windows_cli`
- `output/artifact_android`
- `output/artifact_log_generator`

Result file semantics (unified):

- `post_change_last.json` (state): execution steps and failure location.
- `result.json` (summary): overall success/failure and module-level aggregation.
- `result_cases.json` (case details): per-case failure details.
- `logs/output.log` (aggregated log): key error lines for triage.

## Config path variables

Suite TOML files support `${repo_root}` and relative paths.

## Quick start

From repository root:

- Daily one-command flow:
  - `python tools/run.py post-change --app tracer_core --run-tests always --build-dir build_fast --concise`
  - `build_fast` here is the default quick path for flexible CMake backends.
- Milestone/release flow:
  - `python tools/run.py verify --app tracer_core --quick --scope batch --concise`
- Windows CLI（Rust-only）:
  - `python test/run.py suite --suite artifact_windows_cli --bin-dir apps/tracer_cli/windows/rust_cli/build_fast/bin --no-format-on-success --concise`
- Android edit loop:
  - `python tools/run.py build --app tracer_android --profile android_edit`
- Android style gate:
  - `python tools/run.py verify --app tracer_android --profile android_style --concise`
- Android CI-like gate:
  - `python tools/run.py verify --app tracer_android --profile android_ci --concise`
- Android post-change validation:
  - `python tools/run.py post-change --app tracer_android --run-tests always --concise`

## Verify scope layering

- Internal logic tests (unit/component):
  - `python tools/run.py verify --app tracer_core --scope unit`
- Artifact/result checks (integration/e2e/snapshot gates):
  - `python tools/run.py verify --app tracer_core --scope artifact --build-dir build_fast --concise`
  - `build_fast` here applies to flexible CMake backends such as Windows CLI flows.
  - Includes fixed snapshot gates for `day/month/range × md/tex/typ` (`test/golden/report_triplet/*/v1`).
- Full verify pipeline (unit + artifact):
  - `python tools/run.py verify --app tracer_core --scope batch --build-dir build_fast --concise`
- Lightweight task checks (build + native runtime smoke only):
  - `python tools/run.py verify --app tracer_core --scope task --build-dir build_fast --concise`

## Test Data Policy

- `test/data` is the single canonical input source for integration/e2e.
- `artifact_windows_cli` (physical folder: `suites/tracer_windows_rust_cli`) reads test input from `test/data` via suite config.
- Android runtime assets `input/full` are synced from `test/data` before build (`preBuild`).
- `unit/component` tests should prefer `test/fixtures/unit` and avoid depending on large datasets.

## CommandSpec 契约断言字段（TOML）

在 `[[commands]]` / `[[command_groups]]` 中可使用：

- `expect_stdout_contains = ["token", ...]`
- `expect_stdout_regex = ["(?im)^usage:.*", ...]`
- `expect_stdout_any_of = ["A", "B", "C"]`（至少命中一个）
- `expect_stderr_contains = ["token", ...]`
- `expect_error_code = "runtime.generic_error"`
- `expect_error_category = "runtime"`
- `expect_json_fields = [ ... ]`

`expect_json_fields` 支持：

- `"path.to.field"`：仅检查字段存在
- `"path.to.field::expected"`：检查字段值
- 支持数组下标：`"series[0].date"`、`"stats.values[2]::10"`

## Java env for Android build (Windows)

For Android Gradle build in this workspace, use Android Studio bundled JBR:

- `JAVA_HOME=C:\Application\Android\as\jbr`

Validation (PowerShell):

```powershell
$env:JAVA_HOME
java -version
```

Example verified output:

```text
C:\Application\Android\as\jbr
openjdk version "21.0.9" 2025-10-21
OpenJDK Runtime Environment (build 21.0.9+-14649483-b1163.86)
OpenJDK 64-Bit Server VM (build 21.0.9+-14649483-b1163.86, mixed mode)
```



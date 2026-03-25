# format

## 目的

通过 Python 统一入口调用格式化，不直接手写 `cmake --build ... --target format`
或 `cargo fmt`。

统一入口：

```powershell
python tools/run.py format ...
```

### 5. 直接按路径格式化

如果要直接覆盖仓库路径，而不是借某个 app 的 CMake `format` target：

```powershell
python tools/run.py format --paths libs apps/tracer_core_shell
python tools/run.py format --paths libs
python tools/run.py format --paths apps/tracer_core_shell

```

说明：

- 这条路径模式直接调用 `clang-format -style=file`
- 会递归扫描传入目录下的 C/C++ 源文件
- 会跳过常见 build/output 目录，例如 `build*`、`out/`

## 常用命令

### 1. CMake app 默认格式化

```powershell
python tools/run.py format --app tracer_core_shell
```

说明：

- 对 CMake backend，默认会执行：
  - `cmake --build <build_dir> --target format`
- 如果对应 `build_dir` 还没配置，工具会先自动 `configure`

### 2. 指定 build 目录

```powershell
python tools/run.py format --app tracer_core_shell --build-dir build_fast
```

### 3. 指定 profile

```powershell
python tools/run.py format --app tracer_core_shell --profile fast --build-dir build_fast
```

### 4. 自定义 CMake target

如果要跑自定义 target，例如只检查不落盘：

```powershell
python tools/run.py format --app tracer_core_shell --build-dir build_fast -- --target check-format
```

说明：

- `--` 后面的参数会原样透传给 `cmake --build`
- 一旦显式传了 `--target`，工具不会再自动补默认 `format`



### 6. 直接按路径检查格式

```powershell
python tools/run.py format --paths libs --check
python tools/run.py format --paths apps/tracer_core_shell --check
```

说明：

- `--check` 不会改文件
- 对 path mode，会等价于逐文件执行：
  - `clang-format --dry-run --Werror -style=file`
- 对 CMake app mode，如果没有显式传 `-- --target ...`，会默认改走 `check-format`

## Cargo backend

对 Cargo app，`format` 会自动改走 `cargo fmt`。

例如：

```powershell
python tools/run.py format --app tracer_windows_rust_cli
```

等价于在对应 app 目录执行：

```powershell
cargo fmt
```

## 适用边界

- `format` 适合做统一格式化入口
- 如果你需要 `clang-tidy` / `clang-analyzer`，用的是：
  - `python tools/run.py tidy ...`
  - `python tools/run.py analyze ...`
- 这几个命令不是一回事

## 参考

- `tools/toolchain/cli/handlers/quality/format.py`
- `tools/toolchain/commands/cmd_quality/format.py`
- `tools/README.md`

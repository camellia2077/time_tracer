# Windows Build Guide

本指南用于固化 Windows 构建策略，覆盖 `time_tracer` / `tracer_windows_cli` 的常用构建参数、推荐组合和回退路径。

## Command Policy

统一使用 Python 入口命令，不直接调用裸 `cmake`/`ninja`：

```bash
python scripts/run.py build ...
python scripts/run.py verify ...
```

## Core Switches

以下开关主要作用于 `apps/tracer_core`（`tracer_windows_cli` 通过 add_subdirectory 复用）：

1. `TT_USE_BUNDLED_SQLITE`
作用：Windows 下使用 SQLite amalgamation 源码内置编译，避免外部 SQLite DLL 依赖。
默认：Windows `ON`，其他平台 `OFF`。
2. `TT_TOML_HEADER_ONLY`
作用：Windows 下将 toml++ 作为 header-only 使用，避免 `libtomlplusplus-3.dll`。
默认：Windows `ON`，其他平台 `OFF`。
3. `TT_STATIC_MINGW_RUNTIME`
作用：MinGW 目标优先静态链接 `libgcc/libstdc++/winpthread`，减少运行时 DLL。
默认：Windows `ON`，其他平台 `OFF`。
4. `TT_ENABLE_EXPERIMENTAL_LTO`
作用：启用实验性 LTO/FTO 开关（默认关闭）。
默认：`OFF`。

## Recommended Profiles

1. `default`（稳定）
用途：日常开发/PR 快速验证。
命令：

```bash
python scripts/run.py verify --app tracer_windows_cli --profile fast --build-dir build_fast --concise
```

2. `optimized`（稳定发布）
用途：Windows 发布构建（bundled sqlite + header-only toml++ + static runtime）。
命令：

```bash
python scripts/run.py verify --app tracer_windows_cli --profile release_bundle --build-dir build_release --concise
```

3. `lto-experimental`（实验）
用途：仅用于试验 LTO 参数组合，建议允许失败，不阻塞主线。
命令：

```bash
python scripts/run.py verify --app tracer_windows_cli --profile release_bundle --build-dir build_lto_exp --concise --cmake-args=-DTT_ENABLE_EXPERIMENTAL_LTO=ON
```

## Rollback Playbook

任一开关都可以单独关闭，快速回退到稳定路径：

1. 关闭 bundled sqlite（回退系统 SQLite）：

```bash
python scripts/run.py build --app tracer_windows_cli --profile release_bundle --build-dir build_release --cmake-args=-DTT_USE_BUNDLED_SQLITE=OFF
```

2. 关闭 toml header-only（回退动态库）：

```bash
python scripts/run.py build --app tracer_windows_cli --profile release_bundle --build-dir build_release --cmake-args=-DTT_TOML_HEADER_ONLY=OFF
```

3. 关闭 MinGW 静态运行时（回退 runtime DLL）：

```bash
python scripts/run.py build --app tracer_windows_cli --profile release_bundle --build-dir build_release --cmake-args=-DTT_STATIC_MINGW_RUNTIME=OFF
```

4. 关闭实验 LTO：

```bash
python scripts/run.py build --app tracer_windows_cli --profile release_bundle --build-dir build_release --cmake-args=-DTT_ENABLE_EXPERIMENTAL_LTO=OFF
```

建议：每次只变更一个开关，并记录构建日志与二进制依赖变化。

## MinGW `std::print` / `stdc++exp` 兼容性结论（2026-02-24）

结论适用范围：Windows + MinGW（MSYS2 UCRT）工具链。

1. 在 `g++ 15.2.0` 下，`std::format` 可不依赖 `-lstdc++exp`。
2. 在 `g++ 15.2.0` 下，`std::print` / `std::println` 仍需 `-lstdc++exp`，否则会出现未定义符号：
   - `std::__open_terminal`
   - `std::__write_to_terminal`
3. 若代码仍使用 `<print>`（例如 `std::print/std::println`），当前不应移除 `-lstdc++exp`。

### 最小复现命令（用于后续复查）

```bash
g++ -std=c++23 format_only.cpp -o format_only.exe
g++ -std=c++23 print_only.cpp -o print_only.exe
g++ -std=c++23 print_only.cpp -o print_only_exp.exe -lstdc++exp
```

预期：
1. `format_only` 链接成功。
2. `print_only` 链接失败（缺 `__open_terminal` / `__write_to_terminal`）。
3. `print_only_exp` 链接成功。

### 何时重新评估是否移除 `stdc++exp`

1. GCC 版本升级（尤其主版本变更，例如 15 -> 16）。
2. MSYS2 UCRT 运行库升级（`libstdc++.a` / `libstdc++exp.a` 更新）。
3. 项目从 `std::print/std::println` 迁移到其他输出路径。

参考：
1. GCC 14 changes: https://gcc.gnu.org/gcc-14/changes.html
2. GCC-help (2025-05): https://gcc.gnu.org/pipermail/gcc-help/2025-May/143911.html
3. libstdc++ linkage docs: https://gcc.sourceware.org/onlinedocs/libstdc++/manual/using_dynamic_or_shared.html#manual.intro.using.linkage.experimental

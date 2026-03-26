# Windows Build Guide

本指南用于固化 Windows 构建策略，覆盖 `time_tracer` / `tracer_windows_cli` 的常用构建参数、推荐组合和回退路径。

## Command Policy

统一使用 Python 入口命令，不直接调用裸 `cmake`/`ninja`：

```bash
python tools/run.py build ...
python tools/run.py verify ...
```

## Windows Prerequisites

以下内容是在 Windows 宿主机上执行构建时的最小必需项，尤其适用于
`tracer_windows_rust_cli` 的 Rust + MSVC 链路。

### 必须安装

1. Visual Studio Build Tools 或 Visual Studio Community/Professional
要求：安装时包含 `使用 C++ 的桌面开发` 工作负载。

2. `适用于 x64/x86 的 MSVC 生成工具(最新版)`
作用：提供 `cl.exe`、正确的 `link.exe` 与对应的 MSVC 链接工具链。

3. `MSVC v143 - VS 2022 C++ x64/x86 生成工具`
作用：为当前仓库的 Windows Rust/MSVC 构建提供稳定、明确的 v143 工具集。

4. `Windows 11 SDK`
可选版本：`10.0.26100.7705` 或 `10.0.22621.0`，安装任意一个即可。
作用：提供 `kernel32.lib`、`ntdll.lib`、`ws2_32.lib`、`dbghelp.lib` 等
Windows import libraries。

5. `用于 Windows 的 C++ CMake 工具`
作用：提供 CMake/Windows C++ 工作流所需的 VS 侧集成。虽然 Rust CLI 直接
走 cargo，但本仓库的 Windows 构建经常与 core/runtime CMake 构建配套使用，
建议作为标准前置条件安装。

### 为什么这些是必须的

1. `tracer_windows_rust_cli` 当前使用 Rust `x86_64-pc-windows-msvc` 目标。
这条链路不是纯 GNU/MinGW 路线，最终链接依赖 MSVC linker 与 Windows SDK。

2. 如果缺少上述组件，常见失败特征包括：
   - `where cl` 找不到 `cl.exe`
   - `cargo build` 阶段提示找不到 `kernel32.lib`
   - Rust 链接阶段错误使用了非 MSVC 的 `link.exe`

### 环境检查

安装完成后，建议在 `Developer PowerShell for Visual Studio` 或
`x64 Native Tools Command Prompt for VS 2022` 中运行：

```powershell
where cl
where link
```

预期：

1. `cl.exe` 应指向 Visual Studio / MSVC 工具目录。
2. `link.exe` 应指向 Visual Studio / MSVC 工具目录。
3. 不应命中第三方路径（例如 `C:\masm32\bin\link.exe`）。

### 已知风险

1. 若系统 `PATH` 中存在 `C:\masm32\bin\link.exe` 等第三方 linker，并且排在
MSVC 之前，Rust `cargo` 构建可能会错误调用它，导致：
   - `LNK4044 unrecognized option`
   - `symbols.o : fatal error LNK1136`
   - `extra operand ...rcgu.o`

2. 遇到上述情况时，优先使用 Visual Studio 的开发者命令行重新执行构建，
或清理会劫持 `link.exe` 的 PATH 项。

## Core Switches

以下开关主要作用于 `apps/tracer_core_shell`（`tracer_windows_cli` 通过 add_subdirectory 复用）：

1. `TT_USE_BUNDLED_SQLITE`
作用：Windows 下使用 SQLite amalgamation 源码内置编译，避免外部 SQLite DLL 依赖。
默认：Windows `ON`，其他平台 `OFF`。
2. `TT_TOML_HEADER_ONLY`
作用：Windows 下将 toml++ 作为 header-only 使用，避免 `libtomlplusplus-3.dll`。
默认：Windows `ON`，其他平台 `OFF`。
3. `TT_STATIC_MINGW_RUNTIME`
作用：MinGW 目标优先静态链接 `libgcc/libstdc++/winpthread`，减少运行时 DLL。
默认：Windows `ON`，其他平台 `OFF`。
4. `TT_ENABLE_LTO`
作用：启用显式 LTO/FTO 路径（默认关闭）。
默认：`OFF`。

## Recommended Profiles

1. `optimized`（稳定发布）
用途：Windows 发布构建（bundled sqlite + header-only toml++ + static runtime）。
命令：

```bash
# 先构建 core runtime DLL（windows/build/bin）
python tools/run.py build --app tracer_core --profile release_bundle --build-dir build --runtime-platform windows

# 再基于 windows/build/bin 编译 Rust CLI
python tools/run.py build --app tracer_windows_rust_cli --profile release_bundle --build-dir build --runtime-platform windows
```

2. `lto`（正式可选）
用途：显式启用 LTO 验证路径；默认 profile 仍保持关闭。
命令：

```bash
# C++ 轨专用（显式 LTO 开关走 CMake）
python tools/run.py verify --app tracer_core_shell --profile release_bundle_ci_no_pch --build-dir build_lto --concise --cmake-args=-DTT_ENABLE_LTO=ON
```

## Rollback Playbook

任一开关都可以单独关闭，快速回退到稳定路径：

1. 关闭 bundled sqlite（回退系统 SQLite）：

```bash
# C++ 轨专用（CMake 开关）
python tools/run.py build --app tracer_windows_cli --profile release_bundle --build-dir build --cmake-args=-DTT_USE_BUNDLED_SQLITE=OFF
```

2. 关闭 toml header-only（回退动态库）：

```bash
# C++ 轨专用（CMake 开关）
python tools/run.py build --app tracer_windows_cli --profile release_bundle --build-dir build --cmake-args=-DTT_TOML_HEADER_ONLY=OFF
```

3. 关闭 MinGW 静态运行时（回退 runtime DLL）：

```bash
# C++ 轨专用（CMake 开关）
python tools/run.py build --app tracer_windows_cli --profile release_bundle --build-dir build --cmake-args=-DTT_STATIC_MINGW_RUNTIME=OFF
```

4. 关闭 LTO：

```bash
# C++ 轨专用（CMake 开关）
python tools/run.py build --app tracer_core_shell --profile release_bundle_ci_no_pch --build-dir build --cmake-args=-DTT_ENABLE_LTO=OFF
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

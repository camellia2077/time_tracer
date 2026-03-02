# 关于是否引入 vcpkg 的评估报告

## 1. 当前项目的依赖管理现状

通过分析 `time_tracer` 的 CMake 脚本 (`HostDependencies.cmake`, `AndroidDependencies.cmake`) 以及 `README.md`，目前项目的依赖管理主要基于 **“系统包管理 (MSYS2) + CMake FetchContent”** 的混合模式：

*   **Host (Windows/端侧环境)**:
    *   **SQLite3**: Windows 默认开启 `TT_USE_BUNDLED_SQLITE`，通过 `FetchContent` 下载 amalgamation 源码编译。
    *   **tomlplusplus**: Windows 默认开启 `TT_TOML_HEADER_ONLY`，通过 `FetchContent` 下载 release tar 包。
    *   **nlohmann/json**: 强制依赖系统安装（`find_package REQUIRED`），在 Windows 下依赖 MSYS2 的 `pacman -S mingw-w64-ucrt-x86_64-nlohmann-json`。
    *   **libsodium**: 尝试 `find_package`，失败后回退至 `FetchContent` 从 Github (robinlinden/libsodium-cmake) 拉取 CMake 封装。
    *   **zstd**: 尝试 `find_package`，失败后回退至 `FetchContent` 拉取官方库子目录构建。
*   **Android (交叉编译)**:
    *   几乎完全依赖 `FetchContent` 从源码拉取所有 C++ 依赖（SQLite, tomlplusplus, json, libsodium, zstd），利用 NDK 工具链与项目一同源码编译。
*   **Rust / Python**:
    *   拥有独立的包管理器（Cargo, pip），与 C++ 构建相对隔离，运作良好。

## 2. 引入 `vcpkg` 的优势 (Pros)

1.  **统一且标准化的依赖获取**: 完全淘汰复杂的 `FetchContent` 脚本和 `find_package` 回退逻辑（`HostDependencies.cmake` 中的大量 fallback 代码可以删减 80%），统一使用 `vcpkg.json` (Manifest 模式) 声明 `sqlite3`, `nlohmann-json`, `tomlplusplus`, `libsodium`, `zstd`。
2.  **降低环境配置门槛**: 开发者只需安装 C++ 编译器和 CMake。目前的文档要求 Windows 开发者必须进入 MSYS2 并手动 `pacman` 安装部分依赖。引入 vcpkg 后，CMake 在 configure 阶段即可自动拉取和编译所有缺失的 C++ 端依赖，实现真正的“开箱即用”。
3.  **更好的多平台/原生 MSVC 支持**: 目前项目深度绑定 MSYS2 UCRT64。如果未来希望提供纯原生 Windows MSVC 的支持，vcpkg 是最完美的桥梁，它的 MSVC triplet 生态最成熟。
4.  **版本锁定与可重复构建**:通过 `vcpkg.json` 和对应的 `vcpkg-configuration.json` 能够精确锁定各个第三方库的版本（Baseline），比 `FetchContent` 更安全可靠。

## 3. 引入 `vcpkg` 的劣势与风险 (Cons)

1.  **MinGW 兼容性踩坑风险**: 
    *   目前项目主推 `MSYS2 UCRT64` (GCC 13+ / Clang 16+)。`vcpkg` 虽然支持 MinGW triplet（例如 `x64-mingw-dynamic` / `x64-mingw-static`），但在某些包含复杂构建系统的库上，vcpkg 下的 MinGW 编译容易出现诡异的问题。
    *   同时，`libsodium` 和 `zstd` 在 vcpkg 的 mingw 环境中需要仔细测试是否能顺利编过。
2.  **构建时间增加**: `vcpkg` 会在初次配置时编译所有依赖。虽然 `FetchContent` 也是源码编译，但目前 `nlohmann_json` 是从 MSYS2 安装的预编译包，引入 vcpkg 会增加初次拉取和编译的时间（可以使用 vcpkg binary cache 缓解）。
3.  **Android 构建复杂化**: 
    *   当前 Android 端使用 `FetchContent` 非常顺滑，它能自动继承 CMake 的 NDK toolchain。
    *   若强行在 Android 端切换为 vcpkg，需要配置对应的 Android custom triplets 或整合 vcpkg 的 NDK 支持，容易破坏现有的 Android 构建流水线。如果是 **Host 用 vcpkg，Android 维持 FetchContent**，则又违背了“统一管理”的初衷，反而需要维护两套逻辑。

## 4. 评估结论与建议 (Conclusion)

**综合判断：现在引入 vcpkg 是“有价值的，但不是紧急且必须的”，需要权衡投入产出比。**

### 建议的分阶段策略：

如果你对现状（MSYS2 pacman + FetchContent）觉得维护成本还在可控范围，并以 Android 和 MSYS2 为核心目标，**可以暂缓全面引入 vcpkg**。

**如果你希望提升跨平台移植性（尤其是解耦 MSYS2，拥抱原生 Windows MSVC）：**
非常建议引入 vcpkg。可以采用以下实施方案：
1. **Host 端改造**：为主机端 CMake 引入 `vcpkg.json` Manifest 机制。在 `CMakeLists.txt` 判断：若使用了 vcpkg toolchain，则直接 `find_package()` 这五大核心依赖；否则回退到当前的 `HostDependencies.cmake`。
2. **渐进式替换**：不删除现有的 `AndroidDependencies.cmake`，因为针对 Android NDK 的跨端编译，直接将其作为 CMake 源码子树（FetchContent）反而是目前最无痛的办法。
3. **保留 UCRT64 测试**：确保在使用 vcpkg 提供依赖的前提下，项目的 CI 和本地 `python scripts/run.py` 使用 MinGW 依然能顺畅跑通。

**下一步行动确认：**
你希望我们接下来尝试**为 Host 构建引入 `vcpkg.json` 并改造 CMake 脚本**吗（在保证不破坏现有 Android 编译和 Python 构建流的前提下）？

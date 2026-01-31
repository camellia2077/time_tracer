# 开发环境与构建指南 (Build Guide)

本指南详细介绍了如何搭建开发环境以及如何从源码编译 **TimeTracer**。

## 1. 环境准备

### 1.1 编译器要求
*   **标准**: 必须支持 C++23 标准。
*   **Windows**: 推荐使用 MSVC 2022 (v143) 或更高版本。
*   **Linux**: 推荐使用 GCC 13+ 或 Clang 16+。

### 1.2 外部依赖
项目依赖于以下轻量级开源库：
*   **`sqlite3`**: 负责数据持久化存储。
*   **`toml++`**: 负责解析 TOML 格式的配置文件。
*   **`nlohmann/json`**: 用于中间件数据及插件系统的序列化。

## 2. 项目目录结构

*   **`apps/time_tracer`**: 主应用程序 (CLI) 的源代码目录。
*   **`my_test/test_executables`**: 基于 Python 的自动化集成测试套件。
*   **`scripts/`**: 用于辅助构建与环境准备的脚本。

## 3. 编译步骤

### 3.1 使用快速构建脚本 (推荐)
对于 Windows 用户，建议使用提供的 Shell 脚本进行一键编译：

```powershell
./time_tracer_cpp/apps/time_tracer/scripts/build_fast.sh
```

该脚本将自动完成以下流程：
1.  调用 CMake 生成构建文件。
2.  编译核心应用程序。
3.  编译所有报表格式化驱动 (DLL 插件)。
4.  将生成的插件自动同步至 `plugins/` 目录。

### 3.2 手动 CMake 构建
若需手动控制构建过程：
```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release
```

## 4. 运行验证

项目包含一套完善的回归测试系统，用于验证 CLI 命令与报表生成逻辑的正确性。

```bash
python ./my_test/test_executables/run.py
```

**提示**: 请确保在 `my_test/test_executables/config.toml` 中正确配置了编译后的 EXE 路径及 `plugins/` 文件夹位置。
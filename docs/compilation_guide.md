# 编译指南

本指南将引导您使用 MSYS2 UCRT64 环境在 Windows 上成功编译 `Time_Master` 及其相关组件。

### 1\. 安装 MSYS2 UCRT64 环境 (推荐)

MSYS2 是为 Windows 操作系统设计的强大工具链和包管理系统，我们推荐使用其 UCRT64 子系统以获得最佳的兼容性和性能。

  * **步骤 1: 下载安装程序**
    访问 MSYS2 的官方网站：[https://www.msys2.org/](https://www.msys2.org/)，下载并运行最新的 x86\_64 安装程序。

  * **步骤 2: 执行首次系统更新**
    安装完成后，打开 **MSYS2 UCRT64** 终端（而不是 MSYS 或 MINGW64），并执行以下命令来更新软件包数据库和核心包。请严格执行，这非常重要。

    ```bash
    pacman -Syu
    ```

    *注意：更新过程中，它可能会提示您关闭终端再重新打开来完成更新。请按照它的指示操作，这是正常现象。*

### 2\. 安装编译所需的工具链和库

在 MSYS2 UCRT64 终端中，依次执行以下命令来安装编译本项目所需的所有依赖。

  * **步骤 1: 安装 UCRT64 C++ 工具链**
    这将安装 GCC 编译器、make 等核心构建工具。

    ```bash
    pacman -S mingw-w64-ucrt-x86_64-toolchain
    ```

  * **步骤 2: 安装 CMake**
    项目使用 CMake 来管理和自动化构建过程。

    ```bash
    pacman -S mingw-w64-ucrt-x86_64-cmake
    ```

  * **步骤 3: 安装 nlohmann-json 库**
    这是我们用来解析 `config.json` 文件的库。

    ```bash
    pacman -S mingw-w64-ucrt-x86_64-nlohmann-json
    ```

    *`SQLite3` 库通常会作为 `toolchain` 的一部分被附带安装，一般无需单独安装。*

### 3\. 编译项目

完成以上所有准备工作后，您就可以编译项目了。

  * **步骤 1: 克隆项目**
    如果您还未下载项目，请使用 git 克隆：

    ```bash
    git clone <your-repository-url>
    cd <project-directory>
    ```

  * **步骤 2: 运行构建脚本**
    在项目根目录下，直接运行我们为您准备好的构建脚本。

    ```bash
    ./build.sh
    ```

    该脚本会自动创建一个 `build` 目录，在其中运行 `cmake` 和 `make` 命令，最终在 `build` 目录下生成 `time_tracker_cli.exe` 和 `time_tracker_app.exe` 两个可执行文件。
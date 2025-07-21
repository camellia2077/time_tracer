# Log Generator - 使用指南

`log_generator` 是一个C++编写的测试数据生成器，用于快速创建符合格式的日志（`.txt`）文件，方便对 `Time_Master` 主程序进行测试和验证。

## 程序结构

```
log_generator/
├── activities_config.json  # 配置文件，定义可生成的活动及其权重
├── Config.h                # 配置模块的头文件 (定义数据结构, 声明加载函数)
├── Config.cpp              # 配置模块的源文件 (实现加载函数)
├── LogGenerator.h          # 核心逻辑模块的头文件 (定义LogGenerator类)
├── LogGenerator.cpp        # 核心逻辑模块的源文件 (实现LogGenerator类)
├── Utils.h                 # 工具类头文件，仅含声明
└── main.cpp                # 主文件 (包含Application类, Utils实现和main函数)
```

## 编译指南

本项目使用 MSYS2 UCRT64 环境进行编译。

1.  **环境准备**: 请确保您已按照主项目的 **[编译指南](https://www.google.com/search?q=../compilation_guide.md)** 完成了 MSYS2 环境的配置和工具链的安装。

2.  **执行编译**:
    在 `log_generator` 的根目录下，运行为您准备好的构建脚本：

    ```bash
    ./build.sh
    ```

    脚本将自动完成编译，并在 `build` 目录下生成可执行文件 `log_generator.exe`。

## 使用方法

`log_generator` 是一个命令行工具，接收三个参数来生成指定范围和密度的日志数据。

### 命令格式

```
log_generator.exe <start_year> <end_year> <items_per_day>
```

### 参数说明

  * `<start_year>`: 生成日志的起始年份 (例如: `2024`)。
  * `<end_year>`: 生成日志的结束年份 (包含此年份)。
  * `<items_per_day>`: 每天生成的日志条目数量 (正整数)。

### 示例

生成从2023年到2025年，每天10条日志记录的测试数据：

```bash
./build/log_generator.exe 2023 2025 10
```

程序会读取 `activities_config.json` 中的配置，并根据定义的活动和权重，在程序运行目录下创建一个名为 `Generated_logs` 的文件夹，其中包含了按年份和月份组织好的 `.txt` 日志文件。
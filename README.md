# 依赖项 (Dependencies)

本项目依赖于以下优秀的开源库：
* **[SQLite C Library](https://www.sqlite.org/index.html)**
    * **用途**: C++ 部分的代码直接使用 SQLite C API 进行数据库操作。
    * **许可证**: Public Domain

* **[nlohmann/json](https://github.com/nlohmann/json)**
    * **用途**: 用于项目中所有 JSON 格式数据的解析和序列化（例如加载 `Validator_Config.json` 配置文件）。
    * **许可证**: MIT License

* **[json (Python standard library)](https://docs.python.org/3/library/json.html)**
    * **用途**: 用于加载 `generate_report.json` 配置文件，以及处理项目中涉及到的 JSON 格式数据。
    * **许可证**: Python Software Foundation License (PSF)

* **[sqlite3 (Python standard library)](https://docs.python.org/3/library/sqlite3.html)**
    * **用途**: 用于连接到 `bills.db` SQLite 数据库，并执行 SQL 查询以获取账单数据。
    * **许可证**: Python Software Foundation License (PSF)

* **[Matplotlib](https://matplotlib.org/)**
    * **用途**: 用于生成柱状图，可视化父级支出的汇总数据。
    * **许可证**: Matplotlib License (BSD-style)
# 1 Time_Master 
用于解析文本内容，存入数据库，查询数据库
## 1.1 structure
```
Time_Master/
├── CMakeLists.txt
├── main.cpp # 交互式
├── main_cli.cpp # 命令行传入
│
├── action_handler/ # 所有功能的封装
│   ├── action_handler.cpp
│   └── action_handler.h
│
├── common/ # Contains shared utilities and data structures used throughout the application.
│   ├── common_utils.cpp
│   ├── common_utils.h
│   └── version.h # for version info
│
├── config/ # 用于存放json配置文件
│   ├── config.json # 用于定于父项目的映射
│   ├── format_validator_config.json # 检验转化后项目名称合法性
│   └── interval_processor_config.json # 转化规则
│
├── db_inserter/ 
│   ├── DataImporterr.cpp  # 封装解析数据与插入数据库      
│   └── DataImporterr.h           
│   └── inserter/
│   │   ├── DatabaseInserter.cpp    # 数据库插入
│   │   └── DatabaseInserter.cpp    # 数据库插入
│   └── model/
│   │   └── time_sheet_model.h      # 共享的结构
│   └── parser/
│       ├── ConfigLoader.h       
│       ├── ConfigLoader.cpp        # 加载配置
│       ├── DataFileParserr.h       
│       ├── DataFileParser.cpp      # 解析数据 
│       ├── DataFileParser.h   
│       └── ParserConfig.h         # 读取json配置的父项目映射
│
├── file_handler/ # 读取json配置，递归文件夹查询文件
│   ├── config_loader.cpp # 加载json
│   └── config_loader.h 
│   ├── file_controller.cpp # 封装
│   └── file_controller.h
│   ├── file_utils.cpp # 递归查询
│   └── file_utils.h
│
├── time_master_app/                
│   ├── menu.h           
│   └── menu.cpp         
│
├── queries/             # Contains all logic for querying the database and generating user-facing reports.
│   ├── QueryHandler.cpp    # Implements the QueryHandler class, which acts as a simple interface (Facade) to 
│   └── QueryHandler.h      # Declares the QueryHandler class, the main entry point for all query operations.
│   └── report_generators/
│       ├── query_utils.cpp      # Implements shared helper functions for querying.
│       └── query_utils.h        # Declares various utility functions used by the different query generators.
│       ├── DailyReportQuerier.cpp        # Implements the logic to generate a detailed report for a single day.
│       └── DailyReportQuerier.h          # Declares the class responsible for generating a daily report.
│       ├── MonthlyReportQuerier.cpp     
│       └── MonthlyReportQuerier.h        
│       ├── PeriodReportQuerier.cpp     # Implements the logic for generating reports over a custom period.
│       └── PeriodReportQuerier.h       # Declares the class for generating reports over a specific time period.
│
├── reprocessing/               # 预处理
│   ├── IntervalProcessor.cpp
│   └── IntervalProcessor.h
│   └── input_transfer/ # 转换验证后的输入文件
│   │   ├── LogProcessor.cpp
│   │   ├── LogProcessor.h
│   └── validator/
│       ├── OutputFileValidator.cpp # 转换后文件格式检验
│       └── OutputFileValidator.h 
│       ├── SourceFileValidator.cpp # 源文件格式检验
│       └── SourceFileValidator.h 
│       ├── ValidatorUtils.cpp
│       └── ValidatorUtils.h
│
└── resources/
    ├── app_icon.rc
    └── output_icon.ico
```

## 1.2 命令行使用方法
注意程序要在powershell或cmd中运行
### 1.2.1 基本命令格式
```bash
time_tracker_command <command> [arguments]
```
### 1.2.2 可用命令
| 序号 | 短标签 | 长标签 | 功能描述 |
|---|---|---|---|
| 1 | `-vs <path>` | `--validate-source <path>` | 仅检验源文件的格式 |
| 2 | `-c <path>` | `--convert <path>` | 仅转换文件格式 |
| 3 | `-vo`  | `--validate-output` | 转换后检验输出文件 (需与 `-c` 或 `-a` 配合) |
| 4 | `-a <path>` | `--all <path>` | 执行完整流程(检验源->转换->检验输出) |
| 5 | `-edc`  | `--enable-day-check`  | 启用对月份天数完整性的检查 |
| 6 | `-p <filepath>` | `--process <filepath>` | 解析单个已格式化的txt文件并导入数据库 |
| 7 | `-q d <YYYYMMDD>` | `--query daily <YYYYMMDD>` | 查询指定日期的统计数据 |
| 8 | `-q p <days>` | `--query period <days>` | 查询过去指定天数的统计数据 |
| 9 | `-q m <YYYYMM>` | `--query monthly <YYYYMM>` | 查询指定月份的统计数据 |
| 10 | `-h` | `--help` | 查看此使用帮助 |
| 11 | `-v` | `--version` | 查看程序版本和更新日期 |

### 1.2.3 示例



## 1.3 使用msys2 UCRT64环境进行编译
0. 下载并安装 MSYS2
MSYS2 是为 Windows 操作系统 设计的

访问 MSYS2 的官方网站：https://www.msys2.org/

选择安装程序：

Download the installer: msys2-x86_64-20250622.exe
1. 执行首次更新
```bash
pacman -Syu
```
2. 安装 UCRT64 开发工具
```bash
pacman -S mingw-w64-ucrt-x86_64-toolchain 
```

3. 安装 CMake
项目使用 CMake 来管理构建过程。继续在 MINGW64 终端中输入以下命令来安装它
```bash
pacman -S mingw-w64-ucrt-x86_64-cmake 
```

4. 安装 nlohmann-json 库
```bash
pacman -S mingw-w64-x86_64-nlohmann-json
```

5. 在环境中运行 build.sh
```bash
./build.sh
```

# 2 log_tool 数据预处理
数据预处理与合法性检验
## 2.1 structure
```
process/
├── main.cpp                 # Main program entry point, handles argument parsing, orchestrates file processing and validation.
│ 
├── IntervalProcessor.h      # Header for IntervalProcessor class, declares the class and its structures for processing raw input files into a standardized format.
│ 
├── IntervalProcessor.cpp    # Implementation of IntervalProcessor, contains logic for parsing, mapping, and transforming event data.
│ 
├── FormatValidator.h        # Header for FormatValidator class, declares the class and its structures for validating the format of processed files.
│ 
├── FormatValidator.cpp      # Implementation of FormatValidator, contains logic for checking various format rules and identifying errors.
│ 
└── SharedUtils.h            # Header for sharing ANSI color codes.
```
## 2.2 JSON 配置文件
### 2.2.1 header_format.json

此文件用于定义输出文件中标题头的顺序。

**文件路径**: `configs/header_format.json`

**结构**:
* `header_order`: 一个字符串数组，其中的每个元素都必须与输出文件中的标题头完全对应（包括冒号）。

**使用方法**: 您可以调整数组中元素的顺序来改变输出文件中标题头的排列。

**注意**: `Date:` 必须始终是第一个元素。`Getup:` 和 `Remark:` 必须存在于数组中。

**示例**:
```json
{
  "header_order": [
    "Date:",
    "Status:",
    "Sleep:",
    "Getup:",
    "Remark:"
  ]
}
```
### 2.2.2 interval_processor_config.json
此文件控制着 IntervalProcessor 如何转换和映射原始日志中的活动。

**文件路径**: configs/interval_processor_config.json

**结构**:

* `text_mappings`: 一个键值对集合，用于将原始日志中的中文或自定义缩写映射为标准化的项目路径。

**键**: 原始日志中的文本（例如 "单词" 或 "听力"）。

**值**: 希望转换成的标准项目路径（例如 "study_english_words"）。
* `duration_mappings`: 一个对象，用于定义基于活动持续时间的动态映射规则。

**键**: 一个基础项目路径（必须已在 text_mappings 中定义过，例如 "insomnia"）。

**值**: 一个规则数组，每个规则对象包含：

**less_than_minutes**: 时间阈值（分钟）。

**value**: 当活动持续时间小于此阈值时，最终采用的项目路径。

**使用方法**:

要添加新的文本映射，只需在 text_mappings 对象中新增一行键值对。

要为某个活动（如 "饭"）添加基于时长的分类，先确保它在 text_mappings 中有基础映射（例如 "饭": "meal"）

然后在 duration_mappings 中新增一个以 "meal" 为键的规则数组。程序会按从小到大的顺序检查阈值。

**示例**:
```json
{
    "text_mappings": {
        "单词": "study_english_words",
        "饭": "meal"
    },
    "duration_mappings": {
        "meal": [
            { "less_than_minutes": 25, "value": "meal_short" },
            { "less_than_minutes": 35, "value": "meal_medium" },
            { "less_than_minutes": 60, "value": "meal_long" }
        ]
    }
}
```

### 2.2.3 format_validator_config.json

此文件为 FormatValidator 定义了项目层级关系和分类，用于合法性检验。

**文件路径**: configs/format_validator_config.json

**结构**:
* `PARENT_CATEGORIES`: 一个对象，定义了所有项目的父子层级关系。

**键**: 父类别的名称（例如 "study", "code", "recreation"）。

**值**: 一个字符串数组，包含了所有属于该父类别的子项目标准路径。

**使用方法**:

当您在 interval_processor_config.json 中定义了一个新的标准项目路径后，您应该将这个新路径添加到 PARENT_CATEGORIES 中对应父类别的数组里。

例如，如果您新增了一个活动 recreation_game_new, 您需要将其添加到 "recreation" 类别的数组中。

这确保了合法性检验器能够正确识别所有活动都属于一个已知的类别。

**示例**:
```json
{
  "PARENT_CATEGORIES": {
    "recreation": [
      "recreation_game_clash-royale",
      "recreation_game_overwatch",
      "recreation_bilibili"
    ],
    "study": [
      "study_english_words",
      "study_math_calculus"
    ]
  }
}
```

## 2.3 log_tool使用方法
1. 输入路径为文件夹
2. 启用`-edc`选项会严格检查月份天数完整性

### 2.3.1 基本命令格式
```bash
./log_tool ]<executable_name> <flag> <file or folder path"> [options]
```

### 2.3.2 功能标志
| 序号 | 长标签 | 短标签 | 功能描述 |
|---|---|---|
| 1 | `--convert <filepath>` | `-c` | 转换文件 |
| 2 | `--validate-source <filepath>` | `-vs`| 检验源文件 |
| 3 | `--validate-output <filepath> ` | `-vo` | 检验输出文件 |
| 4 | `--all <filepath> ` | `-a` | 依次执行 -vs (检验源)、-c (转换) 和 -vo (检验输出) |
| 4 | `--enable-day-check` | `-edc` | 启用月份完整性检查 |
| 5 | `--version` | `-v` | 查看版本号 |
| 6 | `--help` | `-h` | 帮助 |


### 2.3.3 配置文件
程序依赖以下配置文件（需放在同级目录）：
- `interval_processor_config.json` - 间隔转换配置
- `format_validator_config.json` - 格式验证配置
- `header_format.json` - 文件头格式顺序配置

### 2.3.4 使用示例


#### 示例1:仅转换文件(序号1)
1. 输入命令
```bash
./log_tool log_tool.exe -p /path/to/input.txt
```
2. 输出文件
`processed_input.txt`

#### 示例2:仅验证文件，不开启日期完整性检测(序号2)




# 3 graph_generator 图表生成
读取数据库并且生成图表
## 3.1 structure 程序结构
```
graph_generator/
├── main.py                     # 命令行程序入口
├── main_input.py               # input交互
├── db_access.py                # 数据库查询
|
├── configs/
│   ├── heatmap_colors.json     # 热力图颜色配置
│   └── timeline_colors.json    # 时间线和柱状图颜色配置
|
└── modules/
    ├── day_analyzer.py         # 负责处理“逻辑日”数据 (从 timeline_generator 提取)
    ├── heatmap_generator.py    # 通用化的热力图生成器 (合并了旧的 heatmap 和 bool_generator)
    └── plotters.py             # 包含所有基于 matplotlib 的绘图类 (时间线和柱状图)
```
## 3.2 commond 命令总览
1. timeline图表生成
2. 柱状图生成
3. 项目热力图生成
4. 睡眠bool状态生成
## 3.3 基本命令格式
```bash
python main.py <command> [arguments]
```
> **注意**：程序需要在 PowerShell 或 CMD 中运行
> 
## 3.4 可用命令
| 序号 | 命令格式 | 功能描述 |
|------|----------|----------|
| 1 | `timeline <YYYYMMDD>` | 为指定日期生成时间线图 |
| 2 | `barchart <YYYYMMDD>` | 为指定日期生成活动时长柱状图 |
| 3 | `heatmap <year> [-p PROJECT]` | 生成项目热力图（年度+月度） |
| 4 | `sleep <year>` | 生成睡眠状态热力图（年度+月度） |
| 5 | `-h, --help` | 查看使用帮助 |
| 6 | `-v, --version` | 查看程序版本 |

## 3.5 使用示例
### 3.5.1 生成时间线图（命令1）
```bash
python main.py timeline 20250624
```
### 3.5.2 生成柱状图（命令2）
```bash
python main.py barchart 20250624
```
### 3.5.3 项目热力图（命令3）
#### 默认生成项目(mystudy)
```bash
python main.py heatmap 2025
```
#### 指定项目meal
```bash
python main.py heatmap 2025 -p meal
```
### 3.5.4 睡眠bool状态热力图（命令4）
```bash
python main.py sleep 2025
```
### 3.5.4 查看帮助（命令5）
```bash
python main.py -h
```
### 3.6 查看版本（命令6）
```bash
python main.py -v
```







# ４ log_generator 日志生成
txt生成器
## 4.1 structure
/project-root
├── activities_config.json //配置
├── Config.h              // 配置模块的头文件 (定义数据结构, 声明加载函数)
├── Config.cpp            // 配置模块的源文件 (实现加载函数)
├── LogGenerator.h        // 核心逻辑模块的头文件 (定义LogGenerator类)
├── LogGenerator.cpp      // 核心逻辑模块的源文件 (实现LogGenerator类)
├── Utils.h               // 【必需的】工具类头文件，仅含声明
└── main.cpp              // 主文件 (包含Application类, Utils实现和main函数)
## 4.2 使用msys2环境进行编译
1. 执行首次更新
```bash
pacman -Syu
```
2. 安装 MinGW-w64 工具链
```bash
pacman -S mingw-w64-x86_64-toolchain
```

3. 安装 CMake
项目使用 CMake 来管理构建过程。继续在 MINGW64 终端中输入以下命令来安装它
```bash
pacman -S mingw-w64-x86_64-cmake
```

4. 安装 nlohmann-json 库
```bash
pacman -S mingw-w64-x86_64-nlohmann-json
```

5. 在环境中运行 build.sh
```bash
./build.sh
```
## 4.3 usage
```
Description: Generates test log data for a given year range. Reads activities from 'activities_config.json'.
  <start_year>      : The starting year (e.g., 1990).
  <end_year>        : The ending year (inclusive).
  <items_per_day>   : Number of log items per day (positive integer).
  --version         : Display version information and exit.
```
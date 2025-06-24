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
主程序，用于解析文本内容，存入数据库，查询数据库
## 1.1 structure
```
db/
├── CMakeLists.txt
├── main.cpp
├── main_command
│
├── common/ # Contains shared utilities and data structures used throughout the application.
│   ├── common_utils.cpp
│   └── common_utils.h
├── database/ # Manages the creation of the database and the importation of data into it.
│   ├── database_importer.cpp
│   └── database_importer.h
├── menu/                # UI components
│   ├── menu.h           # Header for the Menu class
│   └── menu.cpp         # Implementation of the Menu class
├── parsing/                    # Handles data parsing from input files.
│   ├── data_parser.cpp         # Implements the DataFileParser for parsing files and populating data structures.
│   └── data_parser.h           # Defines the DataFileParser class and data structures for parsed data.
├── processing/                 # Manages the file processing workflow.
│   ├── processing.cpp          # Implements file collection, parsing, and database import.
│   └── processing.h            # Declares the main function to start file processing.
├── queries/             # Contains all logic for querying the database and generating user-facing reports.
│   ├── query_handler.cpp    # Implements the QueryHandler class, which acts as a simple interface (Facade) to route query requests.
│   └── query_handler.h      # Declares the QueryHandler class, the main entry point for all query operations.
│   ├── query_day.cpp        # Implements the logic to generate a detailed report for a single day.
│   └── query_day.h          # Declares the class responsible for generating a daily report.
│   ├── query_month.cpp      # Implements the logic to generate a summary report for a specific month.
│   └── query_month.h        # Declares the class that generates a monthly report.
│   ├── query_period.cpp     # Implements the logic for generating reports over a custom period (e.g., last 7 days).
│   └── query_period.h       # Declares the class for generating reports over a specific time period.
│   ├── query_utils.cpp      # Implements shared helper functions for querying, such as building project trees and formatting output.
│   └── query_utils.h        # Declares various utility functions used by the different query generators.
└── resources/
    ├── app_icon.rc
    └── output_icon.ico
```
## 1.2 命令行使用方法
### 1.2.1 基本命令格式
```bash
time_tracker_command <command> [arguments]
```


### 1.2.2 可用命令
| 命令序号 | 标签 | 功能描述 |
|---|---|---|
| 1 |`-p,process <filepath>`|解析txt并导入数据|
| 2 |`-q d,-query daily <YYYYMMDD>`|查询指定日期的统计数据|
| 3 |`-q p,-query period <days>`|查询过去指定天数的统计数据|
| 4 |`-q m,-query monthly <YYYYMM>`|查询指定月份的统计数据|
| 5 |`-h,--help`|查看使用帮助|
| 6 |`--version`|查看程序版本|

### 1.2.3 示例
1.  传入文本
    1.1 传入文件夹下的全部文本 
    ```bash
      ./Time_Master -p,process C:\Date
    ```
    1.2 传入单个txt文件
    ```bash
    ./Time_Master -p,process C:\Date\2024\2024_10.txt
    ```
2. 查询数据库中指定日期的统计数据
`./Time_Master -q d,-query daily 20250523`
3. 查询过7天数的统计数据
`./Time_Master -q p,-query period 7`
查询过30天数的统计数据
`./Time_Master -q p,-query period 30`


| 2 |` <YYYYMMDD>`|查询指定日期的统计数据|
| 3 |`-q p,-query period <days>`|查询过去指定天数的统计数据|
| 4 |`-q m,-query monthly <YYYYMM>`|查询指定月份的统计数据|
| 5 ||`-h,--help`|查看使用磅数|
| 6 ||`--version`|查看程序版本|





## 1.2 compile

在msys64中运行build.bat

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

## 2.3 log_tool使用方法 log_tool usage

### 2.3.1 基本命令格式
```bash
./log_tool log_tool.exe <flag> <文件夹或文件路径> [options]
```

### 2.3.2 功能标志(必选)
|标签|功能描述|
|-----|--------|
|`-p`|仅转换文件|
|`-v`|仅验证输入文件|
|`-pv`|仅转换文件并且验证转化后文件的合法性|

### 2.3.3 参数说明
|参数|描述|是否必须|
|-----|------|--------|
|`<文件夹或文件路径>`|文件夹或文件路径|是|
|`-edc,--enable-day-check`|启用月份完整性检查|否|

### 2.3.4 配置文件
程序依赖以下配置文件（需放在同级目录）：
- `interval_processor_config.json` - 间隔转换配置
- `format_validator_config.json` - 格式验证配置
- `header_format.json` - 文件头格式顺序配置

### 2.3.5 使用示例

#### 示例1：仅转换文件
```bash
./log_tool log_tool.exe -p /path/to/input.txt
```

输出文件：`processed_input.txt`

#### 示例2：仅验证文件，不开启日期完整性检测
```bash
./log_tool log_tool.exe -v /path/to/input.txt
```
输出：控制台显示验证结果，错误内容写入`validation_errors.txt`

#### 示例3：仅验证文件，开启日期完整性检测
```bash
./log_tool log_tool.exe -v /path/to/input.txt -edc
```
输出：控制台显示验证结果，错误内容写入`validation_errors.txt`

#### 示例4：转换并验证，开启日期完整性检测
```bash
./log_tool log_tool.exe -pv /path/to/folder -edc
```
输出文件：
- 验证成功：`final_[原文件名]`
- 验证失败：`error_validation_[原文件名]`
- 错误日志：`validation_errors.txt`

#### 示例4：转换并验证，不开启日期完整性检测
```bash
./log_tool log_tool.exe -pv /path/to/folder
```
输出文件：
- 验证成功：`final_[原文件名]`
- 验证失败：`error_validation_[原文件名]`
- 错误日志：`validation_errors.txt`

### 2.3.4 输出说明
1. **转换结果文件**：
   - `processed_`前缀：仅转换未验证
   - `temp_`前缀：中间文件（转换+验证流程）

2. **验证结果**：
   - 成功：控制台显示绿色成功信息
   - 失败：红色错误信息+错误详情写入`validation_errors.txt`

3. **统计信息**：
   - 处理结束后显示：
     - 耗时统计（总时间/解析时间/转换时间）
     - 成功/失败文件计数

### 2.3.6 注意事项
1. 输入路径可以是单个文件或包含`.txt`的文件夹
2. Windows系统会自动配置UTF-8控制台输出
3. 启用`-edc`选项会严格检查月份天数完整性
4. 验证失败时会保留错误标记文件（`error_validation_*`）


# 3 graph_graph_generator 图表生成
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
## 3.3 timeline生成
生成横坐标为时间，纵坐标为项目名称的timeline图表
### 3.3.1 commond format 命令格式
```bash
python main.py timeline <date>
```
### 3.3.2 参数说明

`<date>` (必需): 您希望查询的目标日期，格式必须为 YYYYMMDD (例如: 20240528)
### 3.3.3 示例
下面的代码给出了生成20240623日的timeline图表:
```bash
python main.py timeline 20240623
```

## 3.4 柱状图生成
生成横坐标为项目名称，纵坐标为持续时间的柱状图
### 3.4.1 commond format 命令格式
```bash
python main.py barchart <date>
```
### 3.4.2 参数说明
`<date>` (必需): 您希望查询的目标日期，格式必须为 YYYYMMDD (例如: 20240623)

### 3.4.3 示例

下面的代码给出了生成20240623日的柱状图图表:
```bash
python main.py barchart 20240623
```

## 3.5  热力图生成
此命令为指定的项目和年份生成两种格式的 HTML 热力图：全年视图和月度视图。
### 3.5.1 commond format 命令格式
```bash
python main.py heatmap <year> [-p <project_name>]
```
### 3.5.2 参数说明
`<year> `(必需): 您希望查询的目标年份 (例如: 2025)。
`-p, --project <project_name> `(可选): 您希望分析的父项目名称。如果省略此参数，程序将默认使用 mystudy。

### 3.5.3 示例
1. 下面的代码生成2025年的热力图(默认项目为 mystudy):
```bash
python main.py heatmap 2025
```
2. 下面的代码生成2025年的热力图，项目为 meal:
```bash
python main.py heatmap 2025 -p meal
```




## 3.6  Sleep布尔状态生成
此命令用于根据数据库中的睡眠记录，为指定年份生成两种格式的 HTML 睡眠状态热力图：全年视图和月度视图。

会在当前目录同时生成两个文件：
1. 2024_sleep_heatmap_annual.html (全年视图)
2. 2024_sleep_heatmap_monthly.html (月度视图)。

### 3.6.1commond format 命令格式
```bash
python main.py sleep <year>
```
### 3.6.2 参数说明

`<year> `(必需): 您希望查询的目标年份 (例如: 2024)。

### 3.6.3 示例
1. 生成2024年的睡眠状态热力图：
```bash
python main.py sleep 2024
```




# ４ log_generator 日志生成
txt生成器
## 4.1 log_generator.py
用于生成测试数据
### 4.1.1 usage
log_generator.exe \<num_days> \<items_per_day>






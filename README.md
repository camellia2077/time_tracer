# 1 db 
主程序，用于解析文本内容，存入数据库，查询数据库
## 1.1 structure
```
db/
├── CMakeLists.txt
├── main.cpp
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
## 1.2 compile
cd 'c:Time_Master_cpp/db'

mkdir build

cd build

cmake .. -D CMAKE_BUILD_TYPE=Release

cmake --build .
# 2 log_tool 数据预处理
数据预处理与合法性检验
## 2.1 structure
```
process/
├── main.cpp                 # Main program entry point, handles argument parsing, orchestrates file processing and validation.
├── IntervalProcessor.h      # Header for IntervalProcessor class, declares the class and its structures for processing raw input files into a standardized format.
├── IntervalProcessor.cpp    # Implementation of IntervalProcessor, contains logic for parsing, mapping, and transforming event data.
├── FormatValidator.h        # Header for FormatValidator class, declares the class and its structures for validating the format of processed files.
├── FormatValidator.cpp      # Implementation of FormatValidator, contains logic for checking various format rules and identifying errors.
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
```
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
### 2.2.2interval_processor_config.json
此文件控制着 IntervalProcessor 如何转换和映射原始日志中的活动。

**文件路径**: configs/interval_processor_config.json

**结构**:

* `text_mappings`: 一个键值对集合，用于将原始日志中的中文或自定义缩写映射为标准化的项目路径。

**键**: 原始日志中的文本（例如 "单词" 或 "听力"）。

**值**: 希望转换成的标准项目路径（例如 "study_english_words"）。

* `duration_mappings`:: 一个对象，用于定义基于活动持续时间的动态映射规则。

**键**: 一个基础项目路径（必须已在 text_mappings 中定义过，例如 "insomnia"）。

**值**: 一个规则数组，每个规则对象包含：

**less_than_minutes**: 时间阈值（分钟）。

**value**: 当活动持续时间小于此阈值时，最终采用的项目路径。

使用方法:

要添加新的文本映射，只需在 text_mappings 对象中新增一行键值对。

要为某个活动（如 "饭"）添加基于时长的分类，先确保它在 text_mappings 中有基础映射（例如 "饭": "meal"）

然后在 duration_mappings 中新增一个以 "meal" 为键的规则数组。程序会按从小到大的顺序检查阈值。

示例:
```
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

### 2.2.3format_validator_config.json

此文件为 FormatValidator 定义了项目层级关系和分类，用于合法性检验。

文件路径: configs/format_validator_config.json

结构:

PARENT_CATEGORIES: 一个对象，定义了所有项目的父子层级关系。

键: 父类别的名称（例如 "study", "code", "recreation"）。

值: 一个字符串数组，包含了所有属于该父类别的子项目标准路径。

使用方法:

当您在 interval_processor_config.json 中定义了一个新的标准项目路径后，您应该将这个新路径添加到 PARENT_CATEGORIES 中对应父类别的数组里。

例如，如果您新增了一个活动 recreation_game_new, 您需要将其添加到 "recreation" 类别的数组中。

这确保了合法性检验器能够正确识别所有活动都属于一个已知的类别。

示例:
```
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

## 2.3 usage

### 2.3.1 输入文件目录
```<path>``` (必需): 源文件或源文件夹的路径。
### 2.3.2 转换格式
-p ```<path>``` or -P ```<path>```: 仅转换。读取源文件并生成一个新的格式化文件，但不进行内容合法性检验。输出文件名为 processed_<原始文件名>.txt。
### 2.3.3 检验合法性
-v ```<path>``` or -V ```<path>```: 仅检验。对源文件的格式和内容进行合法性检验，不生成新文件。
### 2.3.4 转换格式并验证合法性
-pv ```<path>``` or -PV ```<path>```: 转换并检验。首先根据源文件生成新的格式化文件，然后对这个新生成的文件进行合法性检验。
### 2.3.5 开启输入文本日期完整性检验
-edc or ---enable-day-chec
-pv ```<path>``` --enable-day-check :转换文本并，检验合法性且开启日期完整检验
-v  ```<path>```  -edc :检验合法性并且开启日期检验


如果提供的是文件路径，则只处理该文件。

如果提供的是文件夹路径，则处理该文件夹下所有的 .txt 文件。

# 3 graph_graph_generator 数据可视化
读取数据库并且生成图表
## 3.1 structure
graph_generator/
│
├── db_access.py                # 模块：统一数据库访问层 (DAL)
├── heatmap_generator.py        # 生成项目工时热力图 (年/月)
├── timeline_generator.py       # 生成单日活动时间轴图
├── bool_generator.py           # 生成布尔值（如睡眠）热力图
│
├── heatmap_config.json         # 配置文件：用于 heatmap_generator.py
├── timeline_colors_configs.json # 配置文件：用于 timeline_generator.py
└── time_data.db                # 数据库：存储所有时间追踪数据
## 3.2 heatmap_generator.py
用于生成年份的热力图
### 3.2.1 生成热力图(默认study)
python generate_heatmap.py 2024
### 3.2.1 指定项目生成热力图
python generate_heatmap.py 2024 -p code

或

python generate_heatmap.py 2024 --project code

## 3.3 timeline_generator.py
用于生成某日的时间线
### 3.3.1 usage
py timeline_generator.py 20241201
## 3.4 bool_generator.py
用于生成bool类型标头的年份热力图
### 3.4.1 usage
py timeline_generator.py 2024


# ４ log_generator 日志生成
txt生成器
## 4.1 log_generator.py
用于生成测试数据
### 4.1.1 usage
log_generator.exe \<num_days> \<items_per_day>






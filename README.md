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
## 2.1 usage
### 2.1.1 转换格式
-p or -P: 仅转换。读取源文件并生成一个新的格式化文件，但不进行内容合法性检验。输出文件名为 processed_<原始文件名>.txt。
### 2.1.2 检验合法性
-v or -V: 仅检验。对源文件的格式和内容进行合法性检验，不生成新文件。
### 2.1.3 转换格式并验证合法性
-pv or -PV: 转换并检验。首先根据源文件生成新的格式化文件，然后对这个新生成的文件进行合法性检验。
### 2.1.4 文件目录
```<path>``` (必需): 源文件或源文件夹的路径。

如果提供的是文件路径，则只处理该文件。

如果提供的是文件夹路径，则处理该文件夹下所有的 .txt 文件。

# 3 graph_graph_generator 数据可视化
读取数据库并且生成图表
## 3.1 heatmap_generator.py
用于生成年份的热力图
### 3.1.1 生成热力图(默认study)
python generate_heatmap.py 2024
### 3.1.1 指定项目生成热力图
python generate_heatmap.py 2024 -p code

或

python generate_heatmap.py 2024 --project code

## 3.2 timeline_generator.py
用于生成某日的时间线
### 3.2.1 usage
py timeline_generator.py 20241201
## 3.3 bool_generator.py
用于生成bool类型标头的年份热力图
### 3.3.1 usage
py timeline_generator.py 2024


# ４ log_generator 日志生成
txt生成器
## 4.1 log_generator.py
用于生成测试数据
### 4.1.1 usage
log_generator.exe \<num_days> \<items_per_day>






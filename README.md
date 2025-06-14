# db
主程序，用于解析文本内容与查询
## structure
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
├── parsing/             # Handles the parsing and structuring of data from input files.
│   ├── data_parser.cpp
│   └── data_parser.h
├── processing/
│   ├── processing.cpp
│   └── processing.h
├── queries/             # Contains all logic for querying the database and generating user-facing reports.
│
├── query_handler.cpp    # Implements the QueryHandler class, which acts as a simple interface (Facade) to route query requests.
│
├── query_handler.h      # Declares the QueryHandler class, the main entry point for all query operations.
│
├── query_day.cpp        # Implements the logic to generate a detailed report for a single day.
│
├── query_day.h          # Declares the class responsible for generating a daily report.
│
├── query_month.cpp      # Implements the logic to generate a summary report for a specific month.
│
├── query_month.h        # Declares the class that generates a monthly report.
│
├── query_period.cpp     # Implements the logic for generating reports over a custom period (e.g., last 7 days).
│
├── query_period.h       # Declares the class for generating reports over a specific time period.
│
├── query_utils.cpp      # Implements shared helper functions for querying, such as building project trees and formatting output.
│
└── query_utils.h        # Declares various utility functions used by the different query generators.
└── resources/
    ├── app_icon.rc
    └── output_icon.ico
```
## compile
cd 'c:Time_Master_cpp/db'

mkdir build

cd build

cmake .. -D CMAKE_BUILD_TYPE=Release

cmake --build .

# 辅助工具
## log_generator.cpp
用于生成截止时间的测试数据，通过命令行输入生成时间以及每天的项目数量
### usage
program.exe num_days items_per_day

log_generator.exe 3650 30

## processor
用于处理数据和验证合法性
### usage
-v yourfielpath


## time_tracker
### usage
time_tracker.exe <filepath.txt>// 存入数据

time_tracker.exe query 20250501//查询 20250501这天的数据

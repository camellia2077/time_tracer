# 1 db
主程序，用于解析文本内容与查询
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
# 2 process
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

# 3 graph
读取数据库并且生成图表
## 3.1 heatmap_generator.py
用于生成年份的热力图
### 3.1.1 usage
python generate_heatmap.py 2024
## 3.2 timeline_generator.py
用于生成某日的时间线
### 3.2.1 usage
python timeline_generator.py 20241201






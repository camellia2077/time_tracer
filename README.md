# log_generator.cpp
用于生成截止时间的测试数据，通过命令行输入生成时间以及每天的项目数量

program.exe <num_days> <items_per_day>

log_generator.exe 3650 30

# processor
用于处理数据和验证合法性

-v "<yourfielpath>"


# time_tracker
time_tracker.exe <filepath.txt>// 存入数据

time_tracker.exe' query 20250501//查询 20250501这天的数据

# db
## structure
db/
├── CMakeLists.txt
├── main.cpp
├── common/
│   ├── common_utils.cpp
│   └── common_utils.h
├── database/
│   ├── database_importer.cpp
│   └── database_importer.h
├── parsing/
│   ├── data_parser.cpp
│   └── data_parser.h
├── queries/
│   ├── query_day.cpp
│   ├── query_day.h
│   ├── query_handler.cpp
│   ├── query_handler.h
│   ├── query_month.cpp
│   ├── query_month.h
│   ├── query_period.cpp
│   ├── query_period.h
│   ├── query_utils.cpp
│   └── query_utils.h
└── resources/
    ├── app_icon.rc
    └── output_icon.ico
## compile
cd 'c:Time_Master_cpp/db'

mkdir build

cd build

cmake .. -D CMAKE_BUILD_TYPE=Release

cmake --build .


# 性能对比 log_generator.cpp
days_36500_items_30

cpp 0.394981 seconds

py 2.571006 seconds


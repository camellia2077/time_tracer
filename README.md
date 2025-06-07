# log_generator.cpp
用于生成截止时间的测试数据，通过命令行输入生成时间以及每天的项目数量

program.exe <num_days> <items_per_day>

log_generator.exe 3650 30

# processor
用于处理数据和验证合法性

-v <fielpath>

# time_tracker
time_tracker.exe <filepath.txt>// 存入数据

time_tracker.exe' query 20250501//查询 20250501这天的数据

# db
g++ -std=c++17 -O3 -o time_tracker main.cpp -lsqlite3


# 性能对比 log_generator.cpp
days_36500_items_30

cpp 0.394981 seconds

py 2.571006 seconds


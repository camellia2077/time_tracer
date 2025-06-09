# 设置 CMake 所需的最低版本
cmake_minimum_required(VERSION 3.10)

# 定义项目名称和所用语言 (CXX 代表 C++)
project(TimeTrackerApp CXX)

# 设置 C++ 标准为 C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --- 为 Release 构建类型设置优化编译选项 ---
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -march=native -flto")

# --- 为 Release 构建类型设置优化链接选项 ---
set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} -flto -s")


# --- 查找外部依赖 ---
find_package(SQLite3 REQUIRED)

# --- 【修改】收集所有源文件 ---
# 使用 file(GLOB ...) 从每个子目录中自动查找源文件
file(GLOB PARSING_SOURCES "parsing/*.cpp")
file(GLOB DATABASE_SOURCES "database/*.cpp")
file(GLOB QUERIES_SOURCES "queries/*.cpp")

# --- 定义最终的可执行文件 ---
add_executable(
    time_tracker_app
    # 列出所有需要编译的源文件
    main.cpp
    ${PARSING_SOURCES}   # 添加 parsing 目录下的所有 .cpp 文件
    ${DATABASE_SOURCES}  # 添加 database 目录下的所有 .cpp 文件
    ${QUERIES_SOURCES}   # 添加 queries 目录下的所有 .cpp 文件
)

# --- 【修改】为目标添加头文件搜索路径 ---
# 使用 target_include_directories 为我们的程序添加所有需要的头文件目录
# 这是比 include_directories() 更现代、更推荐的做法
target_include_directories(time_tracker_app PRIVATE
    "common"
    "database"
    "parsing"
    "queries"
)

# --- 链接库 ---
target_link_libraries(time_tracker_app PRIVATE SQLite::SQLite3)

# --- 推荐的通用编译选项 ---
target_compile_options(time_tracker_app PRIVATE -Wall)

# 打印一条消息，确认配置完成
message(STATUS "CMake configuration finished. Target 'time_tracker_app' is ready to be built.")
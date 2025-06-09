# 设置 CMake 所需的最低版本
cmake_minimum_required(VERSION 3.10)

# 定义项目名称和所用语言 (CXX 代表 C++)
project(TimeTrackerApp CXX)

# 设置 C++ 标准为 C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --- 【修改】为 Release 构建类型设置优化编译选项 ---
# 将 -O3, -march=native, -flto 添加到 Release 模式的编译器标志中
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -march=native -flto")

# --- 【修改】为 Release 构建类型设置优化链接选项 ---
# 启用链接时优化，并告诉链接器剥离符号信息
set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} -flto -s")


# --- 查找外部依赖 ---
find_package(SQLite3 REQUIRED)

# --- 添加头文件搜索路径 ---
include_directories(. queries)

# --- 收集所有源文件 ---
file(GLOB QUERIES_SOURCES "queries/*.cpp")

# --- 定义最终的可执行文件 ---
add_executable(
    time_tracker_app
    main.cpp
    data_parser.cpp
    database_importer.cpp
    ${QUERIES_SOURCES}
)

# --- 链接库 ---
target_link_libraries(time_tracker_app PRIVATE SQLite::SQLite3)

# --- 推荐的通用编译选项 ---
# -Wall 会应用到所有构建类型（Debug 和 Release）
target_compile_options(time_tracker_app PRIVATE -Wall)

# 打印一条消息，确认配置完成
message(STATUS "CMake configuration finished. Target 'time_tracker_app' is ready to be built.")
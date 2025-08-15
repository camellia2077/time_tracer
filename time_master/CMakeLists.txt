# --- START OF FILE CMakeLists.txt ---

cmake_minimum_required(VERSION 3.16)
project(TimeTrackerApp VERSION "0.2.0" LANGUAGES CXX)
# --- Find External Dependencies ---
find_package(SQLite3 REQUIRED)
find_package(nlohmann_json REQUIRED)

# --- 1. 项目范围的设置 ---
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 设置编译器启动器（如ccache）
find_program(CCACHE_EXECUTABLE ccache)
if(CCACHE_EXECUTABLE)
    message(STATUS "ccache found, enabling compiler launcher.")
    set(CMAKE_CXX_COMPILER_LAUNCHER "${CCACHE_EXECUTABLE}")
endif()

# 设置构建类型的标志
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Os -march=native")
set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} -flto -s")

# --- 2. 包含模块 ---
# 将模块路径添加到CMake的搜索路径中
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

include(SourceFileCollection) # 包含并执行源文件收集脚本
include(TargetSetup)          # 包含并定义 setup_project_target 函数

# --- 3. 定义可执行文件目标 ---
add_executable(time_tracker_app main.cpp ${COMMON_SOURCES} ${DB_INSERTER_SOURCES} ${QUERIES_SOURCES} ${TIME_MASTER_APP_SOURCES} ${REPROCESSING_SOURCES} ${FILE_HANDLER_SOURCES} ${ACTION_HANDLER_SOURCES})
add_executable(time_tracker_cli main_cli.cpp ${COMMON_SOURCES} ${DB_INSERTER_SOURCES} ${QUERIES_SOURCES} ${REPROCESSING_SOURCES} ${FILE_HANDLER_SOURCES} ${ACTION_HANDLER_SOURCES} ${TIME_MASTER_CLI_SOURCES})

# 创建一个包含所有可执行文件目标的列表
set(ALL_TARGETS time_tracker_app time_tracker_cli)

# --- 4. 为每个目标应用通用配置 ---
foreach(TARGET_NAME ${ALL_TARGETS})
    setup_project_target(${TARGET_NAME})
endforeach()

# --- 5. 包含特定功能的模块 ---
include(Win32PostBuildCopy) # 包含并执行DLL复制逻辑
include(Packaging)          # 包含并执行打包逻辑

# --- 6. 结束语 ---
message(STATUS "CMake configuration finished. Targets are ready to be built.")
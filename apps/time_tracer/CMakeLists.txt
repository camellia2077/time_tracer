# --- START OF FILE CMakeLists.txt ---
cmake_minimum_required(VERSION 3.25)

project(TimeTrackerApp VERSION "0.5.5" LANGUAGES CXX)

# [新增] 强制 CMake 的命令行输出使用 UTF-8 编码
set(CMAKE_CLI_FORCE_UTF8 ON)

cmake_policy(SET CMP0115 NEW)

# --- Find External Dependencies ---
find_package(SQLite3 REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(tomlplusplus REQUIRED)

# --- 1. 项目范围的设置 ---
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(ENABLE_APP_ICON "Enable application icon for Windows executables" OFF)

set(PLUGIN_OUTPUT_DIR "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/plugins")

# 设置编译器启动器（如ccache）
find_program(CCACHE_EXECUTABLE ccache)
if(CCACHE_EXECUTABLE)
    message(STATUS "ccache found, enabling compiler launcher.")
    set(CMAKE_CXX_COMPILER_LAUNCHER "${CCACHE_EXECUTABLE}")
endif()

# --- 2. 包含模块 ---
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# 1. 基础工具
include(TargetSetup)
# include(SourceFileCollection) # REMOVED: Replaced by modular CMakeLists.txt
# include(AddReportsLibrary)      # REMOVED: Moved to src/reports/CMakeLists.txt
# include(AddReportsDataLibrary)  # REMOVED: Moved to src/reports/CMakeLists.txt

# 4. 编译标志
include(BuildTypeFlags)

# --- 3. 添加源码子目录 ---
add_subdirectory(src)

# 添加子目录以构建所有日报格式化器的 DLL
add_subdirectory(src/reports/daily/formatters/markdown)
add_subdirectory(src/reports/daily/formatters/typst)
add_subdirectory(src/reports/daily/formatters/latex)
add_subdirectory(src/reports/range/formatters/markdown)
add_subdirectory(src/reports/range/formatters/typst)
add_subdirectory(src/reports/range/formatters/latex)

set(ALL_TARGETS time_tracker_cli)

# 在每次构建后，复制配置文件
# 在每次构建后，复制配置文件 (Moved to src/CMakeLists.txt)
# foreach(TARGET_NAME ${ALL_TARGETS})
#    ...
# endforeach()

# --- 代码格式化工具 ---
include(AddFormatTarget)
include(AddTidyFixTarget)
# --- 5. 包含特定功能的模块 ---
# include(Win32PostBuildCopy) # Moved to src/CMakeLists.txt 
include(Packaging)        

message(STATUS "CMake configuration finished. Targets are ready to be built.")

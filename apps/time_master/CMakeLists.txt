# --- START OF FILE CMakeLists.txt ---

cmake_minimum_required(VERSION 3.16)
project(TimeTrackerApp VERSION "0.2.0" LANGUAGES CXX)

# 所有源文件都必须明确写出它们的扩展名
cmake_policy(SET CMP0115 NEW)

# --- Find External Dependencies ---
find_package(SQLite3 REQUIRED)
find_package(nlohmann_json REQUIRED)

# --- 1. 项目范围的设置 ---
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ==================== [核心修改] ====================
# 定义插件输出目录变量，位于可执行文件目录下的 "plugins" 子文件夹
set(PLUGIN_OUTPUT_DIR "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/plugins")
# ====================================================

# 设置编译器启动器（如ccache）
find_program(CCACHE_EXECUTABLE ccache)
if(CCACHE_EXECUTABLE)
    message(STATUS "ccache found, enabling compiler launcher.")
    set(CMAKE_CXX_COMPILER_LAUNCHER "${CCACHE_EXECUTABLE}")
endif()

# --- 2. 包含模块 ---
# 将模块路径添加到CMake的搜索路径中
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

include(SourceFileCollection) # 包含并执行源文件收集脚本
include(TargetSetup)          # 包含并定义 setup_project_target 函数
include(BuildTypeFlags)       # 包含并执行构建类型标志的设置脚本

# --- 3. 定义可执行文件目标 ---
add_executable(time_tracker_cli src/main_cli.cpp
    ${COMMON_SOURCES}
    ${CONFIG_VALIDATOR}
    ${DB_INSERTER_SOURCES}
    ${REPORTS_SOURCES}
    ${REPROCESSING_SOURCES}
    ${FILE_HANDLER_SOURCES}
    ${ACTION_HANDLER_SOURCES}
    ${TIME_MASTER_CLI_SOURCES}
    )

# 添加 src/reports/daily/formatters/md 子目录
# 这会执行该目录下的 CMakeLists.txt，从而构建 DayMdFormatter.dll
add_subdirectory(src/reports/daily/formatters/md)

# 创建一个包含所有可执行文件目标的列表
set(ALL_TARGETS time_tracker_cli)

# --- 4. 为每个目标应用通用配置 ---
foreach(TARGET_NAME ${ALL_TARGETS})
  setup_project_target(${TARGET_NAME})
endforeach()

# 在每次构建后，无论是否为安装包，都将配置文件复制到输出目录
# 这样可以直接从 build/bin 目录运行程序
foreach(TARGET_NAME ${ALL_TARGETS})
    add_custom_command(
        TARGET ${TARGET_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/src/config"
        "$<TARGET_FILE_DIR:${TARGET_NAME}>/config"
        COMMENT "Copying config folder for ${TARGET_NAME}"
    )
endforeach()

# --- 5. 包含特定功能的模块 ---
include(Win32PostBuildCopy) # 包含并执行DLL复制逻辑
include(Packaging)          # 包含并执行打包逻辑

# --- 6. 结束语 ---
message(STATUS "CMake configuration finished. Targets are ready to be built.")
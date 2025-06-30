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
find_package(nlohmann_json REQUIRED)

# --- 收集所有源文件 ---
# 使用 file(GLOB ...) 从每个子目录中自动查找源文件
file(GLOB COMMON_SOURCES "common/*.cpp")
file(GLOB PARSING_SOURCES "parsing/*.cpp")
file(GLOB DATABASE_SOURCES "database/*.cpp")
file(GLOB QUERIES_SOURCES "queries/*.cpp")
file(GLOB MENU_SOURCES "Menu/*.cpp")
file(GLOB PROCESSING_SOURCES "processing/*.cpp")

# --- 定义两个独立的可执行文件目标 ---

# 目标 1: 交互式菜单应用 (使用 main.cpp)
add_executable(
    time_tracker_app          # 可执行文件名
    main.cpp                  # 入口文件
    ${COMMON_SOURCES}
    ${PARSING_SOURCES}
    ${DATABASE_SOURCES}
    ${QUERIES_SOURCES}
    ${MENU_SOURCES}
    ${PROCESSING_SOURCES}
)

# 目标 2: 命令行工具 (使用 main_command.cpp)
add_executable(
    time_tracker_command      # 可执行文件名
    main_command.cpp          # 入口文件
    ${COMMON_SOURCES}
    ${PARSING_SOURCES} 
    ${DATABASE_SOURCES} 
    ${QUERIES_SOURCES}
    ${MENU_SOURCES}
    ${PROCESSING_SOURCES} 
)


# --- 为 Windows 平台添加图标资源 ---
if(WIN32)
    # 找到 windres.exe 程序
    find_program(WINDRES_EXECUTABLE windres)
    if(NOT WINDRES_EXECUTABLE)
        message(FATAL_ERROR "Failed to find windres.exe. Please ensure it is in your PATH.") 
    endif()

    # 将 .rc 文件添加到源文件列表
    list(APPEND RESOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/resources/app_icon.rc")

    # 将资源文件添加到两个目标中
    target_sources(time_tracker_app PRIVATE ${RESOURCE_FILES})
    target_sources(time_tracker_command PRIVATE ${RESOURCE_FILES})

    # 打印消息
    message(STATUS "Windows icon resource added to both targets.")
endif()

# --- 为每个目标分别添加头文件搜索路径 ---
target_include_directories(time_tracker_app PRIVATE
    "common" "database" "parsing" "queries" "resources" "Menu" "processing"
)
target_include_directories(time_tracker_command PRIVATE
    "common" "database" "parsing" "queries" "resources" "Menu" "processing"
)

# --- 为每个目标分别链接库 ---
target_link_libraries(time_tracker_app PRIVATE 
    SQLite::SQLite3 
    nlohmann_json::nlohmann_json)
target_link_libraries(time_tracker_command PRIVATE 
    SQLite::SQLite3 
    nlohmann_json::nlohmann_json)

# --- 为每个目标分别设置通用编译选项 ---
target_compile_options(time_tracker_app PRIVATE -Wall)
target_compile_options(time_tracker_command PRIVATE -Wall)

# 打印一条消息，确认配置完成
message(STATUS "CMake configuration finished. Targets 'time_tracker_app' and 'time_tracker_command' are ready to be built.")
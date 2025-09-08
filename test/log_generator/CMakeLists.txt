# --- 1. Project Configuration ---
cmake_minimum_required(VERSION 3.16)
project(log_generator_project CXX)

# --- 2. C++ Standard ---
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# --- 3. Dependency Handling (nlohmann/json) ---
find_package(nlohmann_json 3.11.3 REQUIRED)

# --- 4. Executable Definition (MODIFIED) ---
# [核心修改] 更新源文件列表以反映新的目录结构。
# 我们现在使用相对于 CMakeLists.txt 的完整路径。
set(SOURCES
    src/main.cpp
    src/cli/CommandLineParser.cpp
    src/cli/HelpPrinter.cpp       # <-- [新增] 将新文件添加到源列表
    src/config/Config.cpp
    src/file_io/FileManager.cpp
    src/generator/_internal/RemarkGenerator.cpp # <-- [新增]
    src/generator/_internal/EventGenerator.cpp  # <-- [新增]
    src/generator/facade/LogGenerator.cpp
    src/utils/PerformanceReporter.cpp
    src/utils/Utils.cpp
)
add_executable(log_generator ${SOURCES})

# [核心修改] 添加 src 目录作为头文件包含路径。
# 这使得我们可以在代码中使用 #include "cli/CommandLineParser.h" 这样的路径，
# 而不是复杂的相对路径。
target_include_directories(log_generator PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

# --- 5. Linking Dependencies ---
target_link_libraries(log_generator PRIVATE nlohmann_json::nlohmann_json)

# --- 6. Build-time Configuration File Copying (NEW) ---
# [核心修改] 添加一个自定义命令，在构建时自动将配置文件
# 从项目根目录下的 /config 文件夹复制到编译输出目录（例如 build/）。
set(CONFIG_SOURCE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/config/activities_config.json)
# CMAKE_BINARY_DIR 是CMake中指向build目录的变量
set(CONFIG_DESTINATION_PATH ${CMAKE_BINARY_DIR}/config/activities_config.json)

# 确保目标目录存在
add_custom_command(
    TARGET log_generator PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/config
    COMMENT "Ensuring config directory exists in build directory"
)

# 添加复制文件的命令
add_custom_command(
    OUTPUT ${CONFIG_DESTINATION_PATH}
    COMMAND ${CMAKE_COMMAND} -E copy
        ${CONFIG_SOURCE_PATH}
        ${CONFIG_DESTINATION_PATH}
    DEPENDS ${CONFIG_SOURCE_PATH}
    COMMENT "Copying configuration file to build directory"
)

# 创建一个目标来代表这个复制操作
add_custom_target(
    copy_config ALL
    DEPENDS ${CONFIG_DESTINATION_PATH}
)

# 确保在编译可执行文件之前，文件复制操作已完成
add_dependencies(log_generator copy_config)


# --- 7. Optimization and Version-Specific Compilation Options ---
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
  target_compile_options(log_generator PRIVATE
    $<$<CONFIG:Release>:-O3>
    $<$<CONFIG:Release>:-march=native>
  )
endif()

set_property(TARGET log_generator PROPERTY INTERPROCEDURAL_OPTIMIZATION_RELEASE TRUE)
set_property(TARGET log_generator PROPERTY LINK_FLAGS_RELEASE "-s")


# --- 8. Status Messages ---
message(STATUS "Project configured. To compile, run:")
message(STATUS "  cmake --build . --config Release")
message(STATUS "The executable will be found in the build directory.")
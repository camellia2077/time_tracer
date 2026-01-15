# --- 1. Project Configuration ---
cmake_minimum_required(VERSION 3.16)
project(log_generator_project CXX)

# [新增] 尝试查找并启用 ccache (编译器缓存)
# 必须在 add_executable 之前设置
find_program(CCACHE_EXECUTABLE ccache)
if(CCACHE_EXECUTABLE)
    message(STATUS "ccache found: ${CCACHE_EXECUTABLE}")
    set(CMAKE_CXX_COMPILER_LAUNCHER "${CCACHE_EXECUTABLE}")
else()
    message(STATUS "ccache not found. Proceeding without compiler cache.")
endif()

# 将 cmake 目录添加到模块搜索路径，方便 include
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# --- 2. Load Modules (按顺序加载) ---

# A. 设置标准
include(StandardSettings)

# B. 查找依赖
include(Dependencies)

# C. 定义源文件列表 (加载 Sources.cmake)
include(Sources)

# --- 3. Executable Definition ---
# 使用 Sources.cmake 中定义的 ${SOURCES} 变量
add_executable(log_generator ${SOURCES})

# 添加头文件包含路径
target_include_directories(log_generator PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

# --- 4. Link Libraries ---
target_link_libraries(log_generator PRIVATE tomlplusplus::tomlplusplus)

# --- 5. Apply Remaining Modules ---

# D. 应用编译优化 (需要 target 已经定义)
include(CompilerOptions)

# E. 执行文件操作 (需要 target 已经定义)
include(FileOperations)

# --- 6. Status Messages ---
message(STATUS "Project configured.")
message(STATUS "To compile, run: cmake --build . --config Release")
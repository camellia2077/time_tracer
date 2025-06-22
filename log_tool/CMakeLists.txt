cmake_minimum_required(VERSION 3.10) # 最低 CMake 版本要求

# 项目名称
project(TimeMasterCpp VERSION 1.0)

# 设置 C++ 标准为 C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# 设置构建类型，可以根据需要调整
# Debug: 包含调试信息，无优化
# Release: 优化代码，不包含调试信息 (对应 -O3 -s -flto)
# RelWithDebInfo: 优化代码，包含调试信息
# MinSizeRel: 最小化代码大小
if (NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    # 如果没有指定构建类型，默认设置为 Release
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build." FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "RelWithDebInfo" "MinSizeRel")
endif()

# 根据 CMAKE_BUILD_TYPE 设置编译器标志
if (CMAKE_BUILD_TYPE STREQUAL "Release")
    message(STATUS "Building in Release mode with -O3 -march=native -flto -s")
    # 添加你在 g++ 命令中使用的优化和链接时优化标志
    add_compile_options(-O3 -march=native)
    # flto (Link Time Optimization) 应该作为链接器标志添加
    # -s (strip) 也是链接器标志
    set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} -flto -s")
elseif (CMAKE_BUILD_TYPE STREQUAL "Debug")
    message(STATUS "Building in Debug mode with -g")
    add_compile_options(-g)
endif()

# 添加头文件搜索路径
# 这里我们假设 nlohmann/json.hpp 已经在 MSYS2 的标准 MinGW 路径下，
# 如果pacman正确安装了，CMake的find_package通常能找到
# 如果还报错 "fatal error: nlohmann/json.hpp: No such file or directory"
# 你可能需要手动添加，例如：
# include_directories(C:/msys64/mingw64/include) # 替换成你实际的安装路径
# 或者更好的方式是使用 find_package

# 查找 nlohmann/json 库
# CMake 3.11 及以上版本支持 FindnlohmannJson.cmake 模块
# 如果你的 CMake 版本低于 3.11，你可能需要手动下载并放置 FindnlohmannJson.cmake
# 或者直接使用 include_directories 添加路径
find_package(nlohmann_json CONFIG REQUIRED)

# 告诉 CMake 你的源文件
set(SOURCES
    main.cpp
    IntervalProcessor.cpp
    FormatValidator.cpp
    ErrorReporter.cpp
)

# 告诉 CMake 你的头文件 (不需要编译，但有助于IDE理解项目结构)
set(HEADERS
    IntervalProcessor.h
    FormatValidator.h
    SharedUtils.h
    ErrorReporter.h
)

# 创建可执行文件
add_executable(log_tool ${SOURCES} ${HEADERS})

# 链接 nlohmann/json 库
# 这里使用 nlohmann_json::nlohmann_json，这是通过 find_package 找到的现代 CMake 目标
target_link_libraries(log_tool PRIVATE nlohmann_json::nlohmann_json)

# 如果你确定需要链接到 Windows API (SetConsoleOutputCP, SetConsoleCP)，
# 并且它不是默认链接的，可以添加以下行：
# target_link_libraries(log_tool PRIVATE kernel32) # 对于 SetConsoleOutputCP 和 SetConsoleCP

# 也可以将头文件添加到 target 的 public 或 private 包含目录中，
# 这样 IDE 能够更好地识别
target_include_directories(log_tool PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR} # 包含当前目录的头文件，如 SharedUtils.h
)

# 模块：构建类型标志设置 (BuildTypeFlags.cmake)
# ----------------------------------------------------
# 此模块专门负责为不同的构建类型（如 Release）配置编译和链接标志。

message(STATUS "Configuring build type flags...")

# 模块：构建类型标志设置 (BuildTypeFlags.cmake)
# ----------------------------------------------------
# 此模块专门负责为不同的构建类型（如 Release）配置编译和链接标志。

message(STATUS "Configuring build type flags...")

# --- 1. 设置通用的 Release 模式编译标志 ---
# -Os:         优化代码大小
# -march=native: 针对当前机器的CPU架构进行优化，以获得最佳性能
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Os -march=native")

# --- 2. 设置通用的 Release 模式链接器标志 ---
# -s:          剔除可执行文件中的符号信息，减小最终文件大小
set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} -s")

# --- 3. 根据编译器类型，添加特定的链接时优化 (LTO) 标志 ---
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    message(STATUS "GCC detected, enabling Link-Time Optimization (LTO).")
    # 为 GCC 添加 LTO 标志
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -flto")
    set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} -flto")

elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    message(STATUS "Clang detected, enabling Link-Time Optimization (LTO) with lld linker.")
    # 为 Clang 添加 LTO 标志，并指定使用 lld 链接器
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -flto=thin")
    set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} -fuse-ld=lld -flto=thin")
endif()

message(STATUS "Release build flags configured.")
# 模块：构建类型标志设置 (BuildTypeFlags.cmake)
# ----------------------------------------------------
# 此模块专门负责为不同的构建类型（如 Release）配置编译和链接标志。

message(STATUS "Configuring build type flags...")

# 为 "Release" 模式设置 C++ 编译器标志
# -Os:         优化代码大小
# -march=native: 针对当前机器的CPU架构进行优化，以获得最佳性能
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Os -march=native")

# 为 "Release" 模式设置可执行文件的链接器标志
# -flto:       启用链接时优化 (Link-Time Optimization)，进行全局优化
# -s:          剔除可执行文件中的符号信息，减小最终文件大小
set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} -flto -s")

message(STATUS "Release build flags configured with optimizations (-Os, -march=native, -flto).")
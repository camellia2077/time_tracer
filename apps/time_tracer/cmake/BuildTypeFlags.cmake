# 模块：构建类型标志设置 (BuildTypeFlags.cmake)
# ----------------------------------------------------
# 此模块专门负责为不同的构建类型（如 Release）配置编译和链接标志。

message(STATUS "Configuring build type flags...")

# [新增] 提供一个选项来彻底禁用优化，以极大地提高编译速度
# 这主要用于开发阶段，不考虑运行效率和 exe 体积
option(DISABLE_OPTIMIZATION "Disable all optimizations for fastest compilation speed" OFF)

if(DISABLE_OPTIMIZATION)
    message(STATUS "!!! Optimization is DISABLED for faster compilation speed !!!")
    # -O0: 禁用所有优化
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O0")
    # 不剔除符号信息，以便进一步缩短构建时间（虽然对速度影响较小，但符合“不优化”原则）
    set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE}")
    set(CMAKE_SHARED_LINKER_FLAGS_RELEASE "${CMAKE_SHARED_LINKER_FLAGS_RELEASE}")
else()
    # --- 1. 设置通用的 Release 模式编译标志 ---
    # -Os:         优化代码大小（兼顾体积与编译/运行稳定性）
    # -march=native: 针对当前机器的CPU架构进行优化，以获得最佳性能
    # -ffunction-sections/-fdata-sections: 将函数/数据分段，配合链接期 gc-sections 做体积裁剪
    set(CMAKE_CXX_FLAGS_RELEASE
        "${CMAKE_CXX_FLAGS_RELEASE} -Os -march=native -ffunction-sections -fdata-sections")

    # --- 2. 设置通用的 Release 模式链接器标志 ---
    # -s:          剔除可执行文件中的符号信息，减小最终文件大小
    set(CMAKE_EXE_LINKER_FLAGS_RELEASE
        "${CMAKE_EXE_LINKER_FLAGS_RELEASE} -s")
    # 这个是剔除libreports_shared.dll中的符号信息
    set(CMAKE_SHARED_LINKER_FLAGS_RELEASE "${CMAKE_SHARED_LINKER_FLAGS_RELEASE} -s")
endif()

# ==================== [核心修改] ====================
# 添加一个选项来控制LTO，可以由Python脚本通过 -DENABLE_LTO=ON/OFF 来设置
# 默认设为ON，因为对于Release构建来说，这是一个好的实践
option(ENABLE_LTO "Enable Link-Time Optimization for Release builds" ON)
# ====================================================

# --- 3. 根据编译器类型，添加特定的链接时优化 (LTO) 标志 ---
# ==================== [核心修改] ====================
if(ENABLE_LTO AND NOT DISABLE_OPTIMIZATION)
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
else()
    if(DISABLE_OPTIMIZATION)
        message(STATUS "Link-Time Optimization (LTO) is forced OFF because optimizations are disabled.")
    else()
        message(STATUS "Link-Time Optimization (LTO) is disabled by user configuration.")
    endif()
endif()
# ====================================================

if(NOT DISABLE_OPTIMIZATION)
    if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        # 使用 lld，并启用安全 ICF 以折叠等价代码/只读数据，减小主程序体积。
        set(CMAKE_EXE_LINKER_FLAGS_RELEASE
            "${CMAKE_EXE_LINKER_FLAGS_RELEASE} -fuse-ld=lld -Wl,--icf=safe")
        message(STATUS "Executable ICF enabled: -Wl,--icf=safe (lld)")
    endif()

    # 为主程序开启与 DLL 同类的 section-gc。对 Clang+LTO 的 MinGW 组合做兼容兜底。
    if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR NOT ENABLE_LTO)
        set(CMAKE_EXE_LINKER_FLAGS_RELEASE
            "${CMAKE_EXE_LINKER_FLAGS_RELEASE} -Wl,--gc-sections")
        message(STATUS "Executable section-gc enabled: -Wl,--gc-sections")
    else()
        message(STATUS "Executable section-gc skipped for Clang+LTO (MinGW compatibility).")
    endif()
endif()

message(STATUS "Release build flags configured.")

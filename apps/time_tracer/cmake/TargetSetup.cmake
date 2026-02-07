# 模块2：目标属性配置 (TargetSetup.cmake)
# 定义一个函数来避免为每个目标重复设置属性

# ==================== [配置项] ====================
# 定义一个变量来控制警告级别，允许从命令行 -DWARNING_LEVEL=X 覆盖
# 0=无, 1=-Wall, 2=-Wall -Wextra -Wpedantic, 3=级别2 + -Werror
set(WARNING_LEVEL 2 CACHE STRING "Set compiler warning level (0-3)")
# ====================================================


# 定义开关，默认开启（Release模式下可能希望关闭以加快速度，视情况而定）
option(ENABLE_CLANG_TIDY "Enable static analysis with clang-tidy" ON)
option(ENABLE_PCH "Enable Precompiled Headers" ON)

# 查找 clang-tidy 程序 (只查找一次)
if(ENABLE_CLANG_TIDY)
    find_program(CLANG_TIDY_EXE NAMES "clang-tidy")
    if(NOT CLANG_TIDY_EXE)
        message(WARNING "clang-tidy not found. Static analysis will be disabled.")
    else()
        message(STATUS "Found clang-tidy: ${CLANG_TIDY_EXE}")
    endif()
endif()

function(setup_project_target TARGET_NAME)
    # 添加头文件搜索路径
    # 使用 PROJECT_SOURCE_DIR 确保路径始终从项目根目录开始。
    # 所有内部引用均应基于 src/ 根路径进行绝对搜索（如 #include "domain/logic/..."）。
    target_include_directories(${TARGET_NAME} PRIVATE "${PROJECT_SOURCE_DIR}/src")

    # 链接核心依赖库
    target_link_libraries(${TARGET_NAME} PRIVATE
        SQLite::SQLite3
        nlohmann_json::nlohmann_json
        tomlplusplus::tomlplusplus
        stdc++exp # 依然需要该库以支持 C++23 的实验性功能（如 std::expected/std::println 的部分实现）
    )

    # 配置预编译头 (PCH) - 使用绝对路径
    # 使用 PROJECT_SOURCE_DIR
    # target_precompile_headers 会自动处理 -include 标志，无需手动添加 target_compile_options
    if(ENABLE_PCH)
        target_precompile_headers(${TARGET_NAME} PRIVATE "${PROJECT_SOURCE_DIR}/src/pch.hpp")
    endif()

    if(ENABLE_CLANG_TIDY AND CLANG_TIDY_EXE)
        set_target_properties(${TARGET_NAME} PROPERTIES 
            CXX_CLANG_TIDY "${CLANG_TIDY_EXE}"
        )
    endif()
    
    # ==================== [警告等级] ====================
    # 根据 WARNING_LEVEL 的值来分级设置警告
    if(WARNING_LEVEL GREATER_EQUAL 1)
        message(STATUS "Applying Warning Level 1: -Wall")
        target_compile_options(${TARGET_NAME} PRIVATE -Wall)
    endif()

    if(WARNING_LEVEL GREATER_EQUAL 2)
        message(STATUS "Applying Warning Level 2: -Wextra -Wpedantic")
        target_compile_options(${TARGET_NAME} PRIVATE -Wextra -Wpedantic)
    endif()

    if(WARNING_LEVEL GREATER_EQUAL 3)
        message(STATUS "Applying Warning Level 3 (Strict): -Werror")
        target_compile_options(${TARGET_NAME} PRIVATE -Werror)
    endif()
    # ====================================================

    # 为 Windows 平台添加图标资源
    if(WIN32 AND CMAKE_RC_COMPILER AND ENABLE_APP_ICON)
        get_target_property(TARGET_TYPE ${TARGET_NAME} TYPE)
        if(TARGET_TYPE STREQUAL "EXECUTABLE")
            # 使用 PROJECT_SOURCE_DIR 确保路径正确
            target_sources(${TARGET_NAME} PRIVATE "${PROJECT_SOURCE_DIR}/src/resources/app_icon.rc")
            message(STATUS "Icon resource added to target: ${TARGET_NAME}")
        endif()
    endif()
endfunction()
# 模块2：目标属性配置 (TargetSetup.cmake)
# 定义一个函数来避免为每个目标重复设置属性

# ==================== [配置项] ====================
# 定义一个变量来控制警告级别，允许从命令行 -DWARNING_LEVEL=X 覆盖
# 0=无, 1=-Wall, 2=-Wall -Wextra -Wpedantic, 3=级别2 + -Werror
set(WARNING_LEVEL 2 CACHE STRING "Set compiler warning level (0-3)")
# ====================================================

function(setup_project_target TARGET_NAME)
    # 添加头文件搜索路径
    # 使用 PROJECT_SOURCE_DIR 确保路径始终从项目根目录开始，支持作为子项目构建
    target_include_directories(${TARGET_NAME} PRIVATE "${PROJECT_SOURCE_DIR}/src")

    # 链接库
    # TODO: 2026年后关注 std::format 兼容性。
    # 目前强制链接 stdc++exp 是为了支持实验性 format
    target_link_libraries(${TARGET_NAME} PRIVATE
        SQLite::SQLite3
        nlohmann_json::nlohmann_json
        tomlplusplus::tomlplusplus
        stdc++exp
    )

    # 配置预编译头 (PCH) - 使用绝对路径
    # 使用 PROJECT_SOURCE_DIR
    # target_precompile_headers 会自动处理 -include 标志，无需手动添加 target_compile_options
    target_precompile_headers(${TARGET_NAME} PRIVATE "${PROJECT_SOURCE_DIR}/src/pch.hpp")

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
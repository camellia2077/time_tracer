# 模块2：目标属性配置 (TargetSetup.cmake)
# 定义一个函数来避免为每个目标重复设置属性

# ==================== [核心修改] ====================
# 定义一个变量来控制警告级别，允许从命令行 -DWARNING_LEVEL=X 覆盖
# 0=无, 1=-Wall, 2=-Wall -Wextra -Wpedantic, 3=级别2 + -Werror
set(WARNING_LEVEL 2 CACHE STRING "Set compiler warning level (0-3)")
# ====================================================

function(setup_project_target TARGET_NAME)
    # 添加头文件搜索路径
    target_include_directories(${TARGET_NAME} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src")

    # 链接库
    target_link_libraries(${TARGET_NAME} PRIVATE
        SQLite::SQLite3
        nlohmann_json::nlohmann_json
        stdc++exp
    )

    # 配置预编译头 (PCH) - 使用绝对路径
    target_precompile_headers(${TARGET_NAME} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src/pch.hpp")

    # ==================== [核心修改] ====================
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

    # 自动包含PCH头文件 - 使用绝对路径
    target_compile_options(${TARGET_NAME} PRIVATE -include "${CMAKE_CURRENT_SOURCE_DIR}/src/pch.hpp")

    # 为 Windows 平台添加图标资源
    if(WIN32 AND CMAKE_RC_COMPILER)
        target_sources(${TARGET_NAME} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src/resources/app_icon.rc")
    endif()
endfunction()
# 模块2：目标属性配置 (TargetSetup.cmake)
# 定义一个函数来避免为每个目标重复设置属性

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

    # 设置通用编译选项
    target_compile_options(${TARGET_NAME} PRIVATE -Wall)

    # 自动包含PCH头文件 - 使用绝对路径
    target_compile_options(${TARGET_NAME} PRIVATE -include "${CMAKE_CURRENT_SOURCE_DIR}/src/pch.hpp")

    # 为 Windows 平台添加图标资源
    if(WIN32 AND CMAKE_RC_COMPILER)
        target_sources(${TARGET_NAME} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src/resources/app_icon.rc")
    endif()
endfunction()
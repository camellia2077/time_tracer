# cmake/AddFormatTarget.cmake

# 1. 查找工具
find_program(CLANG_FORMAT_EXE NAMES "clang-format")

if(CLANG_FORMAT_EXE)
    message(STATUS "Found clang-format: ${CLANG_FORMAT_EXE}")

    # 2. 自动扫描所有源文件与头文件
    # 逻辑修补：由于项目采用了 Clean Architecture，源文件分布在 src/ 下的多个层级中。
    # 采用递归扫描以确保所有新层（api, application, domain, infrastructure, shared）都能被覆盖。
    file(GLOB_RECURSE ALL_FORMAT_SOURCES
        "src/*.cpp"
        "src/*.hpp"
        "src/*.h"
        # 排除外部生成的或不需要格式化的文件（如有需要可在下面添加 EXCLUDE 逻辑）
    )

    # 4. 建立单独的任务以绕过 Windows 命令行长度限制
    set(CHECK_DEP_TARGETS "")
    set(PREV_FORMAT_TARGET "")
    set(COUNTER 0)

    foreach(FILE_PATH ${ALL_FORMAT_SOURCES})
        math(EXPR COUNTER "${COUNTER} + 1")
        
        # --- 目标 A: 格式化 (串行链条以防止头文件并发读写冲突) ---
        set(FILE_FORMAT_TARGET "format_step_${COUNTER}")
        add_custom_target(${FILE_FORMAT_TARGET}
            COMMAND ${CLANG_FORMAT_EXE} -i -style=file "${FILE_PATH}"
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
            COMMENT "[${COUNTER}/FORMAT] Formatting: ${FILE_PATH}"
            VERBATIM
        )
        if(PREV_FORMAT_TARGET)
            add_dependencies(${FILE_FORMAT_TARGET} ${PREV_FORMAT_TARGET})
        endif()
        set(PREV_FORMAT_TARGET ${FILE_FORMAT_TARGET})

        # --- 目标 B: 检查格式 (并行执行以提升速度) ---
        set(FILE_CHECK_TARGET "check_format_step_${COUNTER}")
        add_custom_target(${FILE_CHECK_TARGET}
            COMMAND ${CLANG_FORMAT_EXE} --dry-run --Werror -style=file "${FILE_PATH}"
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
            COMMENT "[${COUNTER}/CHECK-FORMAT] Checking: ${FILE_PATH}"
            VERBATIM
        )
        list(APPEND CHECK_DEP_TARGETS ${FILE_CHECK_TARGET})
    endforeach()

    # 5. 定义顶层汇总目标
    add_custom_target(format)
    if(PREV_FORMAT_TARGET)
        add_dependencies(format ${PREV_FORMAT_TARGET})
    endif()

    add_custom_target(check-format)
    if(CHECK_DEP_TARGETS)
        add_dependencies(check-format ${CHECK_DEP_TARGETS})
    endif()
else()
    message(WARNING "clang-format not found. 'format' target will not be available.")
endif()
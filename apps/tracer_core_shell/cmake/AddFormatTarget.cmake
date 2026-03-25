# cmake/AddFormatTarget.cmake

# 1. 查找工具
find_program(CLANG_FORMAT_EXE NAMES "clang-format")

if(CLANG_FORMAT_EXE)
    message(STATUS "Found clang-format: ${CLANG_FORMAT_EXE}")

    # 2. 自动扫描 shell host 的真实源码家族。
    # 这里不能继续假设存在 src/ 根目录；tracer_core_shell 现在使用 api/、host/、tests/
    # 和少量根目录 PCH 头文件作为真实源码入口。
    set(FORMAT_GLOB_PATTERNS
        "${CMAKE_CURRENT_SOURCE_DIR}/api/*.c"
        "${CMAKE_CURRENT_SOURCE_DIR}/api/*.cc"
        "${CMAKE_CURRENT_SOURCE_DIR}/api/*.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/api/*.cxx"
        "${CMAKE_CURRENT_SOURCE_DIR}/api/*.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/api/*.hh"
        "${CMAKE_CURRENT_SOURCE_DIR}/api/*.hpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/api/*.hxx"
        "${CMAKE_CURRENT_SOURCE_DIR}/api/*.inc"
        "${CMAKE_CURRENT_SOURCE_DIR}/api/*.ipp"
        "${CMAKE_CURRENT_SOURCE_DIR}/api/*.ixx"
        "${CMAKE_CURRENT_SOURCE_DIR}/api/*.cppm"
        "${CMAKE_CURRENT_SOURCE_DIR}/host/*.c"
        "${CMAKE_CURRENT_SOURCE_DIR}/host/*.cc"
        "${CMAKE_CURRENT_SOURCE_DIR}/host/*.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/host/*.cxx"
        "${CMAKE_CURRENT_SOURCE_DIR}/host/*.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/host/*.hh"
        "${CMAKE_CURRENT_SOURCE_DIR}/host/*.hpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/host/*.hxx"
        "${CMAKE_CURRENT_SOURCE_DIR}/host/*.inc"
        "${CMAKE_CURRENT_SOURCE_DIR}/host/*.ipp"
        "${CMAKE_CURRENT_SOURCE_DIR}/host/*.ixx"
        "${CMAKE_CURRENT_SOURCE_DIR}/host/*.cppm"
        "${CMAKE_CURRENT_SOURCE_DIR}/tests/*.c"
        "${CMAKE_CURRENT_SOURCE_DIR}/tests/*.cc"
        "${CMAKE_CURRENT_SOURCE_DIR}/tests/*.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/tests/*.cxx"
        "${CMAKE_CURRENT_SOURCE_DIR}/tests/*.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/tests/*.hh"
        "${CMAKE_CURRENT_SOURCE_DIR}/tests/*.hpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/tests/*.hxx"
        "${CMAKE_CURRENT_SOURCE_DIR}/tests/*.inc"
        "${CMAKE_CURRENT_SOURCE_DIR}/tests/*.ipp"
        "${CMAKE_CURRENT_SOURCE_DIR}/tests/*.ixx"
        "${CMAKE_CURRENT_SOURCE_DIR}/tests/*.cppm"
        "${CMAKE_CURRENT_SOURCE_DIR}/pch*.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/pch*.hpp"
    )
    file(GLOB_RECURSE ALL_FORMAT_SOURCES CONFIGURE_DEPENDS
        ${FORMAT_GLOB_PATTERNS}
    )
    list(REMOVE_DUPLICATES ALL_FORMAT_SOURCES)

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

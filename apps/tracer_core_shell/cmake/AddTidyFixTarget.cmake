# cmake/AddTidyFixTarget.cmake

find_program(CLANG_TIDY_EXE NAMES "clang-tidy")

if(CLANG_TIDY_EXE)
    message(STATUS "Found clang-tidy for fix: ${CLANG_TIDY_EXE}")

    option(TIDY_WARNINGS_AS_ERRORS "Treat clang-tidy warnings as errors" OFF)
    set(TIDY_ERR_FLAG "")
    if(TIDY_WARNINGS_AS_ERRORS)
        set(TIDY_ERR_FLAG "--warnings-as-errors=*")
        message(STATUS "Clang-tidy fail-fast mode (warnings-as-errors) ENABLED.")
    endif()
    # 统一支持由外部工具链注入过滤规则；未注入时使用默认值。
    if(NOT DEFINED TT_CLANG_TIDY_HEADER_FILTER OR "${TT_CLANG_TIDY_HEADER_FILTER}" STREQUAL "")
        set(TT_CLANG_TIDY_HEADER_FILTER "^(?!.*[\\\\/]_deps[\\\\/]).*")
    endif()
    if(NOT DEFINED TT_ANALYSIS_COMPILE_DB_DIR OR "${TT_ANALYSIS_COMPILE_DB_DIR}" STREQUAL "")
        set(TT_ANALYSIS_COMPILE_DB_DIR "${CMAKE_BINARY_DIR}/analysis_compile_db")
    endif()

    # 1. 汇总源文件
    # 逻辑修补：采用递归扫描以匹配项目新的分层架构（api, application, domain, infrastructure, shared）。
    # 注意：我们只对 .cpp 进行 Tidy 检查，因为头文件会被包含在 .cpp 中一同分析。
    set(ALL_TIDY_SOURCES "")
    if(DEFINED TT_CLANG_TIDY_SOURCE_ROOTS AND NOT "${TT_CLANG_TIDY_SOURCE_ROOTS}" STREQUAL "")
        set(TT_CLANG_TIDY_SCOPE_ROOTS "${TT_CLANG_TIDY_SOURCE_ROOTS}")
        message(STATUS
            "Using scoped clang-tidy source roots"
            " (scope=${TT_CLANG_TIDY_SOURCE_SCOPE}): ${TT_CLANG_TIDY_SCOPE_ROOTS}"
        )
        foreach(TIDY_SCOPE_ROOT ${TT_CLANG_TIDY_SCOPE_ROOTS})
            if(NOT IS_ABSOLUTE "${TIDY_SCOPE_ROOT}")
                get_filename_component(
                    TIDY_SCOPE_ROOT
                    "${CMAKE_SOURCE_DIR}/${TIDY_SCOPE_ROOT}"
                    ABSOLUTE
                )
            endif()
            if(NOT EXISTS "${TIDY_SCOPE_ROOT}")
                message(WARNING "clang-tidy scope root does not exist: ${TIDY_SCOPE_ROOT}")
                continue()
            endif()
            file(
                GLOB_RECURSE TIDY_SCOPE_SOURCES
                LIST_DIRECTORIES false
                "${TIDY_SCOPE_ROOT}/*.cpp"
            )
            list(APPEND ALL_TIDY_SOURCES ${TIDY_SCOPE_SOURCES})
        endforeach()
    else()
        file(
            GLOB_RECURSE ALL_TIDY_SOURCES
            LIST_DIRECTORIES false
            RELATIVE "${CMAKE_SOURCE_DIR}"
            "src/*.cpp"
        )
    endif()
    list(REMOVE_DUPLICATES ALL_TIDY_SOURCES)

    # 在非 Android 平台执行 tidy 时，跳过 Android 专属实现，避免 JNI 头缺失导致中断。
    if(NOT ANDROID)
        list(LENGTH ALL_TIDY_SOURCES TIDY_SOURCE_COUNT_BEFORE_FILTER)
        set(TIDY_SOURCES_NON_ANDROID "")
        foreach(FILE_PATH ${ALL_TIDY_SOURCES})
            string(REPLACE "\\" "/" FILE_PATH_NORMALIZED "${FILE_PATH}")
            if(NOT FILE_PATH_NORMALIZED MATCHES ".*/android/.*\\.cpp$"
               AND NOT FILE_PATH_NORMALIZED MATCHES ".*/android_.*\\.cpp$")
                list(APPEND TIDY_SOURCES_NON_ANDROID "${FILE_PATH}")
            endif()
        endforeach()
        set(ALL_TIDY_SOURCES ${TIDY_SOURCES_NON_ANDROID})
        list(LENGTH ALL_TIDY_SOURCES TIDY_SOURCE_COUNT_AFTER_FILTER)
        math(EXPR TIDY_ANDROID_EXCLUDED_COUNT
            "${TIDY_SOURCE_COUNT_BEFORE_FILTER} - ${TIDY_SOURCE_COUNT_AFTER_FILTER}"
        )
        if(TIDY_ANDROID_EXCLUDED_COUNT GREATER 0)
            message(STATUS
                "Non-Android tidy: excluded ${TIDY_ANDROID_EXCLUDED_COUNT} Android-related source files."
            )
        endif()
    endif()

    # 2. 创建顶层目标
    add_custom_target(tidy-fix)

    # 3. 构建串行任务链 (用于分析+修复)
    # 我们将为每个文件创建一个独立的 Target，并强制它们按顺序执行。
    # 这样做的目的是：
    # A. 让 Ninja 显示进度条 [x/N]
    # B. 避免并行修改头文件导致的文件损坏 (Race Condition)
    
    set(PREV_FIX_TARGET "")
    set(PREV_CHECK_TARGET "")
    set(COUNTER 0)
    
    foreach(FILE_PATH ${ALL_TIDY_SOURCES})
        math(EXPR COUNTER "${COUNTER} + 1")
        set(TIDY_SOURCE_FILE "${FILE_PATH}")
        if(NOT IS_ABSOLUTE "${TIDY_SOURCE_FILE}")
            get_filename_component(
                TIDY_SOURCE_FILE
                "${CMAKE_SOURCE_DIR}/${TIDY_SOURCE_FILE}"
                ABSOLUTE
            )
        endif()
        
        # --- 目标 A: tidy-fix (含修复) ---
        set(CURRENT_FIX_TARGET "tidy_fix_step_${COUNTER}")
        add_custom_target(${CURRENT_FIX_TARGET}
            COMMAND ${CLANG_TIDY_EXE} 
                -p ${TT_ANALYSIS_COMPILE_DB_DIR} 
                --fix 
                --format-style=file 
                "-header-filter=${TT_CLANG_TIDY_HEADER_FILTER}"
                ${TIDY_ERR_FLAG}
                "${TIDY_SOURCE_FILE}"
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
            COMMENT "[${COUNTER}/FIX] Analyzing and Fixing: ${FILE_PATH}"
            VERBATIM
        )
        if(PREV_FIX_TARGET)
            add_dependencies(${CURRENT_FIX_TARGET} ${PREV_FIX_TARGET})
        endif()
        set(PREV_FIX_TARGET ${CURRENT_FIX_TARGET})

        # --- 目标 B: tidy (仅检查) ---
        set(CURRENT_CHECK_TARGET "tidy_check_step_${COUNTER}")
        add_custom_target(${CURRENT_CHECK_TARGET}
            COMMAND ${CLANG_TIDY_EXE} 
                -p ${TT_ANALYSIS_COMPILE_DB_DIR} 
                --format-style=file 
                "-header-filter=${TT_CLANG_TIDY_HEADER_FILTER}"
                ${TIDY_ERR_FLAG}
                "${TIDY_SOURCE_FILE}"
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
            COMMENT "[${COUNTER}/CHECK] Analyzing: ${FILE_PATH}"
            VERBATIM
        )
        # REMOVED: add_dependencies(${CURRENT_CHECK_TARGET} ${PREV_CHECK_TARGET})
        # 移除上述依赖后，Ninja 就可以并行执行所有检查任务，显著提升速度。
        list(APPEND ALL_CHECK_TARGETS ${CURRENT_CHECK_TARGET})
    endforeach()

    # 4. 设置顶层目标依赖
    add_custom_target(tidy)
    if(ALL_CHECK_TARGETS)
        add_dependencies(tidy ${ALL_CHECK_TARGETS})
    endif()

    if(PREV_FIX_TARGET)
        add_dependencies(tidy-fix ${PREV_FIX_TARGET})
        message(STATUS "Configured tidy and tidy-fix chains for ${COUNTER} files.")
    endif()

else()
    message(WARNING "clang-tidy not found. 'tidy-fix' target will not be available.")
endif()

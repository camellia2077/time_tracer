# clang-tidy fix target

find_program(CLANG_TIDY_EXE NAMES "clang-tidy")

if(CLANG_TIDY_EXE)
    add_custom_target(tidy)
    add_custom_target(tidy-fix)
    if(NOT DEFINED TT_CLANG_TIDY_HEADER_FILTER OR "${TT_CLANG_TIDY_HEADER_FILTER}" STREQUAL "")
        set(TT_CLANG_TIDY_HEADER_FILTER "^(?!.*[\\\\/]_deps[\\\\/]).*")
    endif()
    if(NOT DEFINED TT_ANALYSIS_COMPILE_DB_DIR OR "${TT_ANALYSIS_COMPILE_DB_DIR}" STREQUAL "")
        set(TT_ANALYSIS_COMPILE_DB_DIR "${CMAKE_BINARY_DIR}/analysis_compile_db")
    endif()

    list(LENGTH SOURCES TOTAL_SOURCES)

    set(PREV_TARGET "")
    set(COUNTER 0)

    foreach(FILE_PATH ${SOURCES})
        math(EXPR COUNTER "${COUNTER} + 1")
        set(TIDY_SOURCE_FILE "${FILE_PATH}")
        if(NOT IS_ABSOLUTE "${TIDY_SOURCE_FILE}")
            get_filename_component(
                TIDY_SOURCE_FILE
                "${CMAKE_SOURCE_DIR}/${TIDY_SOURCE_FILE}"
                ABSOLUTE
            )
        endif()
        set(CURRENT_TARGET "tidy_check_step_${COUNTER}")
        set(TIDY_TASK_MARKER "[${COUNTER}/${TOTAL_SOURCES}] [${COUNTER}/CHECK] Analyzing: ${FILE_PATH}")

        add_custom_target(${CURRENT_TARGET}
            COMMAND ${CMAKE_COMMAND} -E echo "${TIDY_TASK_MARKER}"
            COMMAND ${CLANG_TIDY_EXE}
                -p ${TT_ANALYSIS_COMPILE_DB_DIR}
                --format-style=file
                "-header-filter=${TT_CLANG_TIDY_HEADER_FILTER}"
                "${TIDY_SOURCE_FILE}"
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
            COMMENT "[${COUNTER}] Analyzing: ${FILE_PATH}"
            VERBATIM
        )

        if(PREV_TARGET)
            add_dependencies(${CURRENT_TARGET} ${PREV_TARGET})
        endif()
        set(PREV_TARGET ${CURRENT_TARGET})
    endforeach()

    if(PREV_TARGET)
        add_dependencies(tidy ${PREV_TARGET})
        message(STATUS "Configured tidy chain for ${COUNTER} files.")
    endif()

    set(PREV_TARGET "")
    set(COUNTER 0)

    foreach(FILE_PATH ${SOURCES})
        math(EXPR COUNTER "${COUNTER} + 1")
        set(TIDY_SOURCE_FILE "${FILE_PATH}")
        if(NOT IS_ABSOLUTE "${TIDY_SOURCE_FILE}")
            get_filename_component(
                TIDY_SOURCE_FILE
                "${CMAKE_SOURCE_DIR}/${TIDY_SOURCE_FILE}"
                ABSOLUTE
            )
        endif()
        set(CURRENT_TARGET "tidy_fix_step_${COUNTER}")
        set(TIDY_TASK_MARKER "[${COUNTER}/${TOTAL_SOURCES}] [${COUNTER}/CHECK] Analyzing: ${FILE_PATH}")

        add_custom_target(${CURRENT_TARGET}
            COMMAND ${CMAKE_COMMAND} -E echo "${TIDY_TASK_MARKER}"
            COMMAND ${CLANG_TIDY_EXE}
                -p ${TT_ANALYSIS_COMPILE_DB_DIR}
                --fix
                --format-style=file
                "-header-filter=${TT_CLANG_TIDY_HEADER_FILTER}"
                "${TIDY_SOURCE_FILE}"
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
            COMMENT "[${COUNTER}] Analyzing and Fixing: ${FILE_PATH}"
            VERBATIM
        )

        if(PREV_TARGET)
            add_dependencies(${CURRENT_TARGET} ${PREV_TARGET})
        endif()
        set(PREV_TARGET ${CURRENT_TARGET})
    endforeach()

    if(PREV_TARGET)
        add_dependencies(tidy-fix ${PREV_TARGET})
        message(STATUS "Configured tidy-fix chain for ${COUNTER} files.")
    endif()
else()
    message(WARNING "clang-tidy not found. 'tidy' and 'tidy-fix' targets will not be available.")
endif()

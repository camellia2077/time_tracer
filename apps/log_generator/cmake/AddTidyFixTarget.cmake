# clang-tidy fix target

find_program(CLANG_TIDY_EXE NAMES "clang-tidy")

if(CLANG_TIDY_EXE)
    add_custom_target(tidy-fix)

    set(PREV_TARGET "")
    set(COUNTER 0)

    foreach(FILE_PATH ${SOURCES})
        math(EXPR COUNTER "${COUNTER} + 1")
        set(CURRENT_TARGET "tidy_step_${COUNTER}")

        add_custom_target(${CURRENT_TARGET}
            COMMAND ${CLANG_TIDY_EXE}
                -p ${CMAKE_BINARY_DIR}
                --fix
                --format-style=file
                "${FILE_PATH}"
            WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
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
    message(WARNING "clang-tidy not found. 'tidy-fix' target will not be available.")
endif()

# tracer_windows_cli/cmake/AddLintTargets.cmake
#
# Split lint targets into:
# - core (reused from apps/tracer_core subdirectory targets)
# - cli  (apps/tracer_cli/windows/src only)
# - all  (core + cli)

set(TRACER_WINDOWS_CLI_LINT_ROOT "${PROJECT_SOURCE_DIR}/src")

find_program(TRACER_WINDOWS_CLANG_FORMAT_EXE NAMES "clang-format")
find_program(TRACER_WINDOWS_CLANG_TIDY_EXE NAMES "clang-tidy")
if(NOT DEFINED TT_CLANG_TIDY_HEADER_FILTER OR "${TT_CLANG_TIDY_HEADER_FILTER}" STREQUAL "")
    set(TT_CLANG_TIDY_HEADER_FILTER "^(?!.*[\\\\/]_deps[\\\\/]).*")
endif()

file(
    GLOB_RECURSE TRACER_WINDOWS_CLI_FORMAT_SOURCES
    LIST_DIRECTORIES false
    "${TRACER_WINDOWS_CLI_LINT_ROOT}/*.cpp"
    "${TRACER_WINDOWS_CLI_LINT_ROOT}/*.hpp"
    "${TRACER_WINDOWS_CLI_LINT_ROOT}/*.h"
)

file(
    GLOB_RECURSE TRACER_WINDOWS_CLI_TIDY_SOURCES
    LIST_DIRECTORIES false
    "${TRACER_WINDOWS_CLI_LINT_ROOT}/*.cpp"
)

# ------------------------------------------------------------------------------
# Core aliases (targets provided by embedded apps/tracer_core subdirectory)
# ------------------------------------------------------------------------------
if(TARGET format)
    add_custom_target(format_core)
    add_dependencies(format_core format)
endif()

if(TARGET check-format)
    add_custom_target(check_format_core)
    add_dependencies(check_format_core check-format)
endif()

if(TARGET tidy)
    add_custom_target(tidy_core)
    add_dependencies(tidy_core tidy)
endif()

if(TARGET tidy-fix)
    add_custom_target(tidy_fix_core)
    add_dependencies(tidy_fix_core tidy-fix)
endif()

# ------------------------------------------------------------------------------
# CLI clang-format targets
# ------------------------------------------------------------------------------
if(TRACER_WINDOWS_CLANG_FORMAT_EXE)
    set(TRACER_WINDOWS_PREV_FORMAT_TARGET "")
    set(TRACER_WINDOWS_CHECK_FORMAT_TARGETS "")
    set(TRACER_WINDOWS_FORMAT_COUNTER 0)

    foreach(FILE_PATH ${TRACER_WINDOWS_CLI_FORMAT_SOURCES})
        math(EXPR TRACER_WINDOWS_FORMAT_COUNTER
             "${TRACER_WINDOWS_FORMAT_COUNTER} + 1")

        set(FILE_FORMAT_TARGET
            "format_cli_step_${TRACER_WINDOWS_FORMAT_COUNTER}")
        add_custom_target(
            ${FILE_FORMAT_TARGET}
            COMMAND ${TRACER_WINDOWS_CLANG_FORMAT_EXE} -i -style=file "${FILE_PATH}"
            WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
            COMMENT "[CLI ${TRACER_WINDOWS_FORMAT_COUNTER}/FORMAT] Formatting: ${FILE_PATH}"
            VERBATIM
        )
        if(TRACER_WINDOWS_PREV_FORMAT_TARGET)
            add_dependencies(${FILE_FORMAT_TARGET}
                             ${TRACER_WINDOWS_PREV_FORMAT_TARGET})
        endif()
        set(TRACER_WINDOWS_PREV_FORMAT_TARGET ${FILE_FORMAT_TARGET})

        set(FILE_CHECK_FORMAT_TARGET
            "check_format_cli_step_${TRACER_WINDOWS_FORMAT_COUNTER}")
        add_custom_target(
            ${FILE_CHECK_FORMAT_TARGET}
            COMMAND ${TRACER_WINDOWS_CLANG_FORMAT_EXE} --dry-run --Werror -style=file "${FILE_PATH}"
            WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
            COMMENT "[CLI ${TRACER_WINDOWS_FORMAT_COUNTER}/CHECK-FORMAT] Checking: ${FILE_PATH}"
            VERBATIM
        )
        list(APPEND TRACER_WINDOWS_CHECK_FORMAT_TARGETS
             ${FILE_CHECK_FORMAT_TARGET})
    endforeach()

    add_custom_target(format_cli)
    if(TRACER_WINDOWS_PREV_FORMAT_TARGET)
        add_dependencies(format_cli ${TRACER_WINDOWS_PREV_FORMAT_TARGET})
    endif()

    add_custom_target(check_format_cli)
    if(TRACER_WINDOWS_CHECK_FORMAT_TARGETS)
        add_dependencies(check_format_cli ${TRACER_WINDOWS_CHECK_FORMAT_TARGETS})
    endif()
else()
    message(WARNING
            "clang-format not found for tracer_windows_cli lint targets.")
    add_custom_target(format_cli)
    add_custom_target(check_format_cli)
endif()

# ------------------------------------------------------------------------------
# CLI clang-tidy targets
# ------------------------------------------------------------------------------
if(TRACER_WINDOWS_CLANG_TIDY_EXE)
    set(TRACER_WINDOWS_PREV_TIDY_FIX_TARGET "")
    set(TRACER_WINDOWS_CHECK_TIDY_TARGETS "")
    set(TRACER_WINDOWS_TIDY_COUNTER 0)

    foreach(FILE_PATH ${TRACER_WINDOWS_CLI_TIDY_SOURCES})
        math(EXPR TRACER_WINDOWS_TIDY_COUNTER
             "${TRACER_WINDOWS_TIDY_COUNTER} + 1")

        set(FILE_TIDY_FIX_TARGET
            "tidy_fix_cli_step_${TRACER_WINDOWS_TIDY_COUNTER}")
        add_custom_target(
            ${FILE_TIDY_FIX_TARGET}
            COMMAND ${TRACER_WINDOWS_CLANG_TIDY_EXE}
                    -p ${CMAKE_BINARY_DIR}
                    --fix
                    --format-style=file
                    "-header-filter=${TT_CLANG_TIDY_HEADER_FILTER}"
                    "${FILE_PATH}"
            WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
            COMMENT "[CLI ${TRACER_WINDOWS_TIDY_COUNTER}/FIX] Analyzing and Fixing: ${FILE_PATH}"
            VERBATIM
        )
        if(TRACER_WINDOWS_PREV_TIDY_FIX_TARGET)
            add_dependencies(${FILE_TIDY_FIX_TARGET}
                             ${TRACER_WINDOWS_PREV_TIDY_FIX_TARGET})
        endif()
        set(TRACER_WINDOWS_PREV_TIDY_FIX_TARGET ${FILE_TIDY_FIX_TARGET})

        set(FILE_TIDY_CHECK_TARGET
            "tidy_check_cli_step_${TRACER_WINDOWS_TIDY_COUNTER}")
        add_custom_target(
            ${FILE_TIDY_CHECK_TARGET}
            COMMAND ${TRACER_WINDOWS_CLANG_TIDY_EXE}
                    -p ${CMAKE_BINARY_DIR}
                    --format-style=file
                    "-header-filter=${TT_CLANG_TIDY_HEADER_FILTER}"
                    "${FILE_PATH}"
            WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
            COMMENT "[CLI ${TRACER_WINDOWS_TIDY_COUNTER}/CHECK] Analyzing: ${FILE_PATH}"
            VERBATIM
        )
        list(APPEND TRACER_WINDOWS_CHECK_TIDY_TARGETS ${FILE_TIDY_CHECK_TARGET})
    endforeach()

    add_custom_target(tidy_cli)
    if(TRACER_WINDOWS_CHECK_TIDY_TARGETS)
        add_dependencies(tidy_cli ${TRACER_WINDOWS_CHECK_TIDY_TARGETS})
    endif()

    add_custom_target(tidy_fix_cli)
    if(TRACER_WINDOWS_PREV_TIDY_FIX_TARGET)
        add_dependencies(tidy_fix_cli ${TRACER_WINDOWS_PREV_TIDY_FIX_TARGET})
    endif()
else()
    message(WARNING
            "clang-tidy not found for tracer_windows_cli lint targets.")
    add_custom_target(tidy_cli)
    add_custom_target(tidy_fix_cli)
endif()

# ------------------------------------------------------------------------------
# Aggregate targets (core + cli)
# ------------------------------------------------------------------------------
add_custom_target(format_all)
if(TARGET format_core)
    add_dependencies(format_all format_core)
endif()
add_dependencies(format_all format_cli)

add_custom_target(check_format_all)
if(TARGET check_format_core)
    add_dependencies(check_format_all check_format_core)
endif()
add_dependencies(check_format_all check_format_cli)

add_custom_target(tidy_all)
if(TARGET tidy_core)
    add_dependencies(tidy_all tidy_core)
endif()
add_dependencies(tidy_all tidy_cli)

add_custom_target(tidy_fix_all)
if(TARGET tidy_fix_core)
    add_dependencies(tidy_fix_all tidy_fix_core)
endif()
add_dependencies(tidy_fix_all tidy_fix_cli)

# Optional dash-style aliases for command consistency with existing targets.
add_custom_target(check-format_all)
add_dependencies(check-format_all check_format_all)

add_custom_target(tidy-fix_all)
add_dependencies(tidy-fix_all tidy_fix_all)


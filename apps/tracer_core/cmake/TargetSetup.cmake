# Target setup utilities shared by project targets.

# Warning levels:
# 0 = no extra warnings
# 1 = -Wall
# 2 = -Wall -Wextra -Wpedantic
# 3 = level 2 + -Werror
set(WARNING_LEVEL 2 CACHE STRING "Set compiler warning level (0-3)")

# Keep normal build fast: clang-tidy is opt-in via --tidy / run_clang_tidy.sh.
option(ENABLE_CLANG_TIDY "Enable static analysis with clang-tidy" OFF)
option(ENABLE_PCH "Enable Precompiled Headers" ON)

include(CheckCXXSourceCompiles)
if(NOT DEFINED TT_CAN_LINK_STDCXXEXP)
    set(_tt_saved_required_libraries "${CMAKE_REQUIRED_LIBRARIES}")
    set(CMAKE_REQUIRED_LIBRARIES stdc++exp)
    check_cxx_source_compiles("int main() { return 0; }" TT_CAN_LINK_STDCXXEXP)
    set(CMAKE_REQUIRED_LIBRARIES "${_tt_saved_required_libraries}")
endif()

if(ENABLE_CLANG_TIDY)
    find_program(CLANG_TIDY_EXE NAMES "clang-tidy")
    if(NOT CLANG_TIDY_EXE)
        message(WARNING "clang-tidy not found. Static analysis will be disabled.")
    else()
        message(STATUS "Found clang-tidy: ${CLANG_TIDY_EXE}")
    endif()
endif()

if(NOT DEFINED TT_CLANG_TIDY_HEADER_FILTER OR "${TT_CLANG_TIDY_HEADER_FILTER}" STREQUAL "")
    set(TT_CLANG_TIDY_HEADER_FILTER "^(?!.*[\\\\/]_deps[\\\\/]).*")
endif()

function(_setup_target_common TARGET_NAME)
    set(options NO_PCH)
    set(one_value_args PCH_HEADER)
    cmake_parse_arguments(STC "${options}" "${one_value_args}" "" ${ARGN})

    target_include_directories(${TARGET_NAME} PRIVATE "${PROJECT_SOURCE_DIR}/src")

    if(ENABLE_PCH AND NOT STC_NO_PCH)
        set(TT_TARGET_PCH_HEADER "${PROJECT_SOURCE_DIR}/src/pch.hpp")
        if(STC_PCH_HEADER)
            if(IS_ABSOLUTE "${STC_PCH_HEADER}")
                set(TT_TARGET_PCH_HEADER "${STC_PCH_HEADER}")
            else()
                set(TT_TARGET_PCH_HEADER "${PROJECT_SOURCE_DIR}/${STC_PCH_HEADER}")
            endif()
        endif()
        target_precompile_headers(${TARGET_NAME} PRIVATE "${TT_TARGET_PCH_HEADER}")
    endif()

    if(ENABLE_CLANG_TIDY AND CLANG_TIDY_EXE)
        set_target_properties(${TARGET_NAME} PROPERTIES
            CXX_CLANG_TIDY "${CLANG_TIDY_EXE};-header-filter=${TT_CLANG_TIDY_HEADER_FILTER}"
        )
    endif()

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

    if(WIN32 AND CMAKE_RC_COMPILER AND ENABLE_APP_ICON)
        get_target_property(TARGET_TYPE ${TARGET_NAME} TYPE)
        if(TARGET_TYPE STREQUAL "EXECUTABLE")
            set(TT_ICON_RESOURCE_RC "${PROJECT_SOURCE_DIR}/src/resources/app_icon.rc")
            if(EXISTS "${TT_ICON_RESOURCE_RC}")
                target_sources(${TARGET_NAME} PRIVATE "${TT_ICON_RESOURCE_RC}")
                message(STATUS "Icon resource added to target: ${TARGET_NAME}")
            else()
                message(WARNING
                    "ENABLE_APP_ICON=ON but icon resource is missing for target "
                    "${TARGET_NAME}: ${TT_ICON_RESOURCE_RC}"
                )
            endif()
        endif()
    endif()
endfunction()

function(_apply_stdcxxexp_if_needed TARGET_NAME)
    set(options NO_STDCXXEXP)
    cmake_parse_arguments(STX "${options}" "" "" ${ARGN})

    if(ANDROID)
        return()
    endif()

    get_target_property(TARGET_TYPE ${TARGET_NAME} TYPE)
    if(NOT TARGET_TYPE STREQUAL "EXECUTABLE"
       AND NOT TARGET_TYPE STREQUAL "SHARED_LIBRARY"
       AND NOT TARGET_TYPE STREQUAL "MODULE_LIBRARY")
        return()
    endif()

    if(NOT STX_NO_STDCXXEXP AND TT_CAN_LINK_STDCXXEXP)
        target_link_libraries(${TARGET_NAME} PRIVATE stdc++exp)
    endif()
endfunction()

function(_apply_mingw_static_runtime_if_needed TARGET_NAME)
    set(options NO_STATIC_MINGW_RUNTIME)
    cmake_parse_arguments(STM "${options}" "" "" ${ARGN})

    if(STM_NO_STATIC_MINGW_RUNTIME)
        return()
    endif()

    if(NOT WIN32 OR NOT MINGW)
        return()
    endif()

    if(NOT TT_STATIC_MINGW_RUNTIME)
        return()
    endif()

    if(NOT CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        return()
    endif()

    get_target_property(TARGET_TYPE ${TARGET_NAME} TYPE)
    if(NOT TARGET_TYPE STREQUAL "EXECUTABLE"
       AND NOT TARGET_TYPE STREQUAL "SHARED_LIBRARY"
       AND NOT TARGET_TYPE STREQUAL "MODULE_LIBRARY")
        return()
    endif()

    if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        target_link_options(${TARGET_NAME} PRIVATE -fuse-ld=lld)
    endif()

    target_link_options(${TARGET_NAME} PRIVATE
        -static-libgcc
        -static-libstdc++
        -Wl,-Bstatic
        -lwinpthread
        -Wl,-Bdynamic
    )
endfunction()

function(setup_app_target TARGET_NAME)
    _setup_target_common(${TARGET_NAME} ${ARGN})
    _apply_stdcxxexp_if_needed(${TARGET_NAME} ${ARGN})
    _apply_mingw_static_runtime_if_needed(${TARGET_NAME} ${ARGN})
endfunction()

function(link_app_external_dependencies TARGET_NAME)
    target_link_libraries(${TARGET_NAME} PRIVATE
        SQLite::SQLite3
        nlohmann_json::nlohmann_json
        tomlplusplus::tomlplusplus
    )
endfunction()

function(setup_plugin_target TARGET_NAME)
    _setup_target_common(${TARGET_NAME} ${ARGN})
    _apply_stdcxxexp_if_needed(${TARGET_NAME} ${ARGN})
    if(DEFINED TT_STATIC_MINGW_RUNTIME_PLUGINS
       AND NOT TT_STATIC_MINGW_RUNTIME_PLUGINS)
        _apply_mingw_static_runtime_if_needed(
            ${TARGET_NAME}
            NO_STATIC_MINGW_RUNTIME
            ${ARGN}
        )
    else()
        _apply_mingw_static_runtime_if_needed(${TARGET_NAME} ${ARGN})
    endif()
endfunction()

# Backward-compatible alias: existing targets default to app profile.
function(setup_project_target TARGET_NAME)
    setup_app_target(${TARGET_NAME} ${ARGN})
endfunction()

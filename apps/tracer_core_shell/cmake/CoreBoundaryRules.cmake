# Enforce include boundary for reusable core layers.
# core := domain + application

function(enforce_core_include_boundary)
    set(options)
    set(oneValueArgs ROOT)
    set(multiValueArgs CORE_DIRS FORBIDDEN_PREFIXES FORBIDDEN_PATTERNS)
    cmake_parse_arguments(ECB "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    if(NOT ECB_ROOT)
        message(FATAL_ERROR "enforce_core_include_boundary requires ROOT.")
    endif()

    if(NOT ECB_CORE_DIRS)
        message(FATAL_ERROR "enforce_core_include_boundary requires CORE_DIRS.")
    endif()

    if(NOT ECB_FORBIDDEN_PREFIXES)
        set(ECB_FORBIDDEN_PREFIXES "api/" "infra/")
    endif()

    foreach(_core_dir IN LISTS ECB_CORE_DIRS)
        if(IS_ABSOLUTE "${_core_dir}")
            set(_scan_dir "${_core_dir}")
        else()
            set(_scan_dir "${ECB_ROOT}/${_core_dir}")
        endif()

        if(NOT EXISTS "${_scan_dir}")
            continue()
        endif()

        file(GLOB_RECURSE _core_files CONFIGURE_DEPENDS
            LIST_DIRECTORIES false
            "${_scan_dir}/*.h"
            "${_scan_dir}/*.hh"
            "${_scan_dir}/*.hpp"
            "${_scan_dir}/*.hxx"
            "${_scan_dir}/*.c"
            "${_scan_dir}/*.cc"
            "${_scan_dir}/*.cpp"
            "${_scan_dir}/*.cxx"
        )

        foreach(_file IN LISTS _core_files)
            file(STRINGS "${_file}" _include_lines
                REGEX "^[ \t]*#[ \t]*include[ \t]*[<\"].*"
            )

            foreach(_line IN LISTS _include_lines)
                foreach(_prefix IN LISTS ECB_FORBIDDEN_PREFIXES)
                    string(REGEX MATCH
                        "^[ \t]*#[ \t]*include[ \t]*[<\"]${_prefix}"
                        _forbidden_match
                        "${_line}"
                    )
                    if(_forbidden_match)
                        file(RELATIVE_PATH _relative_file "${ECB_ROOT}" "${_file}")
                        message(FATAL_ERROR
                            "[core-boundary] forbidden include in ${_relative_file}: ${_line}"
                        )
                    endif()
                endforeach()

                foreach(_pattern IN LISTS ECB_FORBIDDEN_PATTERNS)
                    string(REGEX MATCH "${_pattern}" _forbidden_pattern_match "${_line}")
                    if(_forbidden_pattern_match)
                        file(RELATIVE_PATH _relative_file "${ECB_ROOT}" "${_file}")
                        message(FATAL_ERROR
                            "[core-boundary] forbidden include pattern in "
                            "${_relative_file}: ${_line}"
                        )
                    endif()
                endforeach()
            endforeach()
        endforeach()
    endforeach()
endfunction()

function(enforce_source_content_boundary)
    set(options)
    set(oneValueArgs ROOT)
    set(multiValueArgs TARGET_DIRS FORBIDDEN_PATTERNS)
    cmake_parse_arguments(ESCB "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    if(NOT ESCB_ROOT)
        message(FATAL_ERROR "enforce_source_content_boundary requires ROOT.")
    endif()

    if(NOT ESCB_TARGET_DIRS)
        message(FATAL_ERROR
            "enforce_source_content_boundary requires TARGET_DIRS."
        )
    endif()

    if(NOT ESCB_FORBIDDEN_PATTERNS)
        message(FATAL_ERROR
            "enforce_source_content_boundary requires FORBIDDEN_PATTERNS."
        )
    endif()

    foreach(_target_dir IN LISTS ESCB_TARGET_DIRS)
        if(IS_ABSOLUTE "${_target_dir}")
            set(_scan_dir "${_target_dir}")
        else()
            set(_scan_dir "${ESCB_ROOT}/${_target_dir}")
        endif()

        if(NOT EXISTS "${_scan_dir}")
            continue()
        endif()

        file(GLOB_RECURSE _content_files CONFIGURE_DEPENDS
            LIST_DIRECTORIES false
            "${_scan_dir}/*.h"
            "${_scan_dir}/*.hh"
            "${_scan_dir}/*.hpp"
            "${_scan_dir}/*.hxx"
            "${_scan_dir}/*.c"
            "${_scan_dir}/*.cc"
            "${_scan_dir}/*.cpp"
            "${_scan_dir}/*.cxx"
        )

        foreach(_file IN LISTS _content_files)
            foreach(_pattern IN LISTS ESCB_FORBIDDEN_PATTERNS)
                file(STRINGS "${_file}" _matched_lines REGEX "${_pattern}")
                if(_matched_lines)
                    list(GET _matched_lines 0 _first_match)
                    file(RELATIVE_PATH _relative_file "${ESCB_ROOT}" "${_file}")
                    message(FATAL_ERROR
                        "[core-boundary] forbidden content pattern in "
                        "${_relative_file}: ${_first_match}"
                    )
                endif()
            endforeach()
        endforeach()
    endforeach()
endfunction()

function(enforce_core_target_link_boundary)
    set(options)
    set(oneValueArgs)
    set(multiValueArgs CORE_TARGETS FORBIDDEN_TARGETS)
    cmake_parse_arguments(ECTB "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    if(NOT ECTB_CORE_TARGETS)
        message(FATAL_ERROR
            "enforce_core_target_link_boundary requires CORE_TARGETS."
        )
    endif()

    if(NOT ECTB_FORBIDDEN_TARGETS)
        set(ECTB_FORBIDDEN_TARGETS
            "taio_lib"
            "tc_adapters_iface"
            "tc_infra_full_lib"
            "tc_infra_reports_lib"
        )
    endif()

    foreach(_core_target IN LISTS ECTB_CORE_TARGETS)
        if(NOT TARGET "${_core_target}")
            continue()
        endif()

        set(_all_links)
        get_target_property(_direct_links "${_core_target}" LINK_LIBRARIES)
        get_target_property(_interface_links "${_core_target}" INTERFACE_LINK_LIBRARIES)

        if(_direct_links)
            list(APPEND _all_links ${_direct_links})
        endif()
        if(_interface_links)
            list(APPEND _all_links ${_interface_links})
        endif()

        foreach(_link_item IN LISTS _all_links)
            foreach(_forbidden_target IN LISTS ECTB_FORBIDDEN_TARGETS)
                string(REGEX MATCH
                    "(^|[^A-Za-z0-9_])${_forbidden_target}([^A-Za-z0-9_]|$)"
                    _forbidden_target_match
                    "${_link_item}"
                )
                if(_forbidden_target_match)
                    message(FATAL_ERROR
                        "[core-boundary] target ${_core_target} links forbidden "
                        "dependency ${_forbidden_target} via: ${_link_item}"
                    )
                endif()
            endforeach()
        endforeach()
    endforeach()
endfunction()
function(enforce_app_shell_source_boundary)
    set(options)
    set(oneValueArgs ROOT)
    set(multiValueArgs ALLOWED_PREFIXES ALLOWED_FILES)
    cmake_parse_arguments(EASB "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    if(NOT EASB_ROOT)
        message(FATAL_ERROR "enforce_app_shell_source_boundary requires ROOT.")
    endif()

    if(NOT EXISTS "${EASB_ROOT}")
        return()
    endif()

    file(GLOB_RECURSE _shell_source_files
        RELATIVE "${EASB_ROOT}"
        LIST_DIRECTORIES false
        "${EASB_ROOT}/*.h"
        "${EASB_ROOT}/*.hh"
        "${EASB_ROOT}/*.hpp"
        "${EASB_ROOT}/*.hxx"
        "${EASB_ROOT}/*.c"
        "${EASB_ROOT}/*.cc"
        "${EASB_ROOT}/*.cpp"
        "${EASB_ROOT}/*.cxx"
        "${EASB_ROOT}/*.ixx"
        "${EASB_ROOT}/*.cppm"
        "${EASB_ROOT}/*.inc"
        "${EASB_ROOT}/*CMakeLists.txt"
        "${EASB_ROOT}/*.md"
    )

    foreach(_relative_file IN LISTS _shell_source_files)
        set(_allowed FALSE)

        foreach(_allowed_prefix IN LISTS EASB_ALLOWED_PREFIXES)
            string(FIND "${_relative_file}" "${_allowed_prefix}" _prefix_pos)
            if(_prefix_pos EQUAL 0)
                set(_allowed TRUE)
                break()
            endif()
        endforeach()

        if(NOT _allowed)
            foreach(_allowed_file IN LISTS EASB_ALLOWED_FILES)
                if(_relative_file STREQUAL "${_allowed_file}")
                    set(_allowed TRUE)
                    break()
                endif()
            endforeach()
        endif()

        if(NOT _allowed)
            message(FATAL_ERROR
                "[core-shell] unexpected source in apps shell: ${_relative_file}. "
                "Move business source to libs/tracer_core/src."
            )
        endif()
    endforeach()
endfunction()

function(enforce_shell_api_include_boundary)
    set(options)
    set(oneValueArgs ROOT)
    set(multiValueArgs API_DIRS FORBIDDEN_PREFIXES FORBIDDEN_PATTERNS EXEMPT_FILES)
    cmake_parse_arguments(ESAIB "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    if(NOT ESAIB_ROOT)
        message(FATAL_ERROR "enforce_shell_api_include_boundary requires ROOT.")
    endif()

    if(NOT ESAIB_API_DIRS)
        message(FATAL_ERROR "enforce_shell_api_include_boundary requires API_DIRS.")
    endif()

    if(NOT ESAIB_FORBIDDEN_PREFIXES)
        set(ESAIB_FORBIDDEN_PREFIXES
            "infra/"
            "api/shared/"
        )
    endif()

    foreach(_api_dir IN LISTS ESAIB_API_DIRS)
        if(IS_ABSOLUTE "${_api_dir}")
            set(_scan_dir "${_api_dir}")
        else()
            set(_scan_dir "${ESAIB_ROOT}/${_api_dir}")
        endif()

        if(NOT EXISTS "${_scan_dir}")
            continue()
        endif()

        file(GLOB_RECURSE _api_files CONFIGURE_DEPENDS
            RELATIVE "${ESAIB_ROOT}"
            LIST_DIRECTORIES false
            "${_scan_dir}/*.h"
            "${_scan_dir}/*.hh"
            "${_scan_dir}/*.hpp"
            "${_scan_dir}/*.hxx"
            "${_scan_dir}/*.c"
            "${_scan_dir}/*.cc"
            "${_scan_dir}/*.cpp"
            "${_scan_dir}/*.cxx"
            "${_scan_dir}/*.inc"
        )

        foreach(_relative_file IN LISTS _api_files)
            list(FIND ESAIB_EXEMPT_FILES "${_relative_file}" _exempt_index)
            if(NOT _exempt_index EQUAL -1)
                continue()
            endif()

            file(STRINGS "${ESAIB_ROOT}/${_relative_file}" _include_lines
                REGEX "^[ \t]*#[ \t]*include[ \t]*[<\"].*"
            )

            foreach(_line IN LISTS _include_lines)
                foreach(_prefix IN LISTS ESAIB_FORBIDDEN_PREFIXES)
                    string(REGEX MATCH
                        "^[ \t]*#[ \t]*include[ \t]*[<\"]${_prefix}"
                        _forbidden_match
                        "${_line}"
                    )
                    if(_forbidden_match)
                        message(FATAL_ERROR
                            "[shell-api-boundary] forbidden include in ${_relative_file}: ${_line}"
                        )
                    endif()
                endforeach()

                foreach(_pattern IN LISTS ESAIB_FORBIDDEN_PATTERNS)
                    string(REGEX MATCH "${_pattern}" _forbidden_pattern_match "${_line}")
                    if(_forbidden_pattern_match)
                        message(FATAL_ERROR
                            "[shell-api-boundary] forbidden include pattern in "
                            "${_relative_file}: ${_line}"
                        )
                    endif()
                endforeach()
            endforeach()
        endforeach()
    endforeach()
endfunction()

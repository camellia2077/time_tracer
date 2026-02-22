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
        set(ECB_FORBIDDEN_PREFIXES "api/" "infrastructure/")
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
            "tracer_adapters"
            "tti"
            "ttri"
            "time_tracker_infrastructure"
            "time_tracker_reports_infrastructure"
            "time_tracker_io_adapters"
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

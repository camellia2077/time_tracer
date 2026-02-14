# Enforce include boundary for reusable core layers.
# core := domain + application

function(enforce_core_include_boundary)
    set(options)
    set(oneValueArgs ROOT)
    set(multiValueArgs CORE_DIRS FORBIDDEN_PREFIXES)
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
            endforeach()
        endforeach()
    endforeach()
endfunction()

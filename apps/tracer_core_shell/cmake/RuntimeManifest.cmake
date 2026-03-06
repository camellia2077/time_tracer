function(_tt_json_array_from_list OUT_VAR)
    set(_result "")
    foreach(_item IN LISTS ARGN)
        string(REPLACE "\\" "\\\\" _escaped "${_item}")
        string(REPLACE "\"" "\\\"" _escaped "${_escaped}")
        if(_result STREQUAL "")
            set(_result "    \"${_escaped}\"")
        else()
            string(APPEND _result ",\n    \"${_escaped}\"")
        endif()
    endforeach()
    set(${OUT_VAR} "${_result}" PARENT_SCOPE)
endfunction()

function(tt_generate_runtime_manifest)
    if(NOT WIN32)
        return()
    endif()

    set(_required_files
        "tracer_core.dll"
        "reports_shared.dll"
        "config/config.toml"
    )

    if(NOT TT_USE_BUNDLED_SQLITE)
        list(APPEND _required_files "libsqlite3-0.dll")
    endif()
    if(NOT TT_TOML_HEADER_ONLY)
        list(APPEND _required_files "libtomlplusplus-3.dll")
    endif()
    if(WIN32 AND MINGW
       AND (NOT TT_STATIC_MINGW_RUNTIME OR NOT TT_STATIC_MINGW_RUNTIME_PLUGINS))
        list(APPEND _required_files
            "libgcc_s_seh-1.dll"
            "libstdc++-6.dll"
            "libwinpthread-1.dll"
        )
    endif()

    set(_required_dirs "config")
    set(_optional_dirs "assets")

    _tt_json_array_from_list(TT_RUNTIME_REQUIRED_FILES_JSON ${_required_files})
    _tt_json_array_from_list(TT_RUNTIME_REQUIRED_DIRS_JSON ${_required_dirs})
    _tt_json_array_from_list(TT_RUNTIME_OPTIONAL_DIRS_JSON ${_optional_dirs})

    file(MAKE_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
    configure_file(
        "${PROJECT_SOURCE_DIR}/cmake/templates/runtime_manifest.json.in"
        "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/runtime_manifest.json"
        @ONLY
    )
    message(STATUS
        "Generated runtime manifest: "
        "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/runtime_manifest.json"
    )
endfunction()

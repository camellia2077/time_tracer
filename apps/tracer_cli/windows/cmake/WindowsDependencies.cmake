include_guard(GLOBAL)

function(_tw_ensure_nlohmann_json)
    if(TARGET nlohmann_json::nlohmann_json)
        return()
    endif()

    find_package(nlohmann_json REQUIRED)
    if(NOT TARGET nlohmann_json::nlohmann_json)
        message(FATAL_ERROR
            "Windows CLI dependency missing: nlohmann_json::nlohmann_json"
        )
    endif()
endfunction()

function(configure_tracer_windows_cli_dependencies)
    _tw_ensure_nlohmann_json()
endfunction()

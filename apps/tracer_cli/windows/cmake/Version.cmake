# tracer_windows_cli/cmake/Version.cmake

set(TRACER_WINDOWS_CLI_VERSION "0.2.0")

if(NOT TRACER_WINDOWS_CLI_VERSION MATCHES "^[0-9]+\\.[0-9]+\\.[0-9]+$")
    message(FATAL_ERROR
        "Invalid TRACER_WINDOWS_CLI_VERSION: ${TRACER_WINDOWS_CLI_VERSION}. "
        "Expected MAJOR.MINOR.PATCH."
    )
endif()

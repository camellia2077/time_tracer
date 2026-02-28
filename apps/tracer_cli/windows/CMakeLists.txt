cmake_minimum_required(VERSION 3.25)

include("${CMAKE_CURRENT_LIST_DIR}/cmake/Version.cmake")

project(TracerWindowsApp VERSION "${TRACER_WINDOWS_CLI_VERSION}" LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
# Keep runtime artifacts in one place for CLI/test workflows.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

if(ANDROID)
    message(FATAL_ERROR "apps/tracer_cli/windows does not support ANDROID builds.")
endif()

set(TRACER_WINDOWS_COMPAT_SOURCE_DIR
    "${CMAKE_CURRENT_LIST_DIR}/../tracer_core"
)

# Keep tracer_windows_cli Release flags aligned with tracer_core so CLI binaries
# inherit size-oriented defaults (-Os/-s/--gc-sections, etc.).
include("${TRACER_WINDOWS_COMPAT_SOURCE_DIR}/cmake/BuildTypeFlags.cmake")

# Stage 4 bootstrap:
# Reuse tracer_core core/adapters; desktop CLI delivery lives in tracer_windows_cli.
# ttc = tracer_core_compat
set(TRACER_WINDOWS_COMPAT_BINARY_DIR "${CMAKE_BINARY_DIR}/ttc")
add_subdirectory(
    "${TRACER_WINDOWS_COMPAT_SOURCE_DIR}"
    "${TRACER_WINDOWS_COMPAT_BINARY_DIR}"
)

set(TRACER_WINDOWS_CLI_SOURCE_ROOT
    "${CMAKE_CURRENT_LIST_DIR}/src"
)
set(TRACER_WINDOWS_ASSETS_SOURCE_DIR
    "${CMAKE_CURRENT_LIST_DIR}/assets"
)
set(TRACER_WINDOWS_REQUIRED_TEMPLATE_FILES
    "templates/report_chart_base.html.tpl"
    "templates/report_chart_option_line.js.tpl"
    "templates/report_chart_option_bar.js.tpl"
    "templates/report_chart_option_pie.js.tpl"
    "templates/report_chart_option_heatmap_year.js.tpl"
    "templates/report_chart_option_heatmap_month.js.tpl"
)
foreach(template_file IN LISTS TRACER_WINDOWS_REQUIRED_TEMPLATE_FILES)
    if(NOT EXISTS "${TRACER_WINDOWS_ASSETS_SOURCE_DIR}/${template_file}")
        message(FATAL_ERROR
            "Missing chart template: "
            "${TRACER_WINDOWS_ASSETS_SOURCE_DIR}/${template_file}"
        )
    endif()
endforeach()
if(NOT EXISTS "${TRACER_WINDOWS_ASSETS_SOURCE_DIR}/vendor/echarts/echarts.min.js")
    message(FATAL_ERROR
        "Missing vendored ECharts script: "
        "${TRACER_WINDOWS_ASSETS_SOURCE_DIR}/vendor/echarts/echarts.min.js"
    )
endif()
# twcli = tracer_windows_cli
set(TRACER_WINDOWS_CLI_BINARY_DIR "${CMAKE_BINARY_DIR}/twcli")
add_subdirectory(
    "${TRACER_WINDOWS_CLI_SOURCE_ROOT}/api/cli"
    "${TRACER_WINDOWS_CLI_BINARY_DIR}"
)

include("${CMAKE_CURRENT_LIST_DIR}/cmake/AddLintTargets.cmake")

add_executable(time_tracer_cli
    "${TRACER_WINDOWS_CLI_SOURCE_ROOT}/api/cli/main.cpp"
)
setup_app_target(time_tracer_cli NO_PCH)
target_include_directories(time_tracer_cli PRIVATE
    "${TRACER_WINDOWS_COMPAT_SOURCE_DIR}/src"
)
if(WIN32)
    target_link_libraries(time_tracer_cli PRIVATE
        tracer_windows_app
    )
else()
    target_link_libraries(time_tracer_cli PRIVATE
        tracer_cli_app
    )
endif()

add_dependencies(time_tracer_cli tracer_core_shared)
add_custom_command(
    TARGET time_tracer_cli POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "$<TARGET_FILE:tracer_core_shared>"
    "$<TARGET_FILE_DIR:time_tracer_cli>/"
    COMMENT "Copying tracer_core runtime DLL for time_tracer_cli"
)

add_custom_target(tracer_windows_cli_assets_sync ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${TRACER_WINDOWS_ASSETS_SOURCE_DIR}"
    "$<TARGET_FILE_DIR:time_tracer_cli>/assets"
    COMMENT "Synchronizing assets folder for time_tracer_cli"
)
add_dependencies(tracer_windows_cli_assets_sync time_tracer_cli)

set(TRACER_WINDOWS_RUNTIME_DIR "${CMAKE_BINARY_DIR}/runtime/bin")
add_custom_command(
    TARGET time_tracer_cli POST_BUILD
    COMMAND ${CMAKE_COMMAND}
        -DSOURCE_BIN="$<TARGET_FILE_DIR:time_tracer_cli>"
        -DDEST_BIN="${TRACER_WINDOWS_RUNTIME_DIR}"
        -P "${CMAKE_CURRENT_LIST_DIR}/cmake/StageRuntimeLayout.cmake"
    COMMENT "Staging runtime layout under ${TRACER_WINDOWS_RUNTIME_DIR}"
)

set(TRACER_WINDOWS_CONFIG_SOURCE_DIR ""
    CACHE PATH "Generated tracer_windows_cli config directory.")
if(TRACER_WINDOWS_CONFIG_SOURCE_DIR STREQUAL "")
    message(FATAL_ERROR
        "TRACER_WINDOWS_CONFIG_SOURCE_DIR is required. "
        "Use `python scripts/run.py build --app tracer_windows_cli` "
        "or pass -DTRACER_WINDOWS_CONFIG_SOURCE_DIR=<path> manually."
    )
endif()

get_filename_component(TRACER_WINDOWS_CONFIG_SOURCE_DIR
    "${TRACER_WINDOWS_CONFIG_SOURCE_DIR}"
    ABSOLUTE
)
get_filename_component(TRACER_WINDOWS_LEGACY_SOURCE_CONFIG_DIR
    "${CMAKE_CURRENT_LIST_DIR}/../tracer_core/config"
    ABSOLUTE
)
if(TRACER_WINDOWS_CONFIG_SOURCE_DIR STREQUAL TRACER_WINDOWS_LEGACY_SOURCE_CONFIG_DIR)
    message(FATAL_ERROR
        "Invalid TRACER_WINDOWS_CONFIG_SOURCE_DIR: "
        "${TRACER_WINDOWS_CONFIG_SOURCE_DIR}. "
        "Windows CLI must consume generated config artifacts (apps/tracer_cli/windows/config), "
        "not source config (apps/tracer_core/config)."
    )
endif()

set(TRACER_WINDOWS_REQUIRED_CONFIG_FILES
    "config.toml"
    "converter/interval_processor_config.toml"
    "converter/alias_mapping.toml"
    "converter/duration_rules.toml"
    "meta/bundle.toml"
)
foreach(required_file IN LISTS TRACER_WINDOWS_REQUIRED_CONFIG_FILES)
    if(NOT EXISTS "${TRACER_WINDOWS_CONFIG_SOURCE_DIR}/${required_file}")
        message(FATAL_ERROR
            "Invalid TRACER_WINDOWS_CONFIG_SOURCE_DIR: "
            "${TRACER_WINDOWS_CONFIG_SOURCE_DIR}. "
            "Missing required config file: ${required_file}"
        )
    endif()
endforeach()

if(NOT EXISTS "${TRACER_WINDOWS_CONFIG_SOURCE_DIR}/config.toml")
    message(FATAL_ERROR
        "Invalid TRACER_WINDOWS_CONFIG_SOURCE_DIR: "
        "${TRACER_WINDOWS_CONFIG_SOURCE_DIR}. "
        "Missing config.toml."
    )
endif()
message(STATUS "Tracer Windows config source: ${TRACER_WINDOWS_CONFIG_SOURCE_DIR}")

add_custom_target(tracer_windows_cli_config_sync ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${TRACER_WINDOWS_CONFIG_SOURCE_DIR}"
    "$<TARGET_FILE_DIR:time_tracer_cli>/config"
    COMMENT "Synchronizing config folder for time_tracer_cli"
)
add_dependencies(tracer_windows_cli_config_sync time_tracer_cli)

add_custom_command(
    TARGET time_tracer_cli POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E rm -f
    "$<TARGET_FILE_DIR:time_tracer_cli>/time_tracker_cli.exe"
    COMMENT "Removing deprecated binary name: time_tracker_cli.exe"
)

if(WIN32)
    set(ALL_TARGETS time_tracer_cli)
    include("${TRACER_WINDOWS_COMPAT_SOURCE_DIR}/cmake/Win32PostBuildCopy.cmake")
endif()

add_custom_target(tracer_windows_cli ALL DEPENDS time_tracer_cli)
add_custom_target(tracer_windows_runtime_layout DEPENDS time_tracer_cli)

message(STATUS
    "Tracer Windows build enabled. apps/tracer_core is now core/android only."
)


# tracer_windows_cli/src/api/cli/CMakeLists.txt

set(TRACER_WINDOWS_CLI_SOURCE_ROOT
    "${CMAKE_CURRENT_LIST_DIR}/../.."
)
set(TRACER_WINDOWS_CLI_PROJECT_ROOT
    "${CMAKE_CURRENT_LIST_DIR}/../../.."
)
set(TRACER_WINDOWS_CORE_SOURCE_ROOT
    "${CMAKE_CURRENT_LIST_DIR}/../../../../tracer_core/src"
)

include("${TRACER_WINDOWS_CLI_PROJECT_ROOT}/cmake/WindowsDependencies.cmake")
configure_tracer_windows_cli_dependencies()

set(TIME_TRACER_CLI_CORE_SOURCES
    # Framework - Core only
    "framework/core/command_parser.cpp"
    "framework/core/arg_definitions.cpp"
    "framework/core/command_catalog.cpp"

    # Runtime bootstrap
    "../../bootstrap/cli_runtime_factory.cpp"
    "../../bootstrap/cli_runtime_factory_loader.cpp"
    "../../bootstrap/cli_runtime_factory_check.cpp"
    "../../bootstrap/cli_runtime_factory_proxy.cpp"

    # App
    "impl/app/cli_application.cpp"
    "impl/app/app_runner.cpp"

    # Utils
    "impl/utils/console_helper.cpp"
    "impl/utils/tree_formatter.cpp"
    "impl/utils/sqlite_utils.cpp"
    "${TRACER_WINDOWS_CORE_SOURCE_ROOT}/domain/utils/time_utils.cpp"
    "${TRACER_WINDOWS_CORE_SOURCE_ROOT}/shared/utils/period_utils.cpp"
)

set(TIME_TRACER_CLI_COMMAND_SOURCES
    "impl/commands/register_all_commands.cpp"
    # Commands (explicit registration entrypoints)
    "impl/commands/export/export_command.cpp"
    "impl/commands/crypto/crypto_command.cpp"
    "impl/commands/crypto/crypto_passphrase_io.cpp"
    "impl/commands/crypto/crypto_file_runner.cpp"
    "impl/commands/chart/chart_command.cpp"
    "impl/commands/general/doctor_command.cpp"
    "impl/commands/query/query_command.cpp"
    "impl/commands/query/heatmap_config.cpp"
    "impl/commands/query/data_query_parser.cpp"
    "impl/commands/query/tree_command.cpp"
    "impl/presentation/report_chart_html_exporter.cpp"
    "impl/presentation/progress/crypto_progress_renderer.cpp"
    "impl/presentation/report_chart/chart_renderer_factory.cpp"
    "impl/presentation/report_chart/renderers/line_chart_renderer.cpp"
    "impl/presentation/report_chart/renderers/bar_chart_renderer.cpp"
    "impl/presentation/report_chart/renderers/pie_chart_renderer.cpp"
    "impl/presentation/report_chart/renderers/heatmap_year_chart_renderer.cpp"
    "impl/presentation/report_chart/renderers/heatmap_month_chart_renderer.cpp"
    "impl/commands/pipeline/convert_command.cpp"
    "impl/commands/pipeline/import_command.cpp"
    "impl/commands/pipeline/ingest_command.cpp"
    "impl/commands/pipeline/validate_logic_command.cpp"
    "impl/commands/pipeline/validate_structure_command.cpp"
)

add_library(time_tracer_cli_lib STATIC ${TIME_TRACER_CLI_CORE_SOURCES})
setup_app_target(time_tracer_cli_lib NO_PCH)
string(TIMESTAMP TRACER_WINDOWS_BUILD_TIMESTAMP "%Y-%m-%dT%H:%M:%SZ" UTC)
target_compile_definitions(time_tracer_cli_lib PUBLIC
    TRACER_WINDOWS_CLI_VERSION="${PROJECT_VERSION}"
    TRACER_WINDOWS_BUILD_TIMESTAMP="${TRACER_WINDOWS_BUILD_TIMESTAMP}"
)
target_include_directories(time_tracer_cli_lib PUBLIC
    "${TRACER_WINDOWS_CLI_SOURCE_ROOT}"
)
target_include_directories(time_tracer_cli_lib PRIVATE
    "${TRACER_WINDOWS_CORE_SOURCE_ROOT}"
)
target_link_libraries(time_tracer_cli_lib PUBLIC
    tracer_transport
)

add_library(time_tracer_cli_commands STATIC ${TIME_TRACER_CLI_COMMAND_SOURCES})
setup_app_target(time_tracer_cli_commands NO_PCH)
target_include_directories(time_tracer_cli_commands PRIVATE
    "${TRACER_WINDOWS_CORE_SOURCE_ROOT}"
)
target_link_libraries(time_tracer_cli_commands PUBLIC
    time_tracer_cli_lib
)
target_link_libraries(time_tracer_cli_commands PRIVATE
    time_tracker_infrastructure
    nlohmann_json::nlohmann_json
)

# Phase 1 layering target name for Windows CLI app delivery.
add_library(tracer_cli_app INTERFACE)
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    # cli_lib <-> cli_commands has cyclic symbol usage; request linker rescan.
    target_link_libraries(tracer_cli_app INTERFACE
        "$<LINK_GROUP:RESCAN,time_tracer_cli_lib,time_tracer_cli_commands>"
    )
else()
    target_link_libraries(tracer_cli_app INTERFACE
        time_tracer_cli_lib
        time_tracer_cli_commands
    )
endif()

if(WIN32)
    add_library(tracer_windows_app INTERFACE)
    target_link_libraries(tracer_windows_app INTERFACE
        tracer_cli_app
    )
endif()

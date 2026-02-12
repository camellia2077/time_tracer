# src/cli/CMakeLists.txt

set(TIME_TRACKER_CLI_CORE_SOURCES
    # Framework - Core only
    "framework/core/command_parser.cpp"
    "framework/core/arg_definitions.cpp"

    # App
    "impl/app/cli_application.cpp"
    "impl/app/app_runner.cpp"

    # IO
    "impl/io/console_input.cpp"

    # Utils
    "impl/utils/help_formatter.cpp"
    "impl/utils/console_helper.cpp"
    "impl/utils/tree_formatter.cpp"
    "impl/utils/date_utils.cpp"
    "impl/utils/sqlite_utils.cpp"
)

set(TIME_TRACKER_CLI_COMMAND_SOURCES
    # Commands (contain static registrars)
    "impl/commands/export/export_command.cpp"
    "impl/commands/query/query_command.cpp"
    "impl/commands/query/data_query_executor.cpp"
    "impl/commands/query/data_query_parser.cpp"
    "impl/commands/query/data_query_repository.cpp"
    "impl/commands/query/data_query_statistics.cpp"
    "impl/commands/query/data_query_output.cpp"
    "impl/commands/query/tree_command.cpp"
    "impl/commands/pipeline/convert_command.cpp"
    "impl/commands/pipeline/import_command.cpp"
    "impl/commands/pipeline/ingest_command.cpp"
    "impl/commands/pipeline/validate_logic_command.cpp"
    "impl/commands/pipeline/validate_structure_command.cpp"
)

add_library(time_tracker_cli_lib STATIC ${TIME_TRACKER_CLI_CORE_SOURCES})
setup_app_target(time_tracker_cli_lib)
target_link_libraries(time_tracker_cli_lib PUBLIC
    time_tracker_application
    time_tracker_infrastructure
)

add_library(time_tracker_cli_commands STATIC ${TIME_TRACKER_CLI_COMMAND_SOURCES})
setup_app_target(time_tracker_cli_commands)
target_link_libraries(time_tracker_cli_commands PUBLIC
    time_tracker_cli_lib
    time_tracker_application
    time_tracker_infrastructure
)

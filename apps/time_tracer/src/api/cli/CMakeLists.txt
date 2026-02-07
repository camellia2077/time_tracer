# src/cli/CMakeLists.txt
add_library(time_tracker_cli_lib STATIC
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

    # Commands
    "impl/commands/export/export_command.cpp"
    "impl/commands/query/query_command.cpp"
    "impl/commands/query/data_query_executor.cpp"
    "impl/commands/query/tree_command.cpp"
    "impl/commands/pipeline/convert_command.cpp"
    "impl/commands/pipeline/import_command.cpp"
    "impl/commands/pipeline/ingest_command.cpp"
    "impl/commands/pipeline/validate_logic_command.cpp"
    "impl/commands/pipeline/validate_structure_command.cpp"
)

setup_project_target(time_tracker_cli_lib)

target_link_libraries(time_tracker_cli_lib PUBLIC
    time_tracker_application
    time_tracker_infrastructure
)

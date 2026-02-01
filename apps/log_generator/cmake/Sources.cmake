# Source Files Definition

set(SOURCES
    # Entry
    src/main.cpp
    
    # Application
    src/application/application.cpp
    src/application/config/config_handler.cpp
    src/application/workflow/workflow_handler.cpp
    src/application/reporting/report_handler.cpp
    
    # CLI
    src/cli/framework/command_line_parser.cpp
    src/cli/commands/help_command.cpp
    src/cli/commands/version_command.cpp
    
    # Infrastructure - Config
    src/infrastructure/config/config.cpp
    src/infrastructure/config/config_facade.cpp
    
    # Infrastructure - IO
    src/infrastructure/io/file_manager.cpp
    
    # Domain - Components
    src/domain/components/remark_generator.cpp
    src/domain/components/event_generator.cpp
    src/domain/components/day_generator.cpp

    # Domain - Impl
    src/domain/impl/log_generator.cpp

    # Domain - Strategies
    src/domain/strategies/sleep_scheduler.cpp
    
    # Utils
    src/utils/performance_reporter.cpp
    src/utils/utils.cpp
)

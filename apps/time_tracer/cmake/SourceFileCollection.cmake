# src/cmake/SourceFileCollection.cmake
# 模块1：源文件收集
# 职责：仅定义源文件列表变量，不创建任何构建目标。

# ----------------------------------------------------
# 最佳实践：显式列出所有源文件以确保构建系统的稳定性和可预测性。

# --- Common  ---
set(COMMON_SOURCES
    "src/common/utils/time_utils.cpp"
)
set(SERIALIZER_SOURCES
    "src/serializer/json_serializer.cpp"
    "src/serializer/core/log_deserializer.cpp"
    "src/serializer/core/log_serializer.cpp"
)

set(VALIDATOR_SOURCES

    # common 模块
    "src/validator/common/ValidatorUtils.cpp"

    # json 模块
    "src/validator/json/facade/JsonValidator.cpp"
    "src/validator/json/rules/ActivityRules.cpp"
    "src/validator/json/rules/DateRules.cpp"

    # source_txt 模块
    "src/validator/txt/facade/TextValidator.cpp"
    "src/validator/txt/rules/LineRules.cpp"
    "src/validator/txt/rules/StructureRules.cpp"
)


# --- Shared Reports Library ---
# 创建一个静态库，包含所有报告生成器共享的源文件
# 这样可以避免在主程序和多个DLL中重复编译相同的代码

set(REPORTS_SHARED_SOURCES
    "src/common/utils/string_utils.cpp"
    "src/reports/daily/common/day_base_config.cpp"
    "src/reports/daily/formatters/statistics/stat_formatter.cpp"
    "src/reports/shared/utils/config/config_utils.cpp"
    "src/reports/shared/utils/tree/project_tree_builder.cpp"

    "src/reports/shared/utils/format/bool_to_string.cpp"
    "src/reports/shared/utils/format/report_string_utils.cpp"
    "src/reports/shared/utils/format/time_format.cpp"
    "src/reports/shared/formatters/base/project_tree_formatter.cpp"

    "src/reports/shared/config/tex_style_config.cpp"
    "src/reports/shared/config/typst_style_config.cpp"

    "src/reports/shared/formatters/markdown/markdown_formatter.cpp" 
    "src/reports/shared/formatters/typst/typ_utils.cpp"
    "src/reports/shared/formatters/latex/tex_utils.cpp"
    "src/reports/shared/formatters/latex/tex_common_utils.cpp"

    "src/reports/monthly/common/month_base_config.cpp"
    "src/reports/period/common/period_base_config.cpp" 
)


# --- Time Master CLI Sources ---
set(CLI_SOURCES
    # --- Framework (Core Logic) ---
    "src/cli/framework/core/command_parser.cpp"
    "src/cli/framework/io/console_input.cpp"

    "src/cli/impl/app/cli_application.cpp"
    "src/cli/impl/utils/help_formatter.cpp"

    "src/cli/impl/commands/export/export_command.cpp"

    "src/cli/impl/commands/query/query_command.cpp"

    "src/cli/impl/commands/pipeline/convert_command.cpp"
    "src/cli/impl/commands/pipeline/import_command.cpp"
    "src/cli/impl/commands/pipeline/ingest_command.cpp"
    "src/cli/impl/commands/pipeline/validate_output_command.cpp"
    "src/cli/impl/commands/pipeline/validate_source_command.cpp"
)
# --- DB Inserter Sources ---
set(BOOTSTRAP_SOURCES
    "src/bootstrap/startup_validator.cpp"
)

# --- Importer Sources (Database Ingestion Module) ---
set(IMPORTER_SOURCES
    # Top-level Facade & Service
    "src/importer/data_importer.cpp"
    "src/importer/import_service.cpp"

    # Parser Layer (Parsing logic)
    "src/importer/parser/json_parser.cpp"
    "src/importer/parser/memory_parser.cpp"
    "src/importer/parser/details/day_parser.cpp"
    "src/importer/parser/details/activity_parser.cpp"

    # Storage Layer (Database persistence)
    "src/importer/storage/repository.cpp"
    "src/importer/storage/sqlite/writer.cpp"
    "src/importer/storage/sqlite/project_resolver.cpp"
    "src/importer/storage/sqlite/connection.cpp"
    "src/importer/storage/sqlite/statement.cpp"
)

set(REPORTS_SOURCES
    # Root
    "src/reports/report_service.cpp"
    # Export
    "src/reports/services/daily_report_service.cpp"
    "src/reports/services/monthly_report_service.cpp"
    "src/reports/services/period_report_service.cpp"
    # Daily Reports
    "src/reports/daily/queriers/day_querier.cpp"
    "src/reports/daily/queriers/day_querier.cpp"
    "src/reports/daily/queriers/batch_day_data_fetcher.cpp"
    # Monthly Reports
    "src/reports/monthly/queriers/month_querier.cpp"
    "src/reports/monthly/queriers/batch_month_data_fetcher.cpp"
    
    
    # Period Reports
    "src/reports/period/queriers/period_querier.cpp"
    "src/reports/period/queriers/batch_period_data_fetcher.cpp"

)

# --- converter Sources ---
set(CONVERTER_SOURCES
    "src/converter/log_processor.cpp"

    # --- 配置模块 (Config)  ---
    "src/converter/config/converter_config.cpp"
    "src/converter/config/toml_converter_config_loader.cpp"

    # --- 转换模块 (Convert) ---
    # Facade
    "src/converter/convert/facade/converter_service.cpp"
    
    # Core
    "src/converter/convert/core/activity_mapper.cpp"
    "src/converter/convert/core/day_processor.cpp"
    "src/converter/convert/core/day_stats.cpp"

    # IO
    "src/converter/convert/io/text_parser.cpp"
)
# --- File Handler Sources ---
set(IO_SOURCES
    "src/io/file_controller.cpp"
    "src/io/core/file_reader.cpp"
    "src/io/core/file_writer.cpp"
    "src/io/core/file_system_helper.cpp"
    "src/io/utils/file_utils.cpp"
)

set(CONFIG_SOURCES
    "src/config/config_loader.cpp"

    "src/config/internal/config_parser_utils.cpp"
    "src/config/loader/report_config_loader.cpp"
    "src/config/loader/toml_loader_utils.cpp"


    # 顶层外观
    "src/config/validator/facade/config_facade.cpp"

    # converter 领域的 Facade 和 Pipelines
    "src/config/validator/converter/facade/converter_facade.cpp"
    "src/config/validator/converter/rules/duration_rule.cpp"
    "src/config/validator/converter/rules/main_rule.cpp"
    "src/config/validator/converter/rules/mapping_rule.cpp"

    "src/config/validator/plugins/facade/plugin_validator.cpp"

    # reports 领域的 Facade
    "src/config/validator/reports/facade/query_facade.cpp"

    # reports 领域的 Strategies
    "src/config/validator/reports/strategies/base_strategy.cpp"
    "src/config/validator/reports/strategies/strategy_factory.cpp"

    # reports 领域的 Strategies
    "src/config/validator/reports/strategies/daily/daily_md.cpp"
    "src/config/validator/reports/strategies/daily/daily_tex.cpp"
    "src/config/validator/reports/strategies/daily/daily_typ.cpp"
    "src/config/validator/reports/strategies/monthly/monthly.cpp"
    "src/config/validator/reports/strategies/periodic/periodic.cpp"
)



# --- Core Sources ---
set(CORE_SOURCES
    "src/core/Workflow_handler.cpp"
    "src/core/report_handler.cpp"
    "src/core/database/db_manager.cpp"
    

    "src/core/file/pipeline_manager.cpp"

    "src/core/file/steps/file_collector.cpp"
    "src/core/file/steps/source_validator_step.cpp"
    "src/core/file/steps/converter_step.cpp"
    "src/core/file/steps/output_validator_step.cpp"

    "src/core/file/utils/converter_config_factory.cpp"
    "src/core/file/utils/processed_data_writer.cpp"

    "src/core/reporting/exporter.cpp"
    "src/core/reporting/export_utils.cpp"
    "src/core/reporting/report_file_manager.cpp"
    "src/core/reporting/report_generator.cpp"
)

message(STATUS "Source files collected explicitly.")
# src/application/CMakeLists.txt
add_library(time_tracker_application STATIC
    # Workflow
    "workflow_handler.cpp"

    # Pipeline
    "pipeline/pipeline_manager.cpp"
    "pipeline/steps/logic_linker_step.cpp"
    "pipeline/steps/converter_step.cpp"
    "pipeline/steps/file_collector.cpp"
    "pipeline/steps/structure_validator_step.cpp"
    "pipeline/steps/logic_validator_step.cpp"
    "pipeline/utils/converter_config_factory.cpp"

    # Reporting
    "reporting/report_handler.cpp"
    "reporting/generator/report_generator.cpp"
    
    # Tree
    "reporting/tree/project_tree_viewer.cpp"

    # Importer (Moved here or keep in importer module?)
    # Importer seems like Application/Domain logic. 
    # Let's include 'importer' sources here or make a separate module.
    # The SourceFileCollection had IMPORTER_SOURCES.
    "../importer/data_importer.cpp"
    "../importer/import_service.cpp"
    "../importer/parser/memory_parser.cpp"
    "../importer/parser/details/day_parser.cpp"
    "../importer/parser/details/activity_parser.cpp"
    "../importer/storage/repository.cpp"
    "../importer/storage/sqlite/writer.cpp"
    "../importer/storage/sqlite/project_resolver.cpp"
    "../importer/storage/sqlite/connection.cpp"
    "../importer/storage/sqlite/statement.cpp"

    # Converter (Moved here)
    "../converter/log_processor.cpp"
    "../converter/convert/facade/converter_service.cpp"
    "../converter/convert/core/log_linker.cpp"
    "../converter/convert/core/activity_mapper.cpp"
    "../converter/convert/core/day_processor.cpp"
    "../converter/convert/core/day_stats.cpp"
    "../adapters/input/parser/text_parser.cpp"
    
    # Validator sources (Business rules)
    "../validator/common/validator_utils.cpp"
    "../validator/json/facade/json_validator.cpp"
    "../validator/json/rules/activity_rules.cpp"
    "../validator/json/rules/date_rules.cpp"
    "../validator/txt/facade/text_validator.cpp"
    "../validator/txt/rules/line_rules.cpp"
    "../validator/txt/rules/structure_rules.cpp"

    # Report Services
    "../reports/report_service.cpp"
    "../reports/services/daily_report_service.cpp"
    "../reports/services/monthly_report_service.cpp"
    "../reports/services/weekly_report_service.cpp"
    "../reports/services/yearly_report_service.cpp"
)

setup_project_target(time_tracker_application)

target_link_libraries(time_tracker_application PUBLIC
    time_tracker_infrastructure # Application uses Infra (Repository/IO)
    time_tracker_domain
    reports_data # Reporting depends on Data Layer
    reports_shared
)

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

    # Importer
    "importer/data_importer.cpp"
    "importer/import_service.cpp"
    "parser/memory_parser.cpp"
    "parser/day_parser.cpp"
    "parser/activity_parser.cpp"
    "importer/storage/repository.cpp"
    "importer/storage/sqlite/writer.cpp"
    "importer/storage/sqlite/project_resolver.cpp"
    "importer/storage/sqlite/connection.cpp"
    "importer/storage/sqlite/statement.cpp"

    # Converter & Service (Moved to application/parser or service)
    "../domain/logic/converter/log_processor.cpp"
    "service/converter_service.cpp"
    "../domain/logic/converter/convert/core/log_linker.cpp"
    "../domain/logic/converter/convert/core/activity_mapper.cpp"
    "../domain/logic/converter/convert/core/day_processor.cpp"
    "../domain/logic/converter/convert/core/day_stats.cpp"
    "parser/text_parser.cpp"
    
    # Validator sources (Business rules, Moved to domain/logic)
    "../domain/logic/validator/common/validator_utils.cpp"
    "../domain/logic/validator/json/rules/activity_rules.cpp"
    "../domain/logic/validator/json/rules/date_rules.cpp"
    "../domain/logic/validator/txt/facade/text_validator.cpp"
    "../domain/logic/validator/txt/rules/line_rules.cpp"
    "../domain/logic/validator/txt/rules/structure_rules.cpp"

    # Struct Validator
    "../domain/logic/validator/structure/facade/struct_validator.cpp"
    "../domain/logic/validator/structure/rules/activity_rules.cpp"
    "../domain/logic/validator/structure/rules/date_rules.cpp"

)

setup_project_target(time_tracker_application)

target_link_libraries(time_tracker_application PUBLIC
    time_tracker_infrastructure # Application uses Infra (Repository/IO)
    time_tracker_domain
    reports_data # Reporting depends on Data Layer
    reports_shared
)

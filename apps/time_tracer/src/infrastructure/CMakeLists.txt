# src/infrastructure/CMakeLists.txt
add_subdirectory(reports)

add_library(time_tracker_infrastructure STATIC
    # Persistence
    "persistence/sqlite/db_manager.cpp"
    "persistence/repositories/sqlite_project_repository.cpp"
    
    # IO
    "io/processed_data_writer.cpp"
    "io/file_controller.cpp"
    "io/core/file_reader.cpp"
    "io/core/file_writer.cpp"
    "io/core/file_system_helper.cpp"
    "io/utils/file_utils.cpp"

    # Reports Infrastructure
    "reports/export_utils.cpp"
    "reports/exporter.cpp"
    "reports/report_file_manager.cpp"
    "reports/plugin_manifest.cpp"
    "reports/formatter_registry.cpp"
    "reports/report_service.cpp"
    "reports/services/daily_report_service.cpp"
    "reports/services/monthly_report_service.cpp"
    "reports/services/weekly_report_service.cpp"
    "reports/services/yearly_report_service.cpp"
    "reports/shared/factories/formatter_config_payload.cpp"

    # Serializer
    "serialization/json_serializer.cpp"
    "serialization/core/log_deserializer.cpp"
    "serialization/core/log_serializer.cpp"

    # Config
    "config/config_loader.cpp"
    "config/internal/config_parser_utils.cpp"
    "config/loader/report_config_loader.cpp"
    "config/loader/converter_config_loader.cpp"
    "config/loader/toml_converter_config_loader.cpp"
    "config/loader/toml_loader_utils.cpp"
    "config/validator/converter/rules/duration_rule.cpp"
    "config/validator/converter/rules/main_rule.cpp"
    "config/validator/converter/rules/mapping_rule.cpp"
    "config/validator/plugins/facade/plugin_validator.cpp"
    "config/validator/reports/strategies/base_strategy.cpp"
    "config/validator/reports/strategies/strategy_factory.cpp"
    "config/validator/reports/strategies/daily/daily_md.cpp"
    "config/validator/reports/strategies/daily/daily_tex.cpp"
    "config/validator/reports/strategies/daily/daily_typ.cpp"
    "config/validator/reports/strategies/monthly/monthly.cpp"
    "config/validator/reports/strategies/weekly/weekly.cpp"
    "config/validator/reports/strategies/yearly/yearly.cpp"

    # Bootstrap
    "../application/bootstrap/startup_validator.cpp"
)

setup_app_target(time_tracker_infrastructure)

target_link_libraries(time_tracker_infrastructure PUBLIC
    time_tracker_domain
    SQLite3
    tomlplusplus::tomlplusplus
    nlohmann_json::nlohmann_json
    reports_data
    reports_shared # Config validator needs style config from reports_shared? Yes.
)

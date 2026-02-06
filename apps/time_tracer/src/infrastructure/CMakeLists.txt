# src/infrastructure/CMakeLists.txt
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

    # Serializer (Moved logic here via .. reference or just include them if they verify as infra)
    "../serializer/json_serializer.cpp"
    "../serializer/core/log_deserializer.cpp"
    "../serializer/core/log_serializer.cpp"

    # Config (Loaders are infra)
    "../config/config_loader.cpp"
    "../config/internal/config_parser_utils.cpp"
    "../config/loader/report_config_loader.cpp"
    "../config/loader/converter_config_loader.cpp"
    "../config/loader/toml_converter_config_loader.cpp"
    "../config/loader/toml_loader_utils.cpp"
    "../config/validator/facade/config_facade.cpp"
    "../config/validator/converter/facade/converter_facade.cpp"
    "../config/validator/converter/rules/duration_rule.cpp"
    "../config/validator/converter/rules/main_rule.cpp"
    "../config/validator/converter/rules/mapping_rule.cpp"
    "../config/validator/plugins/facade/plugin_validator.cpp"
    "../config/validator/reports/facade/query_facade.cpp"
    "../config/validator/reports/strategies/base_strategy.cpp"
    "../config/validator/reports/strategies/strategy_factory.cpp"
    "../config/validator/reports/strategies/daily/daily_md.cpp"
    "../config/validator/reports/strategies/daily/daily_tex.cpp"
    "../config/validator/reports/strategies/daily/daily_typ.cpp"
    "../config/validator/reports/strategies/monthly/monthly.cpp"
    "../config/validator/reports/strategies/weekly/weekly.cpp"
    "../config/validator/reports/strategies/yearly/yearly.cpp"

    # Bootstrap
    "../bootstrap/startup_validator.cpp"
)

setup_project_target(time_tracker_infrastructure)

target_link_libraries(time_tracker_infrastructure PUBLIC
    time_tracker_domain
    SQLite3
    tomlplusplus::tomlplusplus
    nlohmann_json::nlohmann_json
    reports_shared # Config validator needs style config from reports_shared? Yes.
)

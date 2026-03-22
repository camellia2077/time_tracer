auto BuildMappingNamesContent(
    const std::optional<std::filesystem::path>& converter_config_toml_path)
    -> std::string;

auto ValidateReportChartRequest(
    const tracer_core::core::dto::DataQueryRequest& request) -> void;

auto BuildReportChartContent(
    sqlite3* db_conn, const tracer_core::core::dto::DataQueryRequest& request)
    -> std::string;

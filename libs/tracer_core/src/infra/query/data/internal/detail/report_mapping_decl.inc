auto BuildMappingNamesContent(
    const std::optional<std::filesystem::path>& converter_config_toml_path)
    -> std::string;

auto BuildMappingAliasKeysContent(
    const std::optional<std::filesystem::path>& converter_config_toml_path)
    -> std::string;

auto BuildWakeKeywordsContent(
    const std::optional<std::filesystem::path>& converter_config_toml_path)
    -> std::string;

auto BuildAuthorableEventTokensContent(
    const std::optional<std::filesystem::path>& converter_config_toml_path)
    -> std::string;

auto ValidateReportChartRequest(
    const tracer_core::core::dto::DataQueryRequest& request) -> void;

auto BuildReportChartContent(
    sqlite3* db_conn, const tracer_core::core::dto::DataQueryRequest& request)
    -> std::string;

auto ValidateReportCompositionRequest(
    const tracer_core::core::dto::DataQueryRequest& request) -> void;

auto BuildReportCompositionContent(
    sqlite3* db_conn, const tracer_core::core::dto::DataQueryRequest& request)
    -> std::string;

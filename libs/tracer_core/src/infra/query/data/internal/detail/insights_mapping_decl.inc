auto BuildMappingNamesContent(
    const std::optional<std::filesystem::path>& converter_config_toml_path)
    -> std::string;

auto BuildActivityHierarchyLeafMappingsContent(
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

auto ValidateInsightsChartRequest(
    const tracer_core::core::dto::DataQueryRequest& request) -> void;

auto BuildInsightsChartContent(
    sqlite3* db_conn, const tracer_core::core::dto::DataQueryRequest& request)
    -> std::string;

auto ValidateInsightsCompositionRequest(
    const tracer_core::core::dto::DataQueryRequest& request) -> void;

auto BuildInsightsCompositionContent(
    sqlite3* db_conn, const tracer_core::core::dto::DataQueryRequest& request)
    -> std::string;

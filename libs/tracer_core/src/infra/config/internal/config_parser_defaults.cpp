// infra/config/internal/config_parser_defaults.cpp
#include "infra/config/internal/config_parser_utils_internal.hpp"

namespace ConfigParserUtils::internal {

auto ParseGlobalDefaults(const toml::table& defaults_tbl,
                         const ConfigParseSource& source, AppConfig& config)
    -> void {
  constexpr std::string_view kSection = "defaults";
  if (const auto kValue = TryReadTypedField<std::string>(
          defaults_tbl, "db_path", source.source_path, kSection, "a string")) {
    config.defaults.kDbPath = ResolveDefaultPath(source.exe_path, *kValue);
  }
  if (const auto kValue = TryReadTypedField<std::string>(
          defaults_tbl, "output_root", source.source_path, kSection,
          "a string")) {
    config.defaults.output_root = ResolveDefaultPath(source.exe_path, *kValue);
  }
  if (const auto kValue = TryReadTypedField<std::string>(
          defaults_tbl, "default_format", source.source_path, kSection,
          "a string")) {
    config.defaults.default_format = *kValue;
  }
}

auto ParseExportDefaults(const toml::table& export_tbl,
                         const fs::path& source_path, AppConfig& config)
    -> void {
  constexpr std::string_view kSection = "commands.export";
  if (const auto kValue = TryReadTypedField<std::string>(
          export_tbl, "format", source_path, kSection, "a string")) {
    config.command_defaults.export_format = *kValue;
  }
}

auto ParseConvertDefaults(const toml::table& convert_tbl,
                          const fs::path& source_path, AppConfig& config)
    -> void {
  constexpr std::string_view kSection = "commands.convert";
  if (const auto kValue =
          TryReadTypedField<bool>(convert_tbl, "save_processed_output",
                                  source_path, kSection, "a boolean")) {
    config.command_defaults.convert_save_processed_output = kValue;
  }
  if (const auto kValue = TryReadTypedField<bool>(
          convert_tbl, "validate_logic", source_path, kSection, "a boolean")) {
    config.command_defaults.convert_validate_logic = kValue;
  }
  if (const auto kValue =
          TryReadTypedField<bool>(convert_tbl, "validate_structure",
                                  source_path, kSection, "a boolean")) {
    config.command_defaults.convert_validate_structure = kValue;
  }
}

auto ParseQueryDefaults(const toml::table& query_tbl,
                        const fs::path& source_path, AppConfig& config)
    -> void {
  constexpr std::string_view kSection = "commands.query";
  if (const auto kValue = TryReadTypedField<std::string>(
          query_tbl, "format", source_path, kSection, "a string")) {
    config.command_defaults.query_format = *kValue;
  }
}

auto ParseIngestDefaults(const toml::table& ingest_tbl,
                         const fs::path& source_path, AppConfig& config)
    -> void {
  constexpr std::string_view kSection = "commands.ingest";
  if (const auto kValue =
          TryReadTypedField<bool>(ingest_tbl, "save_processed_output",
                                  source_path, kSection, "a boolean")) {
    config.command_defaults.ingest_save_processed_output = kValue;
  }
}

// NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
void ParseSystemSettingsImpl(const toml::table& tbl, const fs::path& exe_path,
                             const fs::path& source_config_path,
                             AppConfig& config) {
  auto fill_from_section = [&](const toml::table& section,
                               std::string_view section_key) -> void {
    if (const auto kValue = TryReadTypedField<std::string>(
            section, "error_log", source_config_path, section_key,
            "a string")) {
      config.error_log_path = exe_path / *kValue;
    } else {
      config.error_log_path = exe_path / "error.log";
    }

    if (const auto kValue = TryReadTypedField<std::string>(
            section, "export_root", source_config_path, section_key,
            "a string")) {
      config.kExportPath = ResolveDefaultPath(exe_path, *kValue);
    }

    config.default_save_processed_output =
        TryReadTypedField<bool>(section, "save_processed_output",
                                source_config_path, section_key, "a boolean")
            .value_or(false);
  };

  if (const toml::table* system_tbl =
          TryReadTableField(tbl, "system", source_config_path, "")) {
    fill_from_section(*system_tbl, "system");
    return;
  }

  config.error_log_path = exe_path / "error.log";
}

void ParseCliDefaultsImpl(const toml::table& tbl, const fs::path& exe_path,
                          const fs::path& source_config_path,
                          AppConfig& config) {
  const ConfigParseSource kParseSource{
      .exe_path = exe_path,
      .source_path = source_config_path,
  };
  if (const toml::table* defaults_tbl =
          TryReadTableField(tbl, "defaults", source_config_path, "")) {
    ParseGlobalDefaults(*defaults_tbl, kParseSource, config);
  }

  const toml::table* commands_tbl =
      TryReadTableField(tbl, "commands", source_config_path, "");
  if (commands_tbl == nullptr) {
    return;
  }

  if (const toml::table* sub_tbl = TryReadTableField(
          *commands_tbl, "export", source_config_path, "commands")) {
    ParseExportDefaults(*sub_tbl, source_config_path, config);
  }

  if (const toml::table* sub_tbl = TryReadTableField(
          *commands_tbl, "convert", source_config_path, "commands")) {
    ParseConvertDefaults(*sub_tbl, source_config_path, config);
  }

  if (const toml::table* sub_tbl = TryReadTableField(
          *commands_tbl, "query", source_config_path, "commands")) {
    ParseQueryDefaults(*sub_tbl, source_config_path, config);
  }

  if (const toml::table* sub_tbl = TryReadTableField(
          *commands_tbl, "ingest", source_config_path, "commands")) {
    ParseIngestDefaults(*sub_tbl, source_config_path, config);
  }
}

auto ParseRuntimeConfigPaths(const toml::table& config_tbl,
                             const fs::path& config_dir,
                             const fs::path& source_path, AppConfig& config)
    -> void {
  const toml::table* converter_tbl =
      TryReadTableField(config_tbl, "converter", source_path, "");
  if (converter_tbl == nullptr) {
    ThrowConfigFieldError(source_path, "converter",
                          "is required and must be a table.");
  }
  const std::string kMainConfig = RequireNonEmptyStringField(
      *converter_tbl, "main_config", source_path, "converter");
  config.pipeline.converter_main_config_path =
      NormalizeConfigRelativePath(config_dir, kMainConfig);
  EnsureFileExists(source_path, "converter.main_config",
                   config.pipeline.converter_main_config_path);

  const toml::table* visualization_tbl =
      TryReadTableField(config_tbl, "visualization", source_path, "");
  if (visualization_tbl == nullptr) {
    ThrowConfigFieldError(source_path, "visualization",
                          "is required and must be a table.");
  }
  const std::string kHeatmap = RequireNonEmptyStringField(
      *visualization_tbl, "heatmap", source_path, "visualization");
  const fs::path kHeatmapPath =
      NormalizeConfigRelativePath(config_dir, kHeatmap);
  EnsureFileExists(source_path, "visualization.heatmap", kHeatmapPath);

  const toml::table* insights_tbl =
      TryReadTableField(config_tbl, "insights", source_path, "");
  if (insights_tbl == nullptr) {
    ThrowConfigFieldError(source_path, "insights",
                          "is required and must be a table.");
  }
  const InsightsPathSource kInsightsSource{
      .config_dir = config_dir,
      .source_path = source_path,
  };
  bool has_any_insights_format = false;
  if (const toml::table* typst_tbl =
          TryReadTableField(*insights_tbl, "typst", source_path, "insights")) {
    has_any_insights_format = true;
    LoadInsightsPathsFromTable(*typst_tbl, kInsightsSource, "insights.typst",
                               config.insights.daily_typ_config_path,
                               config.insights.month_typ_config_path,
                               config.insights.period_typ_config_path,
                               config.insights.week_typ_config_path,
                               config.insights.year_typ_config_path);
  }
  if (const toml::table* latex_tbl =
          TryReadTableField(*insights_tbl, "latex", source_path, "insights")) {
    has_any_insights_format = true;
    LoadInsightsPathsFromTable(*latex_tbl, kInsightsSource, "insights.latex",
                               config.insights.daily_tex_config_path,
                               config.insights.month_tex_config_path,
                               config.insights.period_tex_config_path,
                               config.insights.week_tex_config_path,
                               config.insights.year_tex_config_path);
  }
  if (const toml::table* markdown_tbl = TryReadTableField(
          *insights_tbl, "markdown", source_path, "insights")) {
    has_any_insights_format = true;
    LoadInsightsPathsFromTable(*markdown_tbl, kInsightsSource,
                               "insights.markdown",
                               config.insights.daily_md_config_path,
                               config.insights.month_md_config_path,
                               config.insights.period_md_config_path,
                               config.insights.week_md_config_path,
                               config.insights.year_md_config_path);
  }
  if (!has_any_insights_format) {
    ThrowConfigFieldError(source_path, "insights",
                          "must contain at least one insights format table.");
  }
}

}  // namespace ConfigParserUtils::internal

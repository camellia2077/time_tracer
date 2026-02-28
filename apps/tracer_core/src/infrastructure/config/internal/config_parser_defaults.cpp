// infrastructure/config/internal/config_parser_defaults.cpp
#include "infrastructure/config/internal/config_parser_utils_internal.hpp"

namespace ConfigParserUtils::internal {

auto ParseDateCheckMode(std::string_view mode_str, const fs::path& source_path,
                        const std::string& field_path) -> DateCheckMode {
  if (mode_str == "none") {
    return DateCheckMode::kNone;
  }
  if (mode_str == "continuity") {
    return DateCheckMode::kContinuity;
  }
  if (mode_str == "full") {
    return DateCheckMode::kFull;
  }
  ThrowConfigFieldError(source_path, field_path,
                        "has unsupported value '" + std::string(mode_str) +
                            "'. Supported values: none, continuity, full.");
}

auto ParseGlobalDefaults(const toml::table& defaults_tbl,
                         const ConfigParseSource& source, AppConfig& config)
    -> void {
  constexpr std::string_view kSection = "defaults";
  if (const auto value = TryReadTypedField<std::string>(
          defaults_tbl, "db_path", source.source_path, kSection, "a string")) {
    config.defaults.kDbPath = ResolveDefaultPath(source.exe_path, *value);
  }
  if (const auto value = TryReadTypedField<std::string>(
          defaults_tbl, "output_root", source.source_path, kSection,
          "a string")) {
    config.defaults.output_root = ResolveDefaultPath(source.exe_path, *value);
  }
  if (const auto value = TryReadTypedField<std::string>(
          defaults_tbl, "default_format", source.source_path, kSection,
          "a string")) {
    config.defaults.default_format = *value;
  }
}

auto ParseExportDefaults(const toml::table& export_tbl,
                         const fs::path& source_path, AppConfig& config)
    -> void {
  constexpr std::string_view kSection = "commands.export";
  if (const auto value = TryReadTypedField<std::string>(
          export_tbl, "format", source_path, kSection, "a string")) {
    config.command_defaults.export_format = *value;
  }
}

auto ParseConvertDefaults(const toml::table& convert_tbl,
                          const fs::path& source_path, AppConfig& config)
    -> void {
  constexpr std::string_view kSection = "commands.convert";
  if (const auto value = TryReadTypedField<std::string>(
          convert_tbl, "date_check", source_path, kSection, "a string")) {
    config.command_defaults.convert_date_check_mode = ParseDateCheckMode(
        *value, source_path, JoinFieldPath(kSection, "date_check"));
  }
  if (const auto value =
          TryReadTypedField<bool>(convert_tbl, "save_processed_output",
                                  source_path, kSection, "a boolean")) {
    config.command_defaults.convert_save_processed_output = value;
  }
  if (const auto value = TryReadTypedField<bool>(
          convert_tbl, "validate_logic", source_path, kSection, "a boolean")) {
    config.command_defaults.convert_validate_logic = value;
  }
  if (const auto value =
          TryReadTypedField<bool>(convert_tbl, "validate_structure",
                                  source_path, kSection, "a boolean")) {
    config.command_defaults.convert_validate_structure = value;
  }
}

auto ParseQueryDefaults(const toml::table& query_tbl,
                        const fs::path& source_path, AppConfig& config)
    -> void {
  constexpr std::string_view kSection = "commands.query";
  if (const auto value = TryReadTypedField<std::string>(
          query_tbl, "format", source_path, kSection, "a string")) {
    config.command_defaults.query_format = *value;
  }
}

auto ParseIngestDefaults(const toml::table& ingest_tbl,
                         const fs::path& source_path, AppConfig& config)
    -> void {
  constexpr std::string_view kSection = "commands.ingest";
  if (const auto value = TryReadTypedField<std::string>(
          ingest_tbl, "date_check", source_path, kSection, "a string")) {
    config.command_defaults.ingest_date_check_mode = ParseDateCheckMode(
        *value, source_path, JoinFieldPath(kSection, "date_check"));
  }
  if (const auto value =
          TryReadTypedField<bool>(ingest_tbl, "save_processed_output",
                                  source_path, kSection, "a boolean")) {
    config.command_defaults.ingest_save_processed_output = value;
  }
}

auto ParseValidateLogicDefaults(const toml::table& validate_logic_tbl,
                                const fs::path& source_path, AppConfig& config)
    -> void {
  constexpr std::string_view kSection = "commands.validate-logic";
  if (const auto value =
          TryReadTypedField<std::string>(validate_logic_tbl, "date_check",
                                         source_path, kSection, "a string")) {
    config.command_defaults.validate_logic_date_check_mode = ParseDateCheckMode(
        *value, source_path, JoinFieldPath(kSection, "date_check"));
  }
}

void ParseSystemSettingsImpl(const toml::table& tbl, const fs::path& exe_path,
                             const fs::path& source_config_path,
                             AppConfig& config) {
  auto fill_from_section = [&](const toml::table& section,
                               std::string_view section_key) -> void {
    if (const auto value = TryReadTypedField<std::string>(
            section, "error_log", source_config_path, section_key,
            "a string")) {
      config.error_log_path = exe_path / *value;
    } else {
      config.error_log_path = exe_path / "error.log";
    }

    if (const auto value = TryReadTypedField<std::string>(
            section, "export_root", source_config_path, section_key,
            "a string")) {
      config.kExportPath = ResolveDefaultPath(exe_path, *value);
    }

    config.default_save_processed_output =
        TryReadTypedField<bool>(section, "save_processed_output",
                                source_config_path, section_key, "a boolean")
            .value_or(false);

    const bool check =
        TryReadTypedField<bool>(section, "date_check_continuity",
                                source_config_path, section_key, "a boolean")
            .value_or(false);
    config.default_date_check_mode =
        check ? DateCheckMode::kContinuity : DateCheckMode::kNone;
  };

  if (const toml::table* system_tbl =
          TryReadTableField(tbl, "system", source_config_path, "")) {
    fill_from_section(*system_tbl, "system");
    return;
  }

  if (const toml::table* general_tbl =
          TryReadTableField(tbl, "general", source_config_path, "")) {
    fill_from_section(*general_tbl, "general");
    return;
  }

  config.error_log_path = exe_path / "error.log";
}

void ParseCliDefaultsImpl(const toml::table& tbl, const fs::path& exe_path,
                          const fs::path& source_config_path,
                          AppConfig& config) {
  const ConfigParseSource parse_source{
      .exe_path = exe_path,
      .source_path = source_config_path,
  };
  if (const toml::table* defaults_tbl =
          TryReadTableField(tbl, "defaults", source_config_path, "")) {
    ParseGlobalDefaults(*defaults_tbl, parse_source, config);
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

  if (const toml::table* sub_tbl = TryReadTableField(
          *commands_tbl, "validate-logic", source_config_path, "commands")) {
    ParseValidateLogicDefaults(*sub_tbl, source_config_path, config);
  }
}

}  // namespace ConfigParserUtils::internal

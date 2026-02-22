// infrastructure/config/internal/config_parser_utils_internal.hpp
#ifndef INFRASTRUCTURE_CONFIG_INTERNAL_CONFIG_PARSER_UTILS_INTERNAL_H_
#define INFRASTRUCTURE_CONFIG_INTERNAL_CONFIG_PARSER_UTILS_INTERNAL_H_

#include <toml++/toml.h>

#include <filesystem>
#include <optional>
#include <string>
#include <string_view>

#include "infrastructure/config/models/app_config.hpp"

namespace ConfigParserUtils::internal {

namespace fs = std::filesystem;

struct ReportPathSource {
  const fs::path& config_dir;
  const fs::path& source_path;
};

struct ConfigParseSource {
  const fs::path& exe_path;
  const fs::path& source_path;
};

struct BundlePathSource {
  const fs::path& bundle_path;
  const fs::path& config_dir;
};

auto ResolveDefaultPath(const fs::path& exe_path, const std::string& path_value)
    -> fs::path;

auto JoinFieldPath(std::string_view prefix, std::string_view key)
    -> std::string;

[[noreturn]] auto ThrowConfigFieldError(const fs::path& source_path,
                                        const std::string& field_path,
                                        const std::string& message) -> void;

template <typename T>
auto TryReadTypedField(const toml::table& tbl, std::string_view key,
                       const fs::path& source_path,
                       std::string_view field_prefix,
                       std::string_view expected_type) -> std::optional<T> {
  const toml::node* node = tbl.get(key);
  if (node == nullptr) {
    return std::nullopt;
  }

  const auto value = node->value<T>();
  if (!value.has_value()) {
    ThrowConfigFieldError(source_path, JoinFieldPath(field_prefix, key),
                          "must be " + std::string(expected_type) + ".");
  }
  return value;
}

template <typename T>
auto RequireTypedField(const toml::table& tbl, std::string_view key,
                       const fs::path& source_path,
                       std::string_view field_prefix,
                       std::string_view expected_type) -> T {
  if (const auto value = TryReadTypedField<T>(tbl, key, source_path,
                                              field_prefix, expected_type)) {
    return *value;
  }
  ThrowConfigFieldError(
      source_path, JoinFieldPath(field_prefix, key),
      "is required and must be " + std::string(expected_type) + ".");
}

auto TryReadTableField(const toml::table& tbl, std::string_view key,
                       const fs::path& source_path,
                       std::string_view field_prefix) -> const toml::table*;

auto EnsureFileExists(const fs::path& source_path,
                      const std::string& field_path, const fs::path& file_path)
    -> void;

auto NormalizeConfigRelativePath(const fs::path& config_dir,
                                 std::string raw_path) -> fs::path;

auto RequireNonEmptyStringField(const toml::table& tbl, std::string_view key,
                                const fs::path& source_path,
                                std::string_view field_prefix) -> std::string;

auto EnsureFieldAbsent(const toml::table& tbl, std::string_view key,
                       const fs::path& source_path,
                       std::string_view field_prefix,
                       std::string_view replacement_hint) -> void;

auto ParseDateCheckMode(std::string_view mode_str, const fs::path& source_path,
                        const std::string& field_path) -> DateCheckMode;

auto LoadReportPathsFromTable(const toml::table& section,
                              const ReportPathSource& source,
                              std::string_view section_field_path,
                              fs::path& day_path, fs::path& month_path,
                              fs::path& period_path, fs::path& week_path,
                              fs::path& year_path) -> void;

auto ParseGlobalDefaults(const toml::table& defaults_tbl,
                         const ConfigParseSource& source, AppConfig& config)
    -> void;

auto ParseExportDefaults(const toml::table& export_tbl,
                         const fs::path& source_path, AppConfig& config)
    -> void;

auto ParseConvertDefaults(const toml::table& convert_tbl,
                          const fs::path& source_path, AppConfig& config)
    -> void;

auto ParseQueryDefaults(const toml::table& query_tbl,
                        const fs::path& source_path, AppConfig& config) -> void;

auto ParseIngestDefaults(const toml::table& ingest_tbl,
                         const fs::path& source_path, AppConfig& config)
    -> void;

auto ParseValidateLogicDefaults(const toml::table& validate_logic_tbl,
                                const fs::path& source_path, AppConfig& config)
    -> void;

auto ValidateBundleFileList(const toml::table& bundle_tbl,
                            const BundlePathSource& source) -> void;

void ParseSystemSettingsImpl(const toml::table& tbl, const fs::path& exe_path,
                             const fs::path& source_config_path,
                             AppConfig& config);

void ParseCliDefaultsImpl(const toml::table& tbl, const fs::path& exe_path,
                          const fs::path& source_config_path,
                          AppConfig& config);

auto ResolveBundlePathImpl(const fs::path& config_dir) -> fs::path;

auto TryParseBundlePathsImpl(const fs::path& config_dir, AppConfig& config)
    -> bool;

}  // namespace ConfigParserUtils::internal

#endif  // INFRASTRUCTURE_CONFIG_INTERNAL_CONFIG_PARSER_UTILS_INTERNAL_H_

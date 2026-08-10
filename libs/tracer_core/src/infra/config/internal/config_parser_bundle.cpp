// infra/config/internal/config_parser_bundle.cpp
#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <vector>

#include "infra/config/internal/config_parser_utils_internal.hpp"
#include "infra/internal/canonical_file_io.hpp"

namespace ConfigParserUtils::internal {

#include "infra/config/internal/bundle/config_parser_bundle_namespace.inc"

namespace infra_file_io = tracer::core::infrastructure::internal::file_io;

auto LoadInsightsPathsFromTable(const toml::table& section,
                              const InsightsPathSource& source,
                              std::string_view section_field_path,
                              fs::path& day_path, fs::path& month_path,
                              fs::path& period_path, fs::path& week_path,
                              fs::path& year_path) -> void {
  const std::string kRoot = RequireNonEmptyStringField(
      section, "root", source.source_path, section_field_path);
  const fs::path kRootPath =
      NormalizeConfigRelativePath(source.config_dir, kRoot);
  if (!fs::is_directory(kRootPath)) {
    ThrowConfigFieldError(source.source_path,
                          JoinFieldPath(section_field_path, "root"),
                          "must point to an existing directory.");
  }

  fs::path insights_root = kRootPath;
  if (section_field_path == "insights.markdown") {
    const std::string kLocale = RequireNonEmptyStringField(
        section, "default_locale", source.source_path, section_field_path);
    insights_root /= kLocale;
  }

  const auto kLoad = [&](std::string_view key, fs::path& output) {
    output = insights_root / (std::string(key) + ".toml");
    EnsureFileExists(source.source_path, JoinFieldPath(section_field_path, key),
                     output);
  };
  kLoad("day", day_path);
  kLoad("month", month_path);
  kLoad("period", period_path);
  kLoad("week", week_path);
  kLoad("year", year_path);
}

auto ValidateBundleFileList(const toml::table& bundle_tbl,
                            const BundlePathSource& source) -> void {
  const toml::table* file_list_tbl =
      TryReadTableField(bundle_tbl, "file_list", source.bundle_path, "");
  if (file_list_tbl == nullptr) {
    ThrowConfigFieldError(source.bundle_path, "file_list",
                          "is required and must be a table.");
  }

  const toml::node* required_node = file_list_tbl->get("required");
  if (required_node == nullptr) {
    ThrowConfigFieldError(source.bundle_path, "file_list.required",
                          "is required and must be an array.");
  }
  const toml::array* required_array = required_node->as_array();
  if (required_array == nullptr) {
    ThrowConfigFieldError(source.bundle_path, "file_list.required",
                          "must be an array.");
  }

  for (size_t index = 0; index < required_array->size(); ++index) {
    const std::string kFieldPath =
        "file_list.required[" + std::to_string(index) + "]";
    const toml::node* item = required_array->get(index);
    if (item == nullptr || !item->is_string()) {
      ThrowConfigFieldError(source.bundle_path, kFieldPath,
                            "must be a non-empty string.");
    }
    const auto kRelativePath = item->value<std::string>();
    if (!kRelativePath.has_value() || kRelativePath->empty()) {
      ThrowConfigFieldError(source.bundle_path, kFieldPath,
                            "must be a non-empty string.");
    }
    const fs::path kAbsolutePath =
        NormalizeConfigRelativePath(source.config_dir, *kRelativePath);
    EnsureFileExists(source.bundle_path, kFieldPath, kAbsolutePath);
  }

  const toml::node* optional_node = file_list_tbl->get("optional");
  if (optional_node != nullptr && optional_node->as_array() == nullptr) {
    ThrowConfigFieldError(source.bundle_path, "file_list.optional",
                          "must be an array when present.");
  }
}

auto LoadAndroidInsightsPathSetFromTable(const toml::table& section,
                                       const InsightsPathSource& source,
                                       std::string_view section_field_path)
    -> AndroidBundleInsightsConfigPathSet {
  AndroidBundleInsightsConfigPathSet out{};
  LoadInsightsPathsFromTable(section, source, section_field_path, out.day,
                           out.month, out.period, out.week, out.year);
  return out;
}

auto TryResolveAndroidBundleConfigPathsImpl(const fs::path& config_dir)
    -> std::optional<AndroidBundleConfigPaths> {
  const fs::path kBundlePath = ResolveBundlePathImpl(config_dir);
  if (!fs::exists(kBundlePath)) {
    return std::nullopt;
  }

  toml::table bundle_tbl;
  try {
    bundle_tbl = toml::parse(infra_file_io::ReadCanonicalText(kBundlePath));
  } catch (const toml::parse_error& err) {
    throw std::runtime_error("Failed to parse bundle TOML [" +
                             kBundlePath.string() +
                             "]: " + std::string(err.description()));
  } catch (const std::exception& err) {
    throw std::runtime_error("Failed to load bundle TOML [" +
                             kBundlePath.string() +
                             "]: " + std::string(err.what()));
  }

  static_cast<void>(RequireTypedField<int64_t>(bundle_tbl, "schema_version",
                                               kBundlePath, "", "an integer"));
  const std::string kProfile =
      RequireNonEmptyStringField(bundle_tbl, "profile", kBundlePath, "");

  const BundlePathSource kBundleSource{
      .bundle_path = kBundlePath,
      .config_dir = config_dir,
  };
  ValidateBundleFileList(bundle_tbl, kBundleSource);
  const fs::path kConfigPath = config_dir / "program" / "config.toml";
  if (!fs::exists(kConfigPath)) {
    ThrowConfigFieldError(kBundlePath, "file_list.required",
                          "must include config.toml.");
  }
  toml::table config_tbl;
  try {
    config_tbl = toml::parse(infra_file_io::ReadCanonicalText(kConfigPath));
  } catch (const toml::parse_error& err) {
    throw std::runtime_error("Failed to parse config.toml [" +
                             kConfigPath.string() +
                             "]: " + std::string(err.description()));
  }

  AppConfig parsed_config;
  ParseRuntimeConfigPaths(config_tbl, kConfigPath.parent_path(), kConfigPath,
                          parsed_config);

  AndroidBundleConfigPaths out{};
  out.converter_config_toml_path =
      parsed_config.pipeline.converter_main_config_path;
  out.markdown = {
      .day = parsed_config.insights.day_md_config_path,
      .month = parsed_config.insights.month_md_config_path,
      .period = parsed_config.insights.period_md_config_path,
      .week = parsed_config.insights.week_md_config_path,
      .year = parsed_config.insights.year_md_config_path,
  };
  if (!parsed_config.insights.day_typ_config_path.empty()) {
    out.typst = AndroidBundleInsightsConfigPathSet{
        .day = parsed_config.insights.day_typ_config_path,
        .month = parsed_config.insights.month_typ_config_path,
        .period = parsed_config.insights.period_typ_config_path,
        .week = parsed_config.insights.week_typ_config_path,
        .year = parsed_config.insights.year_typ_config_path,
    };
  }
  if (!parsed_config.insights.day_tex_config_path.empty()) {
    out.latex = AndroidBundleInsightsConfigPathSet{
        .day = parsed_config.insights.day_tex_config_path,
        .month = parsed_config.insights.month_tex_config_path,
        .period = parsed_config.insights.period_tex_config_path,
        .week = parsed_config.insights.week_tex_config_path,
        .year = parsed_config.insights.year_tex_config_path,
    };
  }
  if (kProfile == "android" &&
      (out.latex.has_value() || out.typst.has_value())) {
    ThrowConfigFieldError(
        kConfigPath, "insights",
        "must not contain LaTeX or Typst for profile 'android'.");
  }
  return out;
}

auto TryParseBundlePathsImpl(const fs::path& config_dir, AppConfig& config)
    -> bool {
  const fs::path kBundlePath = ResolveBundlePathImpl(config_dir);
  if (!fs::exists(kBundlePath)) {
    return false;
  }

  toml::table bundle_tbl;
  try {
    bundle_tbl = toml::parse(infra_file_io::ReadCanonicalText(kBundlePath));
  } catch (const toml::parse_error& err) {
    throw std::runtime_error("Failed to parse bundle TOML [" +
                             kBundlePath.string() +
                             "]: " + std::string(err.description()));
  } catch (const std::exception& err) {
    throw std::runtime_error("Failed to load bundle TOML [" +
                             kBundlePath.string() +
                             "]: " + std::string(err.what()));
  }

  static_cast<void>(RequireTypedField<int64_t>(bundle_tbl, "schema_version",
                                               kBundlePath, "", "an integer"));
  static_cast<void>(RequireTypedField<std::string>(
      bundle_tbl, "profile", kBundlePath, "", "a string"));
  const BundlePathSource kBundleSource{
      .bundle_path = kBundlePath,
      .config_dir = config_dir,
  };
  ValidateBundleFileList(bundle_tbl, kBundleSource);

  return true;
}

}  // namespace ConfigParserUtils::internal

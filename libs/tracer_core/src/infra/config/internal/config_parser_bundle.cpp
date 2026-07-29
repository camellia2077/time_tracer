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

auto LoadReportPathsFromTable(const toml::table& section,
                              const ReportPathSource& source,
                              std::string_view section_field_path,
                              fs::path& day_path, fs::path& month_path,
                              fs::path& period_path, fs::path& week_path,
                              fs::path& year_path) -> void {
  const std::string root = RequireNonEmptyStringField(
      section, "root", source.source_path, section_field_path);
  const fs::path root_path =
      NormalizeConfigRelativePath(source.config_dir, root);
  if (!fs::is_directory(root_path)) {
    ThrowConfigFieldError(source.source_path,
                          JoinFieldPath(section_field_path, "root"),
                          "must point to an existing directory.");
  }

  fs::path report_root = root_path;
  if (section_field_path == "reports.markdown") {
    const std::string locale = RequireNonEmptyStringField(
        section, "default_locale", source.source_path, section_field_path);
    report_root /= locale;
  }

  const auto load = [&](std::string_view key, fs::path& output) {
    output = report_root / (std::string(key) + ".toml");
    EnsureFileExists(source.source_path,
                     JoinFieldPath(section_field_path, key), output);
  };
  load("day", day_path);
  load("month", month_path);
  load("period", period_path);
  load("week", week_path);
  load("year", year_path);
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

auto LoadAndroidReportPathSetFromTable(const toml::table& section,
                                       const ReportPathSource& source,
                                       std::string_view section_field_path)
    -> AndroidBundleReportConfigPathSet {
  AndroidBundleReportConfigPathSet out{};
  LoadReportPathsFromTable(section, source, section_field_path, out.day,
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
  const fs::path config_path = config_dir / "config.toml";
  if (!fs::exists(config_path)) {
    ThrowConfigFieldError(kBundlePath, "file_list.required",
                          "must include config.toml.");
  }
  toml::table config_tbl;
  try {
    config_tbl = toml::parse(infra_file_io::ReadCanonicalText(config_path));
  } catch (const toml::parse_error& err) {
    throw std::runtime_error("Failed to parse config.toml [" +
                             config_path.string() + "]: " +
                             std::string(err.description()));
  }

  AppConfig parsed_config;
  ParseRuntimeConfigPaths(config_tbl, config_dir, config_path, parsed_config);

  AndroidBundleConfigPaths out{};
  out.converter_config_toml_path =
      parsed_config.pipeline.converter_main_config_path;
  out.markdown = {
      .day = parsed_config.reports.day_md_config_path,
      .month = parsed_config.reports.month_md_config_path,
      .period = parsed_config.reports.period_md_config_path,
      .week = parsed_config.reports.week_md_config_path,
      .year = parsed_config.reports.year_md_config_path,
  };
  if (!parsed_config.reports.day_typ_config_path.empty()) {
    out.typst = AndroidBundleReportConfigPathSet{
        .day = parsed_config.reports.day_typ_config_path,
        .month = parsed_config.reports.month_typ_config_path,
        .period = parsed_config.reports.period_typ_config_path,
        .week = parsed_config.reports.week_typ_config_path,
        .year = parsed_config.reports.year_typ_config_path,
    };
  }
  if (!parsed_config.reports.day_tex_config_path.empty()) {
    out.latex = AndroidBundleReportConfigPathSet{
        .day = parsed_config.reports.day_tex_config_path,
        .month = parsed_config.reports.month_tex_config_path,
        .period = parsed_config.reports.period_tex_config_path,
        .week = parsed_config.reports.week_tex_config_path,
        .year = parsed_config.reports.year_tex_config_path,
    };
  }
  if (kProfile == "android" && (out.latex.has_value() || out.typst.has_value())) {
    ThrowConfigFieldError(config_path, "reports",
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

// infrastructure/config/internal/config_parser_bundle.cpp
#include <cstdint>
#include <stdexcept>

#include "infrastructure/config/internal/config_parser_utils_internal.hpp"

namespace ConfigParserUtils::internal {

auto LoadReportPathsFromTable(const toml::table& section,
                              const ReportPathSource& source,
                              std::string_view section_field_path,
                              fs::path& day_path, fs::path& month_path,
                              fs::path& period_path, fs::path& week_path,
                              fs::path& year_path) -> void {
  EnsureFieldAbsent(section, "range", source.source_path, section_field_path,
                    "period");

  const std::string day = RequireNonEmptyStringField(
      section, "day", source.source_path, section_field_path);
  day_path = NormalizeConfigRelativePath(source.config_dir, day);
  EnsureFileExists(source.source_path, JoinFieldPath(section_field_path, "day"),
                   day_path);

  const std::string month = RequireNonEmptyStringField(
      section, "month", source.source_path, section_field_path);
  month_path = NormalizeConfigRelativePath(source.config_dir, month);
  EnsureFileExists(source.source_path,
                   JoinFieldPath(section_field_path, "month"), month_path);

  const std::string period = RequireNonEmptyStringField(
      section, "period", source.source_path, section_field_path);
  period_path = NormalizeConfigRelativePath(source.config_dir, period);
  EnsureFileExists(source.source_path,
                   JoinFieldPath(section_field_path, "period"), period_path);

  const std::string week = RequireNonEmptyStringField(
      section, "week", source.source_path, section_field_path);
  week_path = NormalizeConfigRelativePath(source.config_dir, week);
  EnsureFileExists(source.source_path,
                   JoinFieldPath(section_field_path, "week"), week_path);

  const std::string year = RequireNonEmptyStringField(
      section, "year", source.source_path, section_field_path);
  year_path = NormalizeConfigRelativePath(source.config_dir, year);
  EnsureFileExists(source.source_path,
                   JoinFieldPath(section_field_path, "year"), year_path);
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
    const std::string field_path =
        "file_list.required[" + std::to_string(index) + "]";
    const toml::node* item = required_array->get(index);
    if (item == nullptr || !item->is_string()) {
      ThrowConfigFieldError(source.bundle_path, field_path,
                            "must be a non-empty string.");
    }
    const auto relative_path = item->value<std::string>();
    if (!relative_path.has_value() || relative_path->empty()) {
      ThrowConfigFieldError(source.bundle_path, field_path,
                            "must be a non-empty string.");
    }
    const fs::path absolute_path =
        NormalizeConfigRelativePath(source.config_dir, *relative_path);
    EnsureFileExists(source.bundle_path, field_path, absolute_path);
  }

  const toml::node* optional_node = file_list_tbl->get("optional");
  if (optional_node != nullptr && optional_node->as_array() == nullptr) {
    ThrowConfigFieldError(source.bundle_path, "file_list.optional",
                          "must be an array when present.");
  }
}

auto TryParseBundlePathsImpl(const fs::path& config_dir, AppConfig& config)
    -> bool {
  const fs::path bundle_path = ResolveBundlePathImpl(config_dir);
  if (!fs::exists(bundle_path)) {
    return false;
  }

  toml::table bundle_tbl;
  try {
    bundle_tbl = toml::parse_file(bundle_path.string());
  } catch (const toml::parse_error& err) {
    throw std::runtime_error("Failed to parse bundle TOML [" +
                             bundle_path.string() +
                             "]: " + std::string(err.description()));
  }

  static_cast<void>(RequireTypedField<int64_t>(bundle_tbl, "schema_version",
                                               bundle_path, "", "an integer"));
  static_cast<void>(RequireTypedField<std::string>(
      bundle_tbl, "profile", bundle_path, "", "a string"));
  const BundlePathSource bundle_source{
      .bundle_path = bundle_path,
      .config_dir = config_dir,
  };
  ValidateBundleFileList(bundle_tbl, bundle_source);

  const toml::table* paths_tbl =
      TryReadTableField(bundle_tbl, "paths", bundle_path, "");
  if (paths_tbl == nullptr) {
    ThrowConfigFieldError(bundle_path, "paths",
                          "is required and must be a table.");
  }

  const toml::table* converter_tbl =
      TryReadTableField(*paths_tbl, "converter", bundle_path, "paths");
  if (converter_tbl == nullptr) {
    ThrowConfigFieldError(bundle_path, "paths.converter",
                          "is required and must be a table.");
  }

  EnsureFieldAbsent(*converter_tbl, "interval_processor_config", bundle_path,
                    "paths.converter", "paths.converter.interval_config");
  EnsureFieldAbsent(*converter_tbl, "interval_processor_config_path",
                    bundle_path, "paths.converter",
                    "paths.converter.interval_config");
  const std::string interval_config = RequireNonEmptyStringField(
      *converter_tbl, "interval_config", bundle_path, "paths.converter");
  config.pipeline.interval_processor_config_path =
      NormalizeConfigRelativePath(config_dir, interval_config);
  EnsureFileExists(bundle_path, "paths.converter.interval_config",
                   config.pipeline.interval_processor_config_path);

  const toml::table* reports_tbl =
      TryReadTableField(*paths_tbl, "reports", bundle_path, "paths");
  if (reports_tbl == nullptr) {
    ThrowConfigFieldError(bundle_path, "paths.reports",
                          "is required and must be a table.");
  }

  EnsureFieldAbsent(*reports_tbl, "md", bundle_path, "paths.reports",
                    "paths.reports.markdown");
  EnsureFieldAbsent(*reports_tbl, "tex", bundle_path, "paths.reports",
                    "paths.reports.latex");
  EnsureFieldAbsent(*reports_tbl, "typ", bundle_path, "paths.reports",
                    "paths.reports.typst");

  const ReportPathSource report_path_source{
      .config_dir = config_dir,
      .source_path = bundle_path,
  };

  bool has_any_report_format = false;

  if (const toml::table* typst_tbl = TryReadTableField(
          *reports_tbl, "typst", bundle_path, "paths.reports")) {
    has_any_report_format = true;
    LoadReportPathsFromTable(*typst_tbl, report_path_source,
                             "paths.reports.typst",
                             config.reports.day_typ_config_path,
                             config.reports.month_typ_config_path,
                             config.reports.period_typ_config_path,
                             config.reports.week_typ_config_path,
                             config.reports.year_typ_config_path);
  }

  if (const toml::table* latex_tbl = TryReadTableField(
          *reports_tbl, "latex", bundle_path, "paths.reports")) {
    has_any_report_format = true;
    LoadReportPathsFromTable(*latex_tbl, report_path_source,
                             "paths.reports.latex",
                             config.reports.day_tex_config_path,
                             config.reports.month_tex_config_path,
                             config.reports.period_tex_config_path,
                             config.reports.week_tex_config_path,
                             config.reports.year_tex_config_path);
  }

  if (const toml::table* markdown_tbl = TryReadTableField(
          *reports_tbl, "markdown", bundle_path, "paths.reports")) {
    has_any_report_format = true;
    LoadReportPathsFromTable(
        *markdown_tbl, report_path_source, "paths.reports.markdown",
        config.reports.day_md_config_path, config.reports.month_md_config_path,
        config.reports.period_md_config_path,
        config.reports.week_md_config_path, config.reports.year_md_config_path);
  }

  if (!has_any_report_format) {
    ThrowConfigFieldError(
        bundle_path, "paths.reports",
        "must contain at least one report format table (markdown, latex, "
        "typst).");
  }

  return true;
}

}  // namespace ConfigParserUtils::internal

// infrastructure/config/internal/config_parser_bundle.cpp
#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <vector>

#include "infrastructure/config/internal/config_parser_utils_internal.hpp"

namespace ConfigParserUtils::internal {

namespace {

auto IsHexColor(const std::string& value) -> bool {
  constexpr std::size_t kHexColorLength = 7;
  if (value.size() != kHexColorLength || value.front() != '#') {
    return false;
  }
  return std::all_of(value.begin() + 1, value.end(),
                     [](unsigned char code_point) -> bool {
                       return std::isxdigit(code_point) != 0;
                     });
}

auto ValidatePositiveHours(const toml::table& thresholds_table,
                           const fs::path& heatmap_path) -> std::size_t {
  const toml::node* positive_hours_node =
      thresholds_table.get("positive_hours");
  if (positive_hours_node == nullptr) {
    ThrowConfigFieldError(heatmap_path, "thresholds.positive_hours",
                          "is required and must be an array.");
  }

  const toml::array* positive_hours_array = positive_hours_node->as_array();
  if (positive_hours_array == nullptr || positive_hours_array->empty()) {
    ThrowConfigFieldError(heatmap_path, "thresholds.positive_hours",
                          "must be a non-empty array.");
  }

  std::vector<double> positive_hours;
  positive_hours.reserve(positive_hours_array->size());
  for (std::size_t index = 0; index < positive_hours_array->size(); ++index) {
    const std::string kFieldPath =
        "thresholds.positive_hours[" + std::to_string(index) + "]";
    const toml::node* item = positive_hours_array->get(index);
    if (item == nullptr) {
      ThrowConfigFieldError(heatmap_path, kFieldPath,
                            "must be a positive number.");
    }
    const auto kValue = item->value<double>();
    if (!kValue.has_value() || !std::isfinite(*kValue) || *kValue <= 0.0) {
      ThrowConfigFieldError(heatmap_path, kFieldPath,
                            "must be a positive number.");
    }
    if (!positive_hours.empty() && *kValue <= positive_hours.back()) {
      ThrowConfigFieldError(heatmap_path, kFieldPath,
                            "must be strictly increasing.");
    }
    positive_hours.push_back(*kValue);
  }

  return positive_hours.size() + 1U;
}

auto ValidatePalettes(const toml::table& palettes_table,
                      const fs::path& heatmap_path,
                      std::size_t expected_palette_size,
                      const std::string& light_palette,
                      const std::string& dark_palette) -> void {
  bool has_light_palette = false;
  bool has_dark_palette = false;
  for (const auto& [key, node] : palettes_table) {
    const std::string kPaletteName = std::string(key.str());
    if (kPaletteName == light_palette) {
      has_light_palette = true;
    }
    if (kPaletteName == dark_palette) {
      has_dark_palette = true;
    }

    const toml::array* colors = node.as_array();
    if (colors == nullptr) {
      ThrowConfigFieldError(heatmap_path, "palettes." + kPaletteName,
                            "must be an array of hex colors.");
    }
    if (colors->size() != expected_palette_size) {
      ThrowConfigFieldError(heatmap_path, "palettes." + kPaletteName,
                            "must contain exactly " +
                                std::to_string(expected_palette_size) +
                                " colors to match thresholds.positive_hours.");
    }
    for (std::size_t index = 0; index < colors->size(); ++index) {
      const std::string kFieldPath =
          "palettes." + kPaletteName + "[" + std::to_string(index) + "]";
      const toml::node* color_node = colors->get(index);
      const auto kColorValue = color_node == nullptr
                                   ? std::optional<std::string>{}
                                   : color_node->value<std::string>();
      if (!kColorValue.has_value() || !IsHexColor(*kColorValue)) {
        ThrowConfigFieldError(heatmap_path, kFieldPath,
                              "must be a hex color (#RRGGBB).");
      }
    }
  }

  if (!has_light_palette) {
    ThrowConfigFieldError(heatmap_path, "defaults.light_palette",
                          "must reference an existing palette key.");
  }
  if (!has_dark_palette) {
    ThrowConfigFieldError(heatmap_path, "defaults.dark_palette",
                          "must reference an existing palette key.");
  }
}

auto ValidateHeatmapConfigFile(const fs::path& heatmap_path) -> void {
  toml::table table;
  try {
    table = toml::parse_file(heatmap_path.string());
  } catch (const toml::parse_error& err) {
    ThrowConfigFieldError(
        heatmap_path, "",
        "failed to parse TOML: " + std::string(err.description()) + ".");
  }

  const auto kSchemaVersion = RequireTypedField<int64_t>(
      table, "schema_version", heatmap_path, "", "an integer");
  if (kSchemaVersion != 1) {
    ThrowConfigFieldError(heatmap_path, "schema_version",
                          "must be 1 for current runtime.");
  }

  const toml::table* thresholds_tbl =
      TryReadTableField(table, "thresholds", heatmap_path, "");
  if (thresholds_tbl == nullptr) {
    ThrowConfigFieldError(heatmap_path, "thresholds",
                          "is required and must be a table.");
  }
  const std::size_t kExpectedPaletteSize =
      ValidatePositiveHours(*thresholds_tbl, heatmap_path);

  const toml::table* defaults_tbl =
      TryReadTableField(table, "defaults", heatmap_path, "");
  if (defaults_tbl == nullptr) {
    ThrowConfigFieldError(heatmap_path, "defaults",
                          "is required and must be a table.");
  }
  const std::string kLightPalette = RequireNonEmptyStringField(
      *defaults_tbl, "light_palette", heatmap_path, "defaults");
  const std::string kDarkPalette = RequireNonEmptyStringField(
      *defaults_tbl, "dark_palette", heatmap_path, "defaults");

  const toml::table* palettes_tbl =
      TryReadTableField(table, "palettes", heatmap_path, "");
  if (palettes_tbl == nullptr || palettes_tbl->empty()) {
    ThrowConfigFieldError(heatmap_path, "palettes",
                          "is required and must contain at least one palette.");
  }

  ValidatePalettes(*palettes_tbl, heatmap_path, kExpectedPaletteSize,
                   kLightPalette, kDarkPalette);
}

}  // namespace

auto LoadReportPathsFromTable(const toml::table& section,
                              const ReportPathSource& source,
                              std::string_view section_field_path,
                              fs::path& day_path, fs::path& month_path,
                              fs::path& period_path, fs::path& week_path,
                              fs::path& year_path) -> void {
  EnsureFieldAbsent(section, "range", source.source_path, section_field_path,
                    "period");

  const std::string kDay = RequireNonEmptyStringField(
      section, "day", source.source_path, section_field_path);
  day_path = NormalizeConfigRelativePath(source.config_dir, kDay);
  EnsureFileExists(source.source_path, JoinFieldPath(section_field_path, "day"),
                   day_path);

  const std::string kMonth = RequireNonEmptyStringField(
      section, "month", source.source_path, section_field_path);
  month_path = NormalizeConfigRelativePath(source.config_dir, kMonth);
  EnsureFileExists(source.source_path,
                   JoinFieldPath(section_field_path, "month"), month_path);

  const std::string kPeriod = RequireNonEmptyStringField(
      section, "period", source.source_path, section_field_path);
  period_path = NormalizeConfigRelativePath(source.config_dir, kPeriod);
  EnsureFileExists(source.source_path,
                   JoinFieldPath(section_field_path, "period"), period_path);

  const std::string kWeek = RequireNonEmptyStringField(
      section, "week", source.source_path, section_field_path);
  week_path = NormalizeConfigRelativePath(source.config_dir, kWeek);
  EnsureFileExists(source.source_path,
                   JoinFieldPath(section_field_path, "week"), week_path);

  const std::string kYear = RequireNonEmptyStringField(
      section, "year", source.source_path, section_field_path);
  year_path = NormalizeConfigRelativePath(source.config_dir, kYear);
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

auto TryParseBundlePathsImpl(const fs::path& config_dir, AppConfig& config)
    -> bool {
  const fs::path kBundlePath = ResolveBundlePathImpl(config_dir);
  if (!fs::exists(kBundlePath)) {
    return false;
  }

  toml::table bundle_tbl;
  try {
    bundle_tbl = toml::parse_file(kBundlePath.string());
  } catch (const toml::parse_error& err) {
    throw std::runtime_error("Failed to parse bundle TOML [" +
                             kBundlePath.string() +
                             "]: " + std::string(err.description()));
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

  const toml::table* paths_tbl =
      TryReadTableField(bundle_tbl, "paths", kBundlePath, "");
  if (paths_tbl == nullptr) {
    ThrowConfigFieldError(kBundlePath, "paths",
                          "is required and must be a table.");
  }

  const toml::table* converter_tbl =
      TryReadTableField(*paths_tbl, "converter", kBundlePath, "paths");
  if (converter_tbl == nullptr) {
    ThrowConfigFieldError(kBundlePath, "paths.converter",
                          "is required and must be a table.");
  }

  EnsureFieldAbsent(*converter_tbl, "interval_processor_config", kBundlePath,
                    "paths.converter", "paths.converter.interval_config");
  EnsureFieldAbsent(*converter_tbl, "interval_processor_config_path",
                    kBundlePath, "paths.converter",
                    "paths.converter.interval_config");
  const std::string kIntervalConfig = RequireNonEmptyStringField(
      *converter_tbl, "interval_config", kBundlePath, "paths.converter");
  config.pipeline.interval_processor_config_path =
      NormalizeConfigRelativePath(config_dir, kIntervalConfig);
  EnsureFileExists(kBundlePath, "paths.converter.interval_config",
                   config.pipeline.interval_processor_config_path);

  const toml::table* visualization_tbl =
      TryReadTableField(*paths_tbl, "visualization", kBundlePath, "paths");
  if (visualization_tbl == nullptr) {
    ThrowConfigFieldError(kBundlePath, "paths.visualization",
                          "is required and must be a table.");
  }
  const std::string kHeatmapConfig = RequireNonEmptyStringField(
      *visualization_tbl, "heatmap", kBundlePath, "paths.visualization");
  const fs::path kHeatmapConfigPath =
      NormalizeConfigRelativePath(config_dir, kHeatmapConfig);
  EnsureFileExists(kBundlePath, "paths.visualization.heatmap",
                   kHeatmapConfigPath);
  ValidateHeatmapConfigFile(kHeatmapConfigPath);

  const toml::table* reports_tbl =
      TryReadTableField(*paths_tbl, "reports", kBundlePath, "paths");
  if (reports_tbl == nullptr) {
    ThrowConfigFieldError(kBundlePath, "paths.reports",
                          "is required and must be a table.");
  }

  EnsureFieldAbsent(*reports_tbl, "md", kBundlePath, "paths.reports",
                    "paths.reports.markdown");
  EnsureFieldAbsent(*reports_tbl, "tex", kBundlePath, "paths.reports",
                    "paths.reports.latex");
  EnsureFieldAbsent(*reports_tbl, "typ", kBundlePath, "paths.reports",
                    "paths.reports.typst");

  const ReportPathSource kReportPathSource{
      .config_dir = config_dir,
      .source_path = kBundlePath,
  };

  bool has_any_report_format = false;

  if (const toml::table* typst_tbl = TryReadTableField(
          *reports_tbl, "typst", kBundlePath, "paths.reports")) {
    has_any_report_format = true;
    LoadReportPathsFromTable(*typst_tbl, kReportPathSource,
                             "paths.reports.typst",
                             config.reports.day_typ_config_path,
                             config.reports.month_typ_config_path,
                             config.reports.period_typ_config_path,
                             config.reports.week_typ_config_path,
                             config.reports.year_typ_config_path);
  }

  if (const toml::table* latex_tbl = TryReadTableField(
          *reports_tbl, "latex", kBundlePath, "paths.reports")) {
    has_any_report_format = true;
    LoadReportPathsFromTable(*latex_tbl, kReportPathSource,
                             "paths.reports.latex",
                             config.reports.day_tex_config_path,
                             config.reports.month_tex_config_path,
                             config.reports.period_tex_config_path,
                             config.reports.week_tex_config_path,
                             config.reports.year_tex_config_path);
  }

  if (const toml::table* markdown_tbl = TryReadTableField(
          *reports_tbl, "markdown", kBundlePath, "paths.reports")) {
    has_any_report_format = true;
    LoadReportPathsFromTable(
        *markdown_tbl, kReportPathSource, "paths.reports.markdown",
        config.reports.day_md_config_path, config.reports.month_md_config_path,
        config.reports.period_md_config_path,
        config.reports.week_md_config_path, config.reports.year_md_config_path);
  }

  if (!has_any_report_format) {
    ThrowConfigFieldError(
        kBundlePath, "paths.reports",
        "must contain at least one report format table (markdown, latex, "
        "typst).");
  }

  return true;
}

}  // namespace ConfigParserUtils::internal

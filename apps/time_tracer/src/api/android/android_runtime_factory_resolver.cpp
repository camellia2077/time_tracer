// api/android/android_runtime_factory_resolver.cpp
#include <toml++/toml.h>

#include <filesystem>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>

#include "api/android/android_runtime_factory_internal.hpp"
#include "infrastructure/config/internal/config_parser_utils.hpp"
#include "infrastructure/persistence/importer/sqlite/connection.hpp"

#ifndef TT_REPORT_ENABLE_LATEX
#define TT_REPORT_ENABLE_LATEX 1
#endif

#ifndef TT_REPORT_ENABLE_TYPST
#define TT_REPORT_ENABLE_TYPST 1
#endif

namespace infrastructure::bootstrap::android_runtime_detail {

namespace {

namespace fs = std::filesystem;

constexpr std::string_view kDatabaseFilename = "time_data.sqlite3";

[[nodiscard]] auto RequireNonEmptyPath(const fs::path& path,
                                       std::string_view field_name)
    -> fs::path {
  if (path.empty()) {
    throw std::invalid_argument("Android runtime " + std::string(field_name) +
                                " must not be empty.");
  }
  return fs::absolute(path);
}

[[nodiscard]] auto ResolveConverterConfigTomlPath(
    const fs::path& converter_config_toml_path) -> fs::path {
  if (converter_config_toml_path.empty()) {
    throw std::invalid_argument(
        "Android runtime converter_config_toml_path must not be empty.");
  }

  const fs::path resolved = fs::absolute(converter_config_toml_path);
  if (!fs::exists(resolved)) {
    throw std::runtime_error("Converter config TOML not found: " +
                             resolved.string());
  }

  return resolved;
}

[[nodiscard]] auto ResolveRequiredReportTomlPath(const fs::path& path,
                                                 std::string_view field_name)
    -> fs::path {
  if (path.empty()) {
    throw std::runtime_error("Android runtime bundle field '" +
                             std::string(field_name) + "' is required.");
  }

  const fs::path resolved = fs::absolute(path);
  if (!fs::exists(resolved)) {
    throw std::runtime_error("Android runtime report TOML not found: " +
                             resolved.string());
  }
  return resolved;
}

[[nodiscard]] auto ResolveReportConfigRoot(
    const fs::path& converter_config_toml_path) -> fs::path {
  const fs::path converter_dir = converter_config_toml_path.parent_path();
  if (converter_dir.filename() != "converter") {
    throw std::runtime_error(
        "Converter config path must be under 'config/"
        "converter': " +
        converter_config_toml_path.string());
  }

  const fs::path config_root = converter_dir.parent_path();
  if (config_root.empty() || !fs::exists(config_root)) {
    throw std::runtime_error("Config root not found for Android runtime: " +
                             config_root.string());
  }
  return config_root;
}

[[nodiscard]] auto JoinFieldPath(std::string_view parent, std::string_view key)
    -> std::string {
  if (parent.empty()) {
    return std::string(key);
  }
  return std::string(parent) + "." + std::string(key);
}

[[noreturn]] auto ThrowBundleFieldError(const fs::path& bundle_path,
                                        std::string_view field_path,
                                        std::string_view message) -> void {
  throw std::runtime_error("Invalid config [" + bundle_path.string() +
                           "] field '" + std::string(field_path) +
                           "': " + std::string(message));
}

[[nodiscard]] auto RequireTableField(const toml::table& parent,
                                     std::string_view key,
                                     const fs::path& bundle_path,
                                     std::string_view parent_field_path)
    -> const toml::table* {
  const std::string field_path = JoinFieldPath(parent_field_path, key);
  const toml::node* node = parent.get(key);
  if (node == nullptr) {
    ThrowBundleFieldError(bundle_path, field_path,
                          "is required and must be a table.");
  }
  const toml::table* table = node->as_table();
  if (table == nullptr) {
    ThrowBundleFieldError(bundle_path, field_path, "must be a table.");
  }
  return table;
}

[[nodiscard]] auto RequireStringField(const toml::table& parent,
                                      std::string_view key,
                                      const fs::path& bundle_path,
                                      std::string_view parent_field_path)
    -> std::string {
  const std::string field_path = JoinFieldPath(parent_field_path, key);
  const toml::node* node = parent.get(key);
  if (node == nullptr || !node->is_string()) {
    ThrowBundleFieldError(bundle_path, field_path,
                          "is required and must be a string.");
  }
  const auto value = node->value<std::string>();
  if (!value.has_value() || value->empty()) {
    ThrowBundleFieldError(bundle_path, field_path,
                          "must be a non-empty string.");
  }
  return *value;
}

[[nodiscard]] auto ValidateAndroidBundlePolicy(const fs::path& config_root)
    -> bool {
  const fs::path bundle_path =
      ConfigParserUtils::ResolveBundlePath(config_root);
  if (!fs::exists(bundle_path)) {
    return false;
  }

  toml::table bundle_tbl;
  try {
    bundle_tbl = toml::parse_file(bundle_path.string());
  } catch (const toml::parse_error& error) {
    throw std::runtime_error("Failed to parse bundle TOML [" +
                             bundle_path.string() +
                             "]: " + std::string(error.description()));
  }

  const std::string profile =
      RequireStringField(bundle_tbl, "profile", bundle_path, "");
  const toml::table* paths_tbl =
      RequireTableField(bundle_tbl, "paths", bundle_path, "");
  const toml::table* reports_tbl =
      RequireTableField(*paths_tbl, "reports", bundle_path, "paths");

  const toml::node* markdown_node = reports_tbl->get("markdown");
  if (markdown_node == nullptr || markdown_node->as_table() == nullptr) {
    ThrowBundleFieldError(bundle_path, "paths.reports.markdown",
                          "is required and must be a table for Android "
                          "runtime.");
  }

  if (profile == "android") {
    if (reports_tbl->get("latex") != nullptr) {
      ThrowBundleFieldError(bundle_path, "paths.reports.latex",
                            "must not be present when profile is 'android'.");
    }
    if (reports_tbl->get("typst") != nullptr) {
      ThrowBundleFieldError(bundle_path, "paths.reports.typst",
                            "must not be present when profile is 'android'.");
    }
  }

  return true;
}

[[nodiscard]] auto BuildLegacyMarkdownConfigPaths(const fs::path& config_root)
    -> AndroidReportConfigPathSet {
  const fs::path markdown_dir = config_root / "reports" / "markdown";
  return {
      .day = ResolveRequiredReportTomlPath(markdown_dir / "day.toml",
                                           "paths.reports.markdown.day"),
      .month = ResolveRequiredReportTomlPath(markdown_dir / "month.toml",
                                             "paths.reports.markdown.month"),
      .period = ResolveRequiredReportTomlPath(markdown_dir / "period.toml",
                                              "paths.reports.markdown.period"),
      .week = ResolveRequiredReportTomlPath(markdown_dir / "week.toml",
                                            "paths.reports.markdown.week"),
      .year = ResolveRequiredReportTomlPath(markdown_dir / "year.toml",
                                            "paths.reports.markdown.year"),
  };
}

[[nodiscard]] auto BuildBundleMarkdownConfigPaths(const AppConfig& config)
    -> AndroidReportConfigPathSet {
  return {
      .day = ResolveRequiredReportTomlPath(config.reports.day_md_config_path,
                                           "paths.reports.markdown.day"),
      .month = ResolveRequiredReportTomlPath(
          config.reports.month_md_config_path, "paths.reports.markdown.month"),
      .period =
          ResolveRequiredReportTomlPath(config.reports.period_md_config_path,
                                        "paths.reports.markdown.period"),
      .week = ResolveRequiredReportTomlPath(config.reports.week_md_config_path,
                                            "paths.reports.markdown.week"),
      .year = ResolveRequiredReportTomlPath(config.reports.year_md_config_path,
                                            "paths.reports.markdown.year"),
  };
}

[[nodiscard]] auto BuildBundleLatexConfigPaths(const AppConfig& config)
    -> std::optional<AndroidReportConfigPathSet> {
  const bool any_defined = !config.reports.day_tex_config_path.empty() ||
                           !config.reports.month_tex_config_path.empty() ||
                           !config.reports.period_tex_config_path.empty() ||
                           !config.reports.week_tex_config_path.empty() ||
                           !config.reports.year_tex_config_path.empty();
#if !TT_REPORT_ENABLE_LATEX
  if (any_defined) {
    throw std::runtime_error(
        "Android runtime bundle contains LaTeX report paths, but this core "
        "build disables LaTeX (TT_REPORT_ENABLE_LATEX=OFF).");
  }
  return std::nullopt;
#else
  if (!any_defined) {
    return std::nullopt;
  }
  return AndroidReportConfigPathSet{
      .day = ResolveRequiredReportTomlPath(config.reports.day_tex_config_path,
                                           "paths.reports.latex.day"),
      .month = ResolveRequiredReportTomlPath(
          config.reports.month_tex_config_path, "paths.reports.latex.month"),
      .period = ResolveRequiredReportTomlPath(
          config.reports.period_tex_config_path, "paths.reports.latex.period"),
      .week = ResolveRequiredReportTomlPath(config.reports.week_tex_config_path,
                                            "paths.reports.latex.week"),
      .year = ResolveRequiredReportTomlPath(config.reports.year_tex_config_path,
                                            "paths.reports.latex.year"),
  };
#endif
}

[[nodiscard]] auto BuildBundleTypstConfigPaths(const AppConfig& config)
    -> std::optional<AndroidReportConfigPathSet> {
  const bool any_defined = !config.reports.day_typ_config_path.empty() ||
                           !config.reports.month_typ_config_path.empty() ||
                           !config.reports.period_typ_config_path.empty() ||
                           !config.reports.week_typ_config_path.empty() ||
                           !config.reports.year_typ_config_path.empty();
#if !TT_REPORT_ENABLE_TYPST
  if (any_defined) {
    throw std::runtime_error(
        "Android runtime bundle contains Typst report paths, but this core "
        "build disables Typst (TT_REPORT_ENABLE_TYPST=OFF).");
  }
  return std::nullopt;
#else
  if (!any_defined) {
    return std::nullopt;
  }
  return AndroidReportConfigPathSet{
      .day = ResolveRequiredReportTomlPath(config.reports.day_typ_config_path,
                                           "paths.reports.typst.day"),
      .month = ResolveRequiredReportTomlPath(
          config.reports.month_typ_config_path, "paths.reports.typst.month"),
      .period = ResolveRequiredReportTomlPath(
          config.reports.period_typ_config_path, "paths.reports.typst.period"),
      .week = ResolveRequiredReportTomlPath(config.reports.week_typ_config_path,
                                            "paths.reports.typst.week"),
      .year = ResolveRequiredReportTomlPath(config.reports.year_typ_config_path,
                                            "paths.reports.typst.year"),
  };
#endif
}

}  // namespace

auto ResolveOutputRoot(const fs::path& output_root) -> fs::path {
  if (output_root.empty()) {
    throw std::invalid_argument(
        "Android runtime output_root must not be empty.");
  }
  return fs::absolute(output_root);
}

auto ResolveDbPath(const fs::path& db_path, const fs::path& output_root)
    -> fs::path {
  if (!db_path.empty()) {
    return fs::absolute(db_path);
  }
  return fs::absolute(output_root / "db" / kDatabaseFilename);
}

auto EnsureDatabaseBootstrapped(const fs::path& db_path) -> void {
  fs::create_directories(db_path.parent_path());

  infrastructure::persistence::importer::sqlite::Connection bootstrap_db(
      db_path.string());
  if (bootstrap_db.GetDb() == nullptr) {
    throw std::runtime_error("Failed to initialize database at: " +
                             db_path.string());
  }
}

auto ResolveAndroidRuntimeConfigPaths(
    const fs::path& requested_converter_config_toml_path)
    -> AndroidRuntimeConfigPaths {
  const fs::path requested_converter_config_path = RequireNonEmptyPath(
      requested_converter_config_toml_path, "converter_config_toml_path");
  const fs::path config_root =
      ResolveReportConfigRoot(requested_converter_config_path);

  const bool has_bundle = ValidateAndroidBundlePolicy(config_root);
  if (has_bundle) {
    AppConfig bundle_config;
    const bool bundle_loaded =
        ConfigParserUtils::TryParseBundlePaths(config_root, bundle_config);
    if (!bundle_loaded) {
      throw std::runtime_error(
          "Bundle path config disappeared during Android runtime bootstrap: " +
          ConfigParserUtils::ResolveBundlePath(config_root).string());
    }
    const std::optional<AndroidReportConfigPathSet> latex_paths =
        BuildBundleLatexConfigPaths(bundle_config);
    const std::optional<AndroidReportConfigPathSet> typst_paths =
        BuildBundleTypstConfigPaths(bundle_config);

    return {
        .converter_config_toml_path = ResolveConverterConfigTomlPath(
            bundle_config.pipeline.interval_processor_config_path),
        .markdown = BuildBundleMarkdownConfigPaths(bundle_config),
        .latex = latex_paths,
        .typst = typst_paths,
        .formatter_policy = {.enable_markdown = true,
                             .enable_latex = latex_paths.has_value(),
                             .enable_typst = typst_paths.has_value()},
    };
  }

  return {
      .converter_config_toml_path =
          ResolveConverterConfigTomlPath(requested_converter_config_path),
      .markdown = BuildLegacyMarkdownConfigPaths(config_root),
      .latex = std::nullopt,
      .typst = std::nullopt,
      .formatter_policy = infrastructure::reports::
          AndroidStaticReportFormatterPolicy::MarkdownOnly(),
  };
}

}  // namespace infrastructure::bootstrap::android_runtime_detail

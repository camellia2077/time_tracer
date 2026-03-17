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

[[nodiscard]] auto ToAndroidReportConfigPathSet(
    const infra_config_internal::AndroidBundleReportConfigPathSet& config)
    -> AndroidReportConfigPathSet {
  return {
      .day = ResolveRequiredReportTomlPath(config.day,
                                           "paths.reports.markdown.day"),
      .month = ResolveRequiredReportTomlPath(config.month,
                                             "paths.reports.markdown.month"),
      .period = ResolveRequiredReportTomlPath(config.period,
                                              "paths.reports.markdown.period"),
      .week = ResolveRequiredReportTomlPath(config.week,
                                            "paths.reports.markdown.week"),
      .year = ResolveRequiredReportTomlPath(config.year,
                                            "paths.reports.markdown.year"),
  };
}

}  // namespace

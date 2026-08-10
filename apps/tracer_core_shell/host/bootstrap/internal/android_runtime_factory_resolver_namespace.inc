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

[[nodiscard]] auto ResolveRequiredInsightsTomlPath(const fs::path& path,
                                                 std::string_view field_name)
    -> fs::path {
  if (path.empty()) {
    throw std::runtime_error("Android runtime bundle field '" +
                             std::string(field_name) + "' is required.");
  }

  const fs::path resolved = fs::absolute(path);
  if (!fs::exists(resolved)) {
    throw std::runtime_error("Android runtime insights TOML not found: " +
                             resolved.string());
  }
  return resolved;
}

[[nodiscard]] auto ResolveInsightsConfigRoot(
    const fs::path& converter_config_toml_path) -> fs::path {
  const fs::path user_config_dir = converter_config_toml_path.parent_path();
  if (user_config_dir.filename() != "user" ||
      converter_config_toml_path.filename() != "behavior.toml") {
    throw std::runtime_error(
        "Converter main config path must be 'config/user/behavior.toml': " +
        converter_config_toml_path.string());
  }

  const fs::path config_root = user_config_dir.parent_path();
  if (config_root.empty() || !fs::exists(config_root)) {
    throw std::runtime_error("Config root not found for Android runtime: " +
                             config_root.string());
  }
  return config_root;
}

[[nodiscard]] auto ToAndroidInsightsConfigPathSet(
    const infra_config_internal::AndroidBundleInsightsConfigPathSet& config)
    -> AndroidInsightsConfigPathSet {
  return {
      .day = ResolveRequiredInsightsTomlPath(config.day,
                                           "insights.markdown.root"),
      .month = ResolveRequiredInsightsTomlPath(config.month,
                                             "insights.markdown.root"),
      .period = ResolveRequiredInsightsTomlPath(config.period,
                                              "insights.markdown.root"),
      .week = ResolveRequiredInsightsTomlPath(config.week,
                                            "insights.markdown.root"),
      .year = ResolveRequiredInsightsTomlPath(config.year,
                                            "insights.markdown.root"),
  };
}

}  // namespace

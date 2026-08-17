struct AndroidBundleInsightsConfigPathSet {
  std::filesystem::path daily;
  std::filesystem::path month;
  std::filesystem::path period;
  std::filesystem::path week;
  std::filesystem::path year;
};

struct AndroidBundleConfigPaths {
  std::filesystem::path converter_config_toml_path;
  AndroidBundleInsightsConfigPathSet markdown;
  std::optional<AndroidBundleInsightsConfigPathSet> latex;
  std::optional<AndroidBundleInsightsConfigPathSet> typst;
};

[[nodiscard]] auto TryResolveAndroidBundleConfigPaths(
    const std::filesystem::path& config_dir)
    -> std::optional<AndroidBundleConfigPaths>;

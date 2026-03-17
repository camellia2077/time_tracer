struct AndroidBundleReportConfigPathSet {
  std::filesystem::path day;
  std::filesystem::path month;
  std::filesystem::path period;
  std::filesystem::path week;
  std::filesystem::path year;
};

struct AndroidBundleConfigPaths {
  std::filesystem::path converter_config_toml_path;
  AndroidBundleReportConfigPathSet markdown;
  std::optional<AndroidBundleReportConfigPathSet> latex;
  std::optional<AndroidBundleReportConfigPathSet> typst;
};

[[nodiscard]] auto TryResolveAndroidBundleConfigPaths(
    const std::filesystem::path& config_dir)
    -> std::optional<AndroidBundleConfigPaths>;

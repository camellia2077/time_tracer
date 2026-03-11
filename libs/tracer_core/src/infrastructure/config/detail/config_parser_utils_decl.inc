void ParseSystemSettings(const toml::table& tbl,
                         const std::filesystem::path& exe_path,
                         const std::filesystem::path& source_config_path,
                         AppConfig& config);

auto ResolveBundlePath(const std::filesystem::path& config_dir)
    -> std::filesystem::path;

auto TryParseBundlePaths(const std::filesystem::path& config_dir,
                         AppConfig& config) -> bool;

void ParseCliDefaults(const toml::table& tbl,
                      const std::filesystem::path& exe_path,
                      const std::filesystem::path& source_config_path,
                      AppConfig& config);

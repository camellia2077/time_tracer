// infrastructure/config/internal/config_parser_utils.cpp
#include "infrastructure/config/internal/config_parser_utils.hpp"

#include "infrastructure/config/internal/config_parser_utils_internal.hpp"

namespace ConfigParserUtils {

// NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
void ParseSystemSettings(const toml::table& tbl,
                         const std::filesystem::path& exe_path,
                         const std::filesystem::path& source_config_path,
                         AppConfig& config) {
  internal::ParseSystemSettingsImpl(tbl, exe_path, source_config_path, config);
}

void ParseCliDefaults(const toml::table& tbl,
                      const std::filesystem::path& exe_path,
                      const std::filesystem::path& source_config_path,
                      AppConfig& config) {
  internal::ParseCliDefaultsImpl(tbl, exe_path, source_config_path, config);
}

auto ResolveBundlePath(const std::filesystem::path& config_dir)
    -> std::filesystem::path {
  return internal::ResolveBundlePathImpl(config_dir);
}

auto TryParseBundlePaths(const std::filesystem::path& config_dir,
                         AppConfig& config) -> bool {
  return internal::TryParseBundlePathsImpl(config_dir, config);
}

}  // namespace ConfigParserUtils

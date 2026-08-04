module;

#include <toml++/toml.h>

#include <filesystem>

#include "infra/config/internal/config_parser_utils_internal.hpp"

module tracer.core.infrastructure.config.internal.config_parser_utils;

namespace config_parser_internal = ConfigParserUtils::internal;

namespace tracer::core::infrastructure::config::internal {

void ParseSystemSettings(const toml::table& tbl,
                         const std::filesystem::path& exe_path,
                         const std::filesystem::path& source_config_path,
                         AppConfig& config) {
  config_parser_internal::ParseSystemSettingsImpl(tbl, exe_path,
                                                  source_config_path, config);
}

void ParseCliDefaults(const toml::table& tbl,
                      const std::filesystem::path& exe_path,
                      const std::filesystem::path& source_config_path,
                      AppConfig& config) {
  config_parser_internal::ParseCliDefaultsImpl(tbl, exe_path,
                                               source_config_path, config);
}

void ParseRuntimeConfigPaths(const toml::table& config_tbl,
                             const std::filesystem::path& config_dir,
                             const std::filesystem::path& source_path,
                             AppConfig& config) {
  config_parser_internal::ParseRuntimeConfigPaths(config_tbl, config_dir,
                                                  source_path, config);
}

auto ResolveBundlePath(const std::filesystem::path& config_dir)
    -> std::filesystem::path {
  return config_parser_internal::ResolveBundlePathImpl(config_dir);
}

auto TryParseBundlePaths(const std::filesystem::path& config_dir,
                         AppConfig& config) -> bool {
  return config_parser_internal::TryParseBundlePathsImpl(config_dir, config);
}

}  // namespace tracer::core::infrastructure::config::internal

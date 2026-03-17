// infrastructure/config/config_loader.cpp
import tracer.adapters.io.core.fs;
import tracer.core.infrastructure.config.internal.config_detail_loader;
import tracer.core.infrastructure.config.internal.config_parser_utils;

#include "infrastructure/config/config_loader.hpp"

#include <toml++/toml.h>

#include <stdexcept>

namespace fs = std::filesystem;
namespace modcore = tracer::adapters::io::modcore;

namespace infra_config_internal = tracer::core::infrastructure::config::internal;

namespace tracer::core::infrastructure::config {

ConfigLoader::ConfigLoader(const std::string& exe_path_str) {
  try {
    exe_path_ = fs::canonical(fs::path(exe_path_str)).parent_path();
  } catch (const fs::filesystem_error& e) {
    throw std::runtime_error("Failed to determine executable directory: " +
                             std::string(e.what()));
  }

  config_dir_path_ = exe_path_ / kConfigDirName;
  main_config_path_ = config_dir_path_ / kConfigFileName;
}

auto ConfigLoader::GetMainConfigPath() const -> std::string {
  return main_config_path_.string();
}

auto ConfigLoader::LoadConfiguration() -> AppConfig {
  if (!modcore::Exists(main_config_path_)) {
    throw std::runtime_error("Configuration file not found: " +
                             main_config_path_.string());
  }

  toml::table tbl;
  try {
    tbl = toml::parse_file(main_config_path_.string());
  } catch (const toml::parse_error& err) {
    throw std::runtime_error("Failed to parse config.toml: " +
                             std::string(err.description()));
  }

  AppConfig app_config;
  app_config.exe_dir_path = exe_path_;

  // 1. 解析基础路径和设置
  infra_config_internal::ParseSystemSettings(tbl, exe_path_,
                                             main_config_path_, app_config);
  infra_config_internal::ParseCliDefaults(tbl, exe_path_, main_config_path_,
                                          app_config);

  const bool kBundlePathsLoaded = infra_config_internal::TryParseBundlePaths(
      config_dir_path_, app_config);
  if (!kBundlePathsLoaded) {
    const fs::path kBundlePath =
        infra_config_internal::ResolveBundlePath(config_dir_path_);
    throw std::runtime_error(
        "Bundle path config not found: " + kBundlePath.string() +
        ". Legacy [converter]/[reports] fallback was removed. "
        "Run `python tools/run.py config-migrate --app tracer_windows "
        "--apply` first.");
  }

  // 2. 加载报表配置
  try {
    infra_config_internal::LoadDetailedReports(app_config);
  } catch (const std::exception& e) {
    throw std::runtime_error("Failed to load report configuration details: " +
                             std::string(e.what()));
  }

  return app_config;
}

}  // namespace tracer::core::infrastructure::config

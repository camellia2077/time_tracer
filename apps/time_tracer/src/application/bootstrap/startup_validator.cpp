// bootstrap/startup_validator.cpp
#include "application/bootstrap/startup_validator.hpp"

#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

#include "infrastructure/config/validator/plugins/facade/plugin_validator.hpp"
#include "infrastructure/reports/plugin_manifest.hpp"
#include "shared/types/ansi_colors.hpp"

namespace fs = std::filesystem;

auto StartupValidator::ValidateEnvironment(const AppConfig& config) -> bool {
  const fs::path kPluginsDir = fs::path(config.exe_dir_path) / "plugins";
  const fs::path kBinDir = config.exe_dir_path;

  const std::vector<std::string> kExpectedFormatterPlugins =
      reports::plugin_manifest::GetExpectedFormatterPluginNames();
  const bool kPluginsOk =
      PluginValidator::Validate(kPluginsDir, kExpectedFormatterPlugins);

  std::vector<std::string> core_runtime_libraries = {
      std::string(reports::plugin_manifest::kCoreRuntimeLibraryName)};
  PluginValidationOptions core_validation_options{};
  core_validation_options.require_formatter_abi = false;
  const bool kCoreOk = PluginValidator::Validate(
      kBinDir, core_runtime_libraries, core_validation_options);

  if (!kPluginsOk || !kCoreOk) {
    namespace colors = time_tracer::common::colors;
    std::cerr << colors::kRed
              << "Fatal: Runtime environment validation failed "
                 "(missing or incompatible runtime DLLs)."
              << colors::kReset << std::endl;
    return false;
  }

  return true;
}

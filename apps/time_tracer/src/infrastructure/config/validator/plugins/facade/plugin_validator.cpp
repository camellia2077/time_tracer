// infrastructure/config/validator/plugins/facade/plugin_validator.cpp
#include "infrastructure/config/validator/plugins/facade/plugin_validator.hpp"

#include <iostream>
#include <vector>

#include "infrastructure/io/core/file_system_helper.hpp"

namespace fs = std::filesystem;

auto PluginValidator::Validate(const fs::path& plugins_path,
                               const std::vector<std::string>& expected_plugins)
    -> bool {
  // [修改] 使用 FileSystemHelper
  if (!FileSystemHelper::Exists(plugins_path) ||
      !FileSystemHelper::IsDirectory(plugins_path)) {
    std::cerr << "[Validator] Error: Plugins directory not found at '"
              << plugins_path.string() << "'." << std::endl;
    return expected_plugins.empty();
  }

  bool all_found = true;
  for (const auto& plugin_name : expected_plugins) {
    fs::path dll_path_with_prefix =
        plugins_path / ("lib" + plugin_name + ".dll");
    fs::path dll_path_without_prefix = plugins_path / (plugin_name + ".dll");

    // [修改] 使用 FileSystemHelper
    if (!FileSystemHelper::Exists(dll_path_with_prefix) &&
        !FileSystemHelper::Exists(dll_path_without_prefix)) {
      std::cerr << "[Validator] Error: Required plugin '" << plugin_name
                << ".dll' not found in the plugins directory." << std::endl;
      all_found = false;
    }
  }

  return all_found;
}

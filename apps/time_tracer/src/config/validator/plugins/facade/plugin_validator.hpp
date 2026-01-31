// config/validator/plugins/facade/plugin_validator.hpp
#ifndef CONFIG_VALIDATOR_PLUGINS_FACADE_PLUGIN_VALIDATOR_H_
#define CONFIG_VALIDATOR_PLUGINS_FACADE_PLUGIN_VALIDATOR_H_

#include <filesystem>
#include <string>
#include <vector>

namespace fs = std::filesystem;

class PluginValidator {
 public:
  static bool validate(const fs::path& plugins_path,
                       const std::vector<std::string>& expected_plugins);
};

#endif  // CONFIG_VALIDATOR_PLUGINS_FACADE_PLUGIN_VALIDATOR_H_
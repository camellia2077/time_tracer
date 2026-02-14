// infrastructure/config/validator/plugins/facade/plugin_validator.hpp
#ifndef INFRASTRUCTURE_CONFIG_VALIDATOR_PLUGINS_FACADE_PLUGIN_VALIDATOR_H_
#define INFRASTRUCTURE_CONFIG_VALIDATOR_PLUGINS_FACADE_PLUGIN_VALIDATOR_H_

#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>

namespace fs = std::filesystem;

struct PluginValidationOptions {
  bool require_formatter_abi = true;
  uint32_t expected_formatter_abi_version = 0U;
};

class PluginValidator {
 public:
  static auto Validate(const fs::path& plugins_path,
                       const std::vector<std::string>& expected_plugins,
                       const PluginValidationOptions& options =
                           PluginValidationOptions{}) -> bool;
};

#endif  // INFRASTRUCTURE_CONFIG_VALIDATOR_PLUGINS_FACADE_PLUGIN_VALIDATOR_H_

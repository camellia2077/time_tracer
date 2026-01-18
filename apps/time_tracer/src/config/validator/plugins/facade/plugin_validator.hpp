// config/validator/plugins/facade/plugin_validator.hpp
#ifndef CONFIG_VALIDATOR_PLUGINS_FACADE_PLUGIN_VALIDATOR_HPP_
#define CONFIG_VALIDATOR_PLUGINS_FACADE_PLUGIN_VALIDATOR_HPP_

#include <filesystem>
#include <string>
#include <vector>

namespace fs = std::filesystem;

class PluginValidator {
public:
    bool validate(const fs::path& plugins_path, const std::vector<std::string>& expected_plugins) const;
};

#endif // CONFIG_VALIDATOR_PLUGINS_FACADE_PLUGIN_VALIDATOR_HPP_
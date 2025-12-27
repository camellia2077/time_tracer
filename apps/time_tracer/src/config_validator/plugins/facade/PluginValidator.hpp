// config_validator/plugins/facade/PluginValidator.hpp
#ifndef PLUGIN_VALIDATOR_HPP
#define PLUGIN_VALIDATOR_HPP

#include <filesystem>
#include <string>
#include <vector>

namespace fs = std::filesystem;

class PluginValidator {
public:
    bool validate(const fs::path& plugins_path, const std::vector<std::string>& expected_plugins) const;
};

#endif // PLUGIN_VALIDATOR_HPP
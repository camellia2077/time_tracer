// src/config_validator/plugins/facade/PluginValidator.cpp
#include "PluginValidator.hpp"
#include <iostream>
#include <vector>

namespace fs = std::filesystem;

bool PluginValidator::validate(const fs::path& plugins_path, const std::vector<std::string>& expected_plugins) const {
    if (!fs::exists(plugins_path) || !fs::is_directory(plugins_path)) {
        std::cerr << "[Validator] Error: Plugins directory not found at '" << plugins_path.string() << "'." << std::endl;
        return expected_plugins.empty();
    }

    std::cout << "[Validator] Validating required plugins in: " << plugins_path.string() << std::endl;

    bool all_found = true;
    for (const auto& plugin_name : expected_plugins) {
        fs::path dll_path_with_prefix = plugins_path / ("lib" + plugin_name + ".dll");
        fs::path dll_path_without_prefix = plugins_path / (plugin_name + ".dll");

        if (!fs::exists(dll_path_with_prefix) && !fs::exists(dll_path_without_prefix)) {
            std::cerr << "[Validator] Error: Required plugin '" << plugin_name << ".dll' not found in the plugins directory." << std::endl;
            all_found = false;
        } else {
            std::cout << "[Validator] -- Found required plugin: " << plugin_name << std::endl;
        }
    }

    if(all_found) {
        std::cout << "[Validator] All required plugins were found." << std::endl;
    }

    return all_found;
}
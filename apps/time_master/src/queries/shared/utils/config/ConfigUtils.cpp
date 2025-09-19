// queries/shared/utils/config/ConfigUtils.cpp
#include "ConfigUtils.hpp"
#include <fstream>
#include <stdexcept>

nlohmann::json load_json_config(const std::string& config_path, const std::string& error_message_prefix) {
    std::ifstream config_file(config_path);
    if (!config_file.is_open()) {
        throw std::runtime_error(error_message_prefix + config_path);
    }
    nlohmann::json config_json;
    config_file >> config_json;
    return config_json;
}
// reports/shared/utils/config/ConfigUtils.cpp
#include "ConfigUtils.hpp"
#include <fstream>
#include <stdexcept>

#include "ConfigUtils.hpp"
#include <fstream>
#include <stdexcept>

// [修改] 实现更新
nlohmann::json load_json_config(const std::filesystem::path& config_path, const std::string& error_message_prefix) {
    std::ifstream config_file(config_path); 
    if (!config_file.is_open()) {
        throw std::runtime_error(error_message_prefix + config_path.string());
    }
    nlohmann::json config_json;
    config_file >> config_json;
    return config_json;
}
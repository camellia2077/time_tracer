#include "file_handler.h"
#include <iostream>
#include <fstream>
#include <stdexcept>
#include <nlohmann/json.hpp>

// Define namespace alias for convenience
namespace fs = std::filesystem;
std::vector<fs::path> FileHandler::find_text_files_recursively(const fs::path& root_path) {
    std::vector<fs::path> files_found;
    
    if (!fs::exists(root_path) || !fs::is_directory(root_path)) {
        // 如果路径不存在或不是一个目录，则直接返回空向量
        return files_found; 
    }

    // 使用 recursive_directory_iterator 进行递归遍历
    for (const auto& entry : fs::recursive_directory_iterator(root_path)) {
        if (entry.is_regular_file() && entry.path().extension() == ".txt") {
            files_found.push_back(entry.path());
        }
    }

    // 对找到的文件进行排序，确保处理顺序一致
    std::sort(files_found.begin(), files_found.end());

    return files_found;
}

FileHandler::FileHandler(const std::string& exe_path_str) {
    try {
        // 1. Get the canonical path to the executable's containing directory
        exe_path = fs::canonical(fs::path(exe_path_str)).parent_path();
    } catch (const fs::filesystem_error& e) {
        throw std::runtime_error("Failed to determine executable directory: " + std::string(e.what()));
    }

    // 2. Build the full paths to the config directory and the main config file
    config_dir_path = exe_path / CONFIG_DIR_NAME;
    main_config_path = config_dir_path / CONFIG_FILE_NAME;
}

std::string FileHandler::get_main_config_path() const {
    return main_config_path.string();
}

AppConfig FileHandler::load_configuration() {
    // 3. Check if the main configuration file exists
    if (!fs::exists(main_config_path)) {
        std::string error_msg = "Configuration file '" + CONFIG_FILE_NAME +
                                "' not found in the '" + CONFIG_DIR_NAME +
                                "' subdirectory. Expected at: " + main_config_path.string();
        throw std::runtime_error(error_msg);
    }

    // 4. Open and parse the main configuration file
    std::ifstream config_file(main_config_path);
    if (!config_file.is_open()) {
        throw std::runtime_error("Could not open configuration file: " + main_config_path.string());
    }

    nlohmann::json j;
    try {
        config_file >> j;
    } catch (const nlohmann::json::parse_error& e) {
        throw std::runtime_error("Failed to parse JSON from '" + main_config_path.string() + "': " + e.what());
    }

    // 5. Create an AppConfig instance and populate it from the parsed JSON
    AppConfig app_config;
    try {
        // Read relative paths from JSON and join them with the config directory's path
        // to create absolute paths for the other configuration files.
        std::string ip_config_relative = j.at("interval_processor_config_path").get<std::string>();
        app_config.interval_processor_config_path = (config_dir_path / ip_config_relative).string();

        std::string fv_config_relative = j.at("format_validator_config_path").get<std::string>();
        app_config.format_validator_config_path = (config_dir_path / fv_config_relative).string();

        std::string error_log_relative = j.at("error_log_path").get<std::string>();
        // Use lexically_normal() to correctly resolve relative pathing like ".."
        app_config.error_log_path = (config_dir_path / error_log_relative).lexically_normal().string();

    } catch (const nlohmann::json::exception& e) {
        // This catches errors like a missing key from .at()
        throw std::runtime_error("Configuration file is missing a required key: " + std::string(e.what()));
    }

    return app_config;
}
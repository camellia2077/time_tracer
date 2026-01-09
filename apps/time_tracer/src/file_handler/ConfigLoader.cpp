// file_handler/ConfigLoader.cpp
#include "ConfigLoader.hpp"
#include <iostream>
#include <fstream>
#include <stdexcept>
#include <nlohmann/json.hpp>

// file_handler/ConfigLoader.cpp

// Define namespace alias for convenience
namespace fs = std::filesystem;

ConfigLoader::ConfigLoader(const std::string& exe_path_str) {
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

std::string ConfigLoader::get_main_config_path() const {
    return main_config_path.string();
}

AppConfig ConfigLoader::load_configuration() {
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

    // 填充可执行文件所在的目录路径
    app_config.exe_dir_path = exe_path.string();

    try {
        // [修改] 路径拼接直接赋值给 fs::path 成员，减少 .string() 调用
        app_config.interval_processor_config_path = config_dir_path / j.at("interval_processor_config_path").get<std::string>();
        app_config.error_log_path = (config_dir_path / j.at("error_log_path").get<std::string>()).lexically_normal();

        app_config.day_typ_config_path = config_dir_path / j.at("day_typ_config_path").get<std::string>();
        app_config.month_typ_config_path = config_dir_path / j.at("month_typ_config_path").get<std::string>();
        app_config.period_typ_config_path = config_dir_path / j.at("period_typ_config_path").get<std::string>();

        app_config.day_tex_config_path = config_dir_path / j.at("day_tex_config_path").get<std::string>();
        app_config.month_tex_config_path = config_dir_path / j.at("month_tex_config_path").get<std::string>();
        app_config.period_tex_config_path = config_dir_path / j.at("period_tex_config_path").get<std::string>();

        app_config.day_md_config_path = config_dir_path / j.at("day_md_config_path").get<std::string>();
        app_config.month_md_config_path = config_dir_path / j.at("month_md_config_path").get<std::string>();
        app_config.period_md_config_path = config_dir_path / j.at("period_md_config_path").get<std::string>();


        // 检查 JSON 对象中是否存在 "export_path" 键
        if (j.contains("export_path")) {
            // 如果存在，则读取其值并赋给 app_config 的新成员
            app_config.export_path = j.at("export_path").get<std::string>();
        }
        // 如果不存在，app_config.export_path 将自然地保持为空（std::nullopt）状态

    } catch (const nlohmann::json::exception& e) {
        throw std::runtime_error("Configuration file is missing a required key: " + std::string(e.what()));
    }

    return app_config;
}
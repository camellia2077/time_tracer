// io/config/ConfigLoader.cpp
#include "io/config/ConfigLoader.hpp"
#include <iostream>
#include <fstream>
#include <stdexcept>
#include <nlohmann/json.hpp>

namespace fs = std::filesystem;

ConfigLoader::ConfigLoader(const std::string& exe_path_str) {
    try {
        exe_path = fs::canonical(fs::path(exe_path_str)).parent_path();
    } catch (const fs::filesystem_error& e) {
        throw std::runtime_error("Failed to determine executable directory: " + std::string(e.what()));
    }

    config_dir_path = exe_path / CONFIG_DIR_NAME;
    main_config_path = config_dir_path / CONFIG_FILE_NAME;
}

std::string ConfigLoader::get_main_config_path() const {
    return main_config_path.string();
}

AppConfig ConfigLoader::load_configuration() {
    if (!fs::exists(main_config_path)) {
        throw std::runtime_error("Configuration file not found: " + main_config_path.string());
    }

    std::ifstream config_file(main_config_path);
    if (!config_file.is_open()) {
        throw std::runtime_error("Could not open configuration file: " + main_config_path.string());
    }

    nlohmann::json j;
    try {
        config_file >> j;
    } catch (const nlohmann::json::parse_error& e) {
        throw std::runtime_error("Failed to parse JSON from config file: " + std::string(e.what()));
    }

    AppConfig app_config;
    app_config.exe_dir_path = exe_path.string();

    // 仅做 JSON 到 AppConfig 结构体的映射
    // 不检查子配置文件的内容
    try {
        if (j.contains("system")) {
            const auto& sys = j.at("system");
            app_config.error_log_path = (config_dir_path / sys.at("error_log").get<std::string>()).lexically_normal();
            if (sys.contains("export_root")) {
                app_config.export_path = sys.at("export_root").get<std::string>();
            }
        } else {
             throw std::runtime_error("Missing 'system' configuration block.");
        }

        if (j.contains("processing")) {
            const auto& proc = j.at("processing");
            app_config.interval_processor_config_path = config_dir_path / proc.at("interval_config").get<std::string>();
        } else {
             throw std::runtime_error("Missing 'processing' configuration block.");
        }

        if (j.contains("reports")) {
            const auto& reports = j.at("reports");
            
            auto load_report_paths = [&](const std::string& key, fs::path& day, fs::path& month, fs::path& period) {
                if (reports.contains(key)) {
                    const auto& section = reports.at(key);
                    day = config_dir_path / section.at("day").get<std::string>();
                    month = config_dir_path / section.at("month").get<std::string>();
                    period = config_dir_path / section.at("period").get<std::string>();
                }
            };

            load_report_paths("typst", app_config.day_typ_config_path, app_config.month_typ_config_path, app_config.period_typ_config_path);
            load_report_paths("latex", app_config.day_tex_config_path, app_config.month_tex_config_path, app_config.period_tex_config_path);
            load_report_paths("markdown", app_config.day_md_config_path, app_config.month_md_config_path, app_config.period_md_config_path);

        } else {
            throw std::runtime_error("Missing 'reports' configuration block.");
        }

    } catch (const nlohmann::json::exception& e) {
        throw std::runtime_error("Configuration structure error: " + std::string(e.what()));
    }

    return app_config;
}
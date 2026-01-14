// config/ConfigLoader.cpp
#include "ConfigLoader.hpp"
#include <iostream>
#include <stdexcept>
#include <nlohmann/json.hpp>

// 引入 IO 模块
#include "io/core/FileReader.hpp"
#include "io/core/FileSystemHelper.hpp"

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
    // 使用 FileSystemHelper 检查文件存在性
    if (!FileSystemHelper::exists(main_config_path)) {
        throw std::runtime_error("Configuration file not found: " + main_config_path.string());
    }

    // 使用 FileReader 读取文件内容
    std::string config_content;
    try {
        config_content = FileReader::read_content(main_config_path);
    } catch (const std::exception& e) {
        throw std::runtime_error("Could not read configuration file: " + std::string(e.what()));
    }

    nlohmann::json j;
    try {
        j = nlohmann::json::parse(config_content);
    } catch (const nlohmann::json::parse_error& e) {
        throw std::runtime_error("JSON parsing error in config.json: " + std::string(e.what()));
    }

    AppConfig app_config;
    app_config.exe_dir_path = exe_path;

    // 1. General/System Settings
    // 优先读取 "system" 节点 (匹配 config.json)，兼容旧的 "general"
    const nlohmann::json* sys_node = nullptr;
    if (j.contains("system")) {
        sys_node = &j["system"];
    } else if (j.contains("general")) {
        sys_node = &j["general"];
    }

    if (sys_node) {
        if (sys_node->contains("error_log")) {
            app_config.error_log_path = exe_path / sys_node->at("error_log").get<std::string>();
        } else {
            app_config.error_log_path = exe_path / "error.log";
        }
        
        // 兼容 export_root (config.json) 和 export_path (代码旧称)
        if (sys_node->contains("export_root")) {
             app_config.export_path = sys_node->at("export_root").get<std::string>();
        } else if (sys_node->contains("export_path")) {
             app_config.export_path = sys_node->at("export_path").get<std::string>();
        }

        // 读取 save_processed_output 配置
        if (sys_node->contains("save_processed_output")) {
            app_config.default_save_processed_output = sys_node->at("save_processed_output").get<bool>();
        }

    } else {
         app_config.error_log_path = exe_path / "error.log";
    }

    // 2. Converter / Pipeline Settings
    if (j.contains("converter")) {
        const auto& proc = j.at("converter");
        app_config.pipeline.interval_processor_config_path = config_dir_path / proc.at("interval_config").get<std::string>();
        
        // 加载 initial_top_parents (如果有)
        if (proc.contains("initial_top_parents")) {
            for (const auto& [key, val] : proc["initial_top_parents"].items()) {
                app_config.pipeline.initial_top_parents[fs::path(key)] = fs::path(val.get<std::string>());
            }
        }
    } else {
        throw std::runtime_error("Missing 'converter' configuration block."); 
    }

    // 3. Reports Settings
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

        load_report_paths("typst", 
            app_config.reports.day_typ_config_path, 
            app_config.reports.month_typ_config_path, 
            app_config.reports.period_typ_config_path
        );
        
        load_report_paths("latex", 
            app_config.reports.day_tex_config_path, 
            app_config.reports.month_tex_config_path, 
            app_config.reports.period_tex_config_path
        );
        
        load_report_paths("markdown", 
            app_config.reports.day_md_config_path, 
            app_config.reports.month_md_config_path, 
            app_config.reports.period_md_config_path
        );

    } else {
        throw std::runtime_error("Missing 'reports' configuration block.");
    }

    return app_config;
}
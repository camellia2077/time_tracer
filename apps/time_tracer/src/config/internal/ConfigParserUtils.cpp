// config/internal/ConfigParserUtils.cpp
#include "ConfigParserUtils.hpp"
#include <iostream>
#include <stdexcept>

namespace fs = std::filesystem;

namespace ConfigParserUtils {

void parse_system_settings(const nlohmann::json& j, const fs::path& exe_path, AppConfig& config) {
    const nlohmann::json* sys_node = nullptr;
    // 兼容 system 和 general 节点
    if (j.contains("system")) {
        sys_node = &j["system"];
    } else if (j.contains("general")) {
        sys_node = &j["general"];
    }

    if (sys_node) {
        // error_log
        if (sys_node->contains("error_log")) {
            config.error_log_path = exe_path / sys_node->at("error_log").get<std::string>();
        } else {
            config.error_log_path = exe_path / "error.log";
        }
        
        // export_root / export_path
        if (sys_node->contains("export_root")) {
             config.export_path = sys_node->at("export_root").get<std::string>();
        } else if (sys_node->contains("export_path")) {
             config.export_path = sys_node->at("export_path").get<std::string>();
        }

        // save_processed_output
        if (sys_node->contains("save_processed_output")) {
            config.default_save_processed_output = sys_node->at("save_processed_output").get<bool>();
        }
        
        // date_check_continuity
        if (sys_node->contains("date_check_continuity")) {
            bool check = sys_node->at("date_check_continuity").get<bool>();
            config.default_date_check_mode = check ? DateCheckMode::Continuity : DateCheckMode::None;
        }

    } else {
         config.error_log_path = exe_path / "error.log";
    }
}

void parse_pipeline_settings(const nlohmann::json& j, const fs::path& config_dir, AppConfig& config) {
    if (j.contains("converter")) {
        const auto& proc = j.at("converter");
        config.pipeline.interval_processor_config_path = config_dir / proc.at("interval_config").get<std::string>();
        
        if (proc.contains("initial_top_parents")) {
            for (const auto& [key, val] : proc["initial_top_parents"].items()) {
                config.pipeline.initial_top_parents[fs::path(key)] = fs::path(val.get<std::string>());
            }
        }
    } else {
        throw std::runtime_error("Missing 'converter' configuration block."); 
    }
}

void parse_report_paths(const nlohmann::json& j, const fs::path& config_dir, AppConfig& config) {
    if (j.contains("reports")) {
        const auto& reports = j.at("reports");
        
        // 辅助 lambda 用于简化路径读取
        auto load_paths = [&](const std::string& key, fs::path& day, fs::path& month, fs::path& period) {
            if (reports.contains(key)) {
                const auto& section = reports.at(key);
                day = config_dir / section.at("day").get<std::string>();
                month = config_dir / section.at("month").get<std::string>();
                period = config_dir / section.at("period").get<std::string>();
            }
        };

        load_paths("typst", 
            config.reports.day_typ_config_path, 
            config.reports.month_typ_config_path, 
            config.reports.period_typ_config_path
        );
        
        load_paths("latex", 
            config.reports.day_tex_config_path, 
            config.reports.month_tex_config_path, 
            config.reports.period_tex_config_path
        );
        
        load_paths("markdown", 
            config.reports.day_md_config_path, 
            config.reports.month_md_config_path, 
            config.reports.period_md_config_path
        );

    } else {
        throw std::runtime_error("Missing 'reports' configuration block.");
    }
}

} // namespace ConfigParserUtils
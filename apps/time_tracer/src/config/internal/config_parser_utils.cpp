// config/internal/config_parser_utils.cpp
#include "config_parser_utils.hpp"
#include <iostream>
#include <stdexcept>

namespace fs = std::filesystem;

namespace ConfigParserUtils {

void parse_system_settings(const toml::table& tbl, const fs::path& exe_path, AppConfig& config) {
    const toml::node_view<const toml::node> sys_node = tbl["system"];
    
    // 如果 [system] 不存在，尝试 [general]
    if (!sys_node) {
        if (tbl.contains("general")) {
            const toml::node_view<const toml::node> gen_node = tbl["general"];
             // 复用逻辑或简单复制
             // 这里为了演示直接写
             if (auto val = gen_node["error_log"].value<std::string>()) {
                config.error_log_path = exe_path / *val;
            } else {
                config.error_log_path = exe_path / "error.log";
            }
             if (auto val = gen_node["export_root"].value<std::string>()) {
                config.export_path = *val;
            }
            config.default_save_processed_output = gen_node["save_processed_output"].value_or(false);
            bool check = gen_node["date_check_continuity"].value_or(false);
            config.default_date_check_mode = check ? DateCheckMode::Continuity : DateCheckMode::None;
            return;
        }
    }

    if (sys_node) {
        if (auto val = sys_node["error_log"].value<std::string>()) {
            config.error_log_path = exe_path / *val;
        } else {
            config.error_log_path = exe_path / "error.log";
        }
        
        if (auto val = sys_node["export_root"].value<std::string>()) {
            config.export_path = *val;
        }

        config.default_save_processed_output = sys_node["save_processed_output"].value_or(false);
        
        bool check = sys_node["date_check_continuity"].value_or(false);
        config.default_date_check_mode = check ? DateCheckMode::Continuity : DateCheckMode::None;
    } else {
         config.error_log_path = exe_path / "error.log";
    }
}

void parse_pipeline_settings(const toml::table& tbl, const fs::path& config_dir, AppConfig& config) {
    if (tbl.contains("converter")) {
        const auto& proc = tbl["converter"];
        
        if (auto val = proc["interval_config"].value<std::string>()) {
            config.pipeline.interval_processor_config_path = config_dir / *val;
        } else {
             throw std::runtime_error("Missing 'interval_config' in [converter] section.");
        }
        
        if (const toml::table* parents = proc["initial_top_parents"].as_table()) {
            for (const auto& [key, val] : *parents) {
                if (auto path_str = val.value<std::string>()) {
                    config.pipeline.initial_top_parents[fs::path(key.str())] = fs::path(*path_str);
                }
            }
        }
    } else {
        throw std::runtime_error("Missing [converter] configuration block."); 
    }
}

void parse_report_paths(const toml::table& tbl, const fs::path& config_dir, AppConfig& config) {
    if (tbl.contains("reports")) {
        const auto& reports = tbl["reports"];
        
        auto load_paths = [&](const std::string& key, fs::path& day, fs::path& month, fs::path& period) {
            if (reports[key].is_table()) {
                const auto& section = reports[key];
                if (auto v = section["day"].value<std::string>()) day = config_dir / *v;
                if (auto v = section["month"].value<std::string>()) month = config_dir / *v;
                if (auto v = section["period"].value<std::string>()) period = config_dir / *v;
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
        throw std::runtime_error("Missing [reports] configuration block.");
    }
}

} // namespace ConfigParserUtils
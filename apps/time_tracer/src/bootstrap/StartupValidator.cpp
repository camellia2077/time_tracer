// bootstrap/StartupValidator.cpp
#include "StartupValidator.hpp"

#include <iostream>
#include <fstream>
#include <filesystem>
#include <vector>
#include <nlohmann/json.hpp>

// 引入核心验证外观类
#include "config/validator/facade/ConfigFacade.hpp"
// 引入颜色输出
#include "common/AnsiColors.hpp"

namespace fs = std::filesystem;

// --- 内部静态辅助函数：读取 JSON 文件 ---
static bool load_json_helper(const fs::path& path, nlohmann::json& out_json) {
    std::ifstream ifs(path);
    if (!ifs.is_open()) return false;
    try {
        ifs >> out_json;
        return true;
    } catch (...) {
        return false;
    }
}

bool StartupValidator::validate_environment(const AppConfig& config) {
    ConfigFacade validator;
    bool all_valid = true;

    // 1. 验证插件 (Plugins)
    // ---------------------------------------------------------
    fs::path plugins_dir = fs::path(config.exe_dir_path) / "plugins";
    
    // validate_plugins 内部会检查目录存在性及必要 DLL
    if (!validator.validate_plugins(plugins_dir)) {
        std::cerr << RED_COLOR << "Fatal: Plugin validation failed." << RESET_COLOR << std::endl;
        return false; // 插件缺失通常是致命错误，直接返回
    }

    // 2. 验证转换器配置 (Converter Configs)
    // ---------------------------------------------------------
    nlohmann::json main_json, mappings_json, duration_rules_json;
    
    if (load_json_helper(config.interval_processor_config_path, main_json)) {
        fs::path base_dir = config.interval_processor_config_path.parent_path();
        
        // 从主配置中读取子配置文件的相对路径
        std::string mappings_file = main_json.value("mappings_config_path", "");
        std::string duration_file = main_json.value("duration_rules_config_path", "");
        
        // 尝试加载子配置 (如果路径不为空)
        if (!mappings_file.empty()) {
            load_json_helper(base_dir / mappings_file, mappings_json);
        }
        if (!duration_file.empty()) {
            load_json_helper(base_dir / duration_file, duration_rules_json);
        }

        // [修正] 调用新的方法名 validate_converter_configs
        if (!validator.validate_converter_configs(main_json, mappings_json, duration_rules_json)) {
             std::cerr << RED_COLOR << "Error: Converter configuration is invalid." << RESET_COLOR << std::endl;
             all_valid = false;
        }
    } else {
        std::cerr << RED_COLOR << "Error: Could not load interval processor config: " << config.interval_processor_config_path << RESET_COLOR << std::endl;
        all_valid = false;
    }

    // 3. 验证报表查询配置 (Report Query Configs)
    // ---------------------------------------------------------
    std::vector<std::pair<std::string, nlohmann::json>> query_configs;
    
    // 收集所有需要验证的配置文件路径
    const std::vector<fs::path> query_config_paths = {
        config.day_tex_config_path, config.month_tex_config_path, config.period_tex_config_path,
        config.day_md_config_path, config.month_md_config_path, config.period_md_config_path,
        config.day_typ_config_path, config.month_typ_config_path, config.period_typ_config_path
    };

    // 遍历并加载
    for (const auto& p : query_config_paths) {
        if (p.empty()) continue; // 跳过空路径
        
        nlohmann::json q_json;
        if (load_json_helper(p, q_json)) {
            query_configs.push_back({p.filename().string(), q_json});
        }
    }

    if (!validator.validate_query_configs(query_configs)) {
        std::cerr << RED_COLOR << "Error: One or more report configurations are invalid." << RESET_COLOR << std::endl;
        all_valid = false;
    }

    return all_valid;
}
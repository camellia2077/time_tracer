// bootstrap/StartupValidator.cpp
#include "StartupValidator.hpp"

#include <iostream>
#include <filesystem>
#include <vector>
#include <nlohmann/json.hpp>

// 引入核心验证外观类
#include "config/validator/facade/ConfigFacade.hpp"
// 引入颜色输出
#include "common/AnsiColors.hpp"
// [新增] 引入 IO 模块
#include "io/core/FileReader.hpp"
#include "io/core/FileSystemHelper.hpp"

namespace fs = std::filesystem;

// --- 内部静态辅助函数：读取 JSON 文件 ---
static bool load_json_helper(const fs::path& path, nlohmann::json& out_json) {
    // [修改] 使用 FileSystemHelper 检查
    if (!FileSystemHelper::exists(path)) {
        // 如果文件不存在，外层调用者通常会打印 "Could not load..."，所以这里可以直接返回
        // 或者也可以在这里打印更详细的 "File not found"
        return false;
    }
    
    try {
        // [修改] 使用 FileReader 读取
        std::string content = FileReader::read_content(path);
        out_json = nlohmann::json::parse(content);
        return true;
    } catch (const std::exception& e) {
        // [修改] 捕获具体异常并打印文件名和错误详情
        std::cerr << RED_COLOR << "Error loading/parsing file [" << path.string() << "]: " 
                  << e.what() << RESET_COLOR << std::endl;
        return false;
    } catch (...) {
        // [新增] 捕获未知异常
        std::cerr << RED_COLOR << "Unknown error occurred while processing file [" 
                  << path.string() << "]" << RESET_COLOR << std::endl;
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
        return false; 
    }

    // 2. 验证核心转换器配置 (Converter Config)
    // ---------------------------------------------------------
    nlohmann::json main_json;
    if (load_json_helper(config.pipeline.interval_processor_config_path, main_json)) {
        fs::path base_dir = config.pipeline.interval_processor_config_path.parent_path();
        
        nlohmann::json mappings_json;
        if (main_json.contains("mappings_config_path")) {
            fs::path p = base_dir / main_json["mappings_config_path"].get<std::string>();
            load_json_helper(p, mappings_json);
        }

        nlohmann::json duration_rules_json;
        if (main_json.contains("duration_rules_config_path")) {
            fs::path p = base_dir / main_json["duration_rules_config_path"].get<std::string>();
            load_json_helper(p, duration_rules_json);
        }

        // [修复] 修正方法名：validate_converter_config -> validate_converter_configs
        if (!validator.validate_converter_configs(main_json, mappings_json, duration_rules_json)) {
             std::cerr << RED_COLOR << "Error: Converter configuration is invalid." << RESET_COLOR << std::endl;
             all_valid = false;
        }
    } else {
        std::cerr << RED_COLOR << "Error: Could not load interval processor config: " << config.pipeline.interval_processor_config_path << RESET_COLOR << std::endl;
        all_valid = false;
    }

    // 3. 验证报表查询配置 (Report Query Configs)
    // ---------------------------------------------------------
    std::vector<std::pair<std::string, nlohmann::json>> query_configs;
    
    const std::vector<fs::path> query_config_paths = {
        config.reports.day_tex_config_path, config.reports.month_tex_config_path, config.reports.period_tex_config_path,
        config.reports.day_md_config_path, config.reports.month_md_config_path, config.reports.period_md_config_path,
        config.reports.day_typ_config_path, config.reports.month_typ_config_path, config.reports.period_typ_config_path
    };

    for (const auto& p : query_config_paths) {
        if (p.empty()) continue; 
        
        nlohmann::json q_json;
        if (load_json_helper(p, q_json)) {
            query_configs.push_back({p.string(), q_json});
        } else {
            std::cerr << YELLOW_COLOR << "Warning: Could not load report config: " << p << RESET_COLOR << std::endl;
        }
    }

    if (!validator.validate_query_configs(query_configs)) {
        std::cerr << RED_COLOR << "Error: One or more report query configurations are invalid." << RESET_COLOR << std::endl;
        all_valid = false;
    }

    return all_valid;
}
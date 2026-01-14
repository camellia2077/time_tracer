// core/file/utils/ConverterConfigFactory.cpp
#include "ConverterConfigFactory.hpp"
#include "converter/config/JsonConverterConfigLoader.hpp"
#include "io/core/FileReader.hpp"
#include "common/AnsiColors.hpp"
#include <nlohmann/json.hpp>
#include <iostream>

namespace fs = std::filesystem;

// 将原有的 loadMergedJson 移入作为私有静态函数或匿名空间函数
static nlohmann::json loadMergedJson(const fs::path& main_config_path) {
    using json = nlohmann::json;
    json main_json;
    try {
        std::string main_content = FileReader::read_content(main_config_path);
        main_json = json::parse(main_content);
        fs::path config_dir = main_config_path.parent_path();

        if (main_json.contains("mappings_config_path")) {
            fs::path map_path = config_dir / main_json["mappings_config_path"].get<std::string>();
            std::string map_content = FileReader::read_content(map_path);
            json map_json = json::parse(map_content);
            if (map_json.contains("text_mappings")) {
                main_json["text_mappings"] = map_json["text_mappings"];
            }
        }
        
        if (main_json.contains("duration_rules_config_path")) {
            fs::path dur_path = config_dir / main_json["duration_rules_config_path"].get<std::string>();
            std::string dur_content = FileReader::read_content(dur_path);
            json dur_json = json::parse(dur_content);
            
            if (dur_json.contains("text_duration_mappings")) {
                main_json["text_duration_mappings"] = dur_json["text_duration_mappings"];
            }
            if (dur_json.contains("duration_mappings")) {
                main_json["duration_mappings"] = dur_json["duration_mappings"];
            }
        }
    } catch (const std::exception& e) {
        std::cerr << RED_COLOR << "Config Merge Error: " << e.what() << RESET_COLOR << std::endl;
        // 根据策略，这里可以选择抛出异常阻断流程
        throw; 
    }
    return main_json;
}

ConverterConfig ConverterConfigFactory::create(const fs::path& interval_config_path, 
                                             const AppConfig& app_config) {
    // 1. 加载并合并 JSON
    nlohmann::json merged_json = loadMergedJson(interval_config_path);

    // 2. 使用 Loader 填充基础配置
    ConverterConfig config;
    JsonConverterConfigLoader loader(merged_json);
    if (!loader.load(config)) {
        throw std::runtime_error("Failed to populate ConverterConfig from JSON.");
    }

    // 3. 注入运行时参数 (initial_top_parents)
    for (const auto& [path_key, path_val] : app_config.pipeline.initial_top_parents) {
        config.initial_top_parents[path_key.string()] = path_val.string();
    }

    return config;
}
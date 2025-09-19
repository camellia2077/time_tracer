// config_validator/ConfigValidator.cpp
#include "ConfigValidator.hpp"
#include "pipelines/MainConfigValidator.hpp"
#include "pipelines/MappingsConfigValidator.hpp"
#include "pipelines/DurationRulesConfigValidator.hpp"

#include <iostream>

using json = nlohmann::json;

bool ConfigValidator::validate(
    const json& main_json,
    const json& mappings_json,
    const json& duration_rules_json)
{
    // --- 1. 调用 MainConfigValidator ---
    std::string mappings_path_str, duration_rules_path_str;
    MainConfigValidator main_validator;
    if (!main_validator.validate(main_json, mappings_path_str, duration_rules_path_str)) {
        // 注意：这里的路径只用于确认main_json的完整性，不再用于打开文件。
        // 如果需要，还可以添加一步检查，确保 main_json 中声明的文件名与外部加载的文件名一致。
        return false;
    }

    // --- 2. 调用 MappingsConfigValidator ---
    MappingsConfigValidator mappings_validator;
    if (!mappings_validator.validate(mappings_json)) {
        return false;
    }

    // --- 3. 调用 DurationRulesConfigValidator ---
    DurationRulesConfigValidator duration_rules_validator;
    if (!duration_rules_validator.validate(duration_rules_json)) {
        return false;
    }

    std::cout << "[ConfigValidator] All configuration data is valid." << std::endl;
    return true;
}
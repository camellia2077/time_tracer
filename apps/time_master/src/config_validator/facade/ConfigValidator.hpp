// config_validator/facade/ConfigValidator.hpp
#ifndef CONFIG_VALIDATOR_HPP
#define CONFIG_VALIDATOR_HPP

#include <string>
#include <vector>
#include <nlohmann/json.hpp>

/**
 * @class ConfigValidator
 * @brief (Top-level Facade) 协调所有子系统验证器完成配置验证。
 */
class ConfigValidator {
public:
    /**
     * @brief 验证预处理相关的配置文件。
     */
    bool validate_preprocessing_configs(
        const nlohmann::json& main_json,
        const nlohmann::json& mappings_json,
        const nlohmann::json& duration_rules_json
    ) const;

    /**
     * @brief 验证所有查询相关的配置文件。
     */
    bool validate_query_configs(
        const std::vector<std::pair<std::string, nlohmann::json>>& query_configs
    ) const;
};

#endif // CONFIG_VALIDATOR_HPP
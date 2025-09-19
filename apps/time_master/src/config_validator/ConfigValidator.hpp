// config_validator/ConfigValidator.hpp
#ifndef CONFIG_VALIDATOR_HPP
#define CONFIG_VALIDATOR_HPP

#include <string>
#include <nlohmann/json.hpp>

/**
 * @class ConfigValidator
 * @brief (Facade) 协调多个子验证器来完成对已加载配置数据的全面验证。
 *
 * 该类不执行任何文件IO操作，它接收由外部模块加载和解析后的json对象。
 */
class ConfigValidator {
public:
    /**
     * @brief 执行所有验证检查。
     * @param main_json 已解析的主配置文件json对象。
     * @param mappings_json 已解析的映射配置文件json对象。
     * @param duration_rules_json 已解析的时长规则配置文件json对象。
     * @return 如果所有配置数据都合法，则返回 true；否则返回 false。
     */
    bool validate(
        const nlohmann::json& main_json,
        const nlohmann::json& mappings_json,
        const nlohmann::json& duration_rules_json
    );
};

#endif // CONFIG_VALIDATOR_HPP
// config_validator/queries/pipelines/QueryRules.hpp
#ifndef QUERY_CONFIG_VALIDATOR_HPP
#define QUERY_CONFIG_VALIDATOR_HPP

#include <string>
#include <nlohmann/json.hpp>

/**
 * @class QueryRules
 * @brief 负责验证单个查询配置文件（如DayTexConfig.json）的内部规则。
 */
class QueryRules {
public:
    /**
     * @brief 对给定的JSON对象执行所有查询配置的验证规则。
     * @param query_json 要验证的JSON对象。
     * @param file_name 从中加载此JSON的原始文件名，用于错误报告。
     * @return 如果配置有效，则返回 true；否则返回 false。
     */
    bool validate(const nlohmann::json& query_json, const std::string& file_name) const;

private:
    // 私有辅助函数
    bool is_valid_hex_color(const std::string& color_string) const;
};

#endif // QUERY_CONFIG_VALIDATOR_HPP
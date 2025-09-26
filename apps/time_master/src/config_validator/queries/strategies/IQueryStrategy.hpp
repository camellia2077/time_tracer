// config_validator/queries/strategies/IQueryStrategy.hpp
#ifndef I_QUERY_STRATEGY_HPP
#define I_QUERY_STRATEGY_HPP

#include <nlohmann/json.hpp>
#include <string>

/**
 * @class IQueryStrategy
 * @brief (接口) 为所有查询配置验证策略定义了一个通用接口。
 */
class IQueryStrategy {
public:
    virtual ~IQueryStrategy() = default;

    /**
     * @brief 对给定的JSON对象执行特定于策略的验证规则。
     * @param query_json 要验证的JSON对象。
     * @param file_name 从中加载此JSON的原始文件名，用于错误报告。
     * @return 如果配置有效，则返回 true；否则返回 false。
     */
    virtual bool validate(const nlohmann::json& query_json, const std::string& file_name) const = 0;
};

#endif // I_QUERY_STRATEGY_HPP
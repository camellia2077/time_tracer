// config_validator/queries/strategies/BaseStrategy.hpp
#ifndef BASE_VALIDATION_STRATEGY_HPP
#define BASE_VALIDATION_STRATEGY_HPP

#include "IQueryStrategy.hpp"

/**
 * @class BaseStrategy
 * @brief (抽象基类) 为所有具体查询验证策略提供通用的验证逻辑。
 *
 * 使用了模板方法模式。validate() 方法是模板方法，它定义了验证的骨架，
 * 将具体的 key 验证推迟到子类中实现 (validate_specific_keys)。
 */
class BaseStrategy : public IQueryStrategy {
public:
    // 模板方法，定义了验证的完整流程
    bool validate(const nlohmann::json& query_json, const std::string& file_name) const final;

    virtual ~BaseStrategy() = default;

protected:
    // 抽象方法，由每个具体的策略子类实现，用于验证各自特有的必需key
    virtual bool validate_specific_keys(const nlohmann::json& query_json, const std::string& file_name) const = 0;

private:
    // 包含了所有策略共享的通用验证规则
    bool validate_common_rules(const nlohmann::json& query_json, const std::string& file_name) const;
    bool validate_statistics_items(const nlohmann::json& query_json, const std::string& file_name) const;
    bool is_valid_hex_color(const std::string& color_string) const;
};

#endif // BASE_VALIDATION_STRATEGY_HPP
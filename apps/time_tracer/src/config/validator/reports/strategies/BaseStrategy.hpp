// config/validator/reports/strategies/BaseStrategy.hpp
#ifndef CONFIG_VALIDATOR_REPORTS_STRATEGIES_BASE_STRATEGY_HPP_
#define CONFIG_VALIDATOR_REPORTS_STRATEGIES_BASE_STRATEGY_HPP_

#include "IQueryStrategy.hpp"
#include <toml++/toml.h>

/**
 * @class BaseStrategy
 * @brief (抽象基类) 为所有具体查询验证策略提供通用的验证逻辑。
 */
class BaseStrategy : public IQueryStrategy {
public:
    // 模板方法，定义了验证的完整流程
    bool validate(const toml::table& query_config, const std::string& file_name) const final;

    virtual ~BaseStrategy() = default;

protected:
    // 抽象方法，由每个具体的策略子类实现
    virtual bool validate_specific_keys(const toml::table& query_config, const std::string& file_name) const = 0;

private:
    // 通用验证规则
    bool validate_common_rules(const toml::table& query_config, const std::string& file_name) const;
    bool validate_statistics_items(const toml::table& query_config, const std::string& file_name) const;
    bool is_valid_hex_color(const std::string& color_string) const;
};

#endif // CONFIG_VALIDATOR_REPORTS_STRATEGIES_BASE_STRATEGY_HPP_
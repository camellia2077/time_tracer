// infrastructure/config/validator/reports/strategies/i_query_strategy.hpp
#ifndef CONFIG_VALIDATOR_REPORTS_STRATEGIES_I_QUERY_STRATEGY_H_
#define CONFIG_VALIDATOR_REPORTS_STRATEGIES_I_QUERY_STRATEGY_H_

#include <toml++/toml.h>

#include <string>

/**
 * @class IQueryStrategy
 * @brief (接口) 为所有查询配置验证策略定义了一个通用接口。
 */
class IQueryStrategy {
 public:
  virtual ~IQueryStrategy() = default;

  /**
   * @brief 对给定的 TOML 表对象执行特定于策略的验证规则。
   * @param query_config 要验证的 TOML 表对象。
   * @param file_name 从中加载此配置的原始文件名，用于错误报告。
   * @return 如果配置有效，则返回 true；否则返回 false。
   */
  [[nodiscard]] virtual auto Validate(const toml::table& query_config,
                                      const std::string& file_name) const
      -> bool = 0;
};

#endif  // CONFIG_VALIDATOR_REPORTS_STRATEGIES_I_QUERY_STRATEGY_H_
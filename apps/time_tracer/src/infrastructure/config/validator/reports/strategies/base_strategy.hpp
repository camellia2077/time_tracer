// infrastructure/config/validator/reports/strategies/base_strategy.hpp
#ifndef CONFIG_VALIDATOR_REPORTS_STRATEGIES_BASE_STRATEGY_H_
#define CONFIG_VALIDATOR_REPORTS_STRATEGIES_BASE_STRATEGY_H_

#include <toml++/toml.h>

#include "infrastructure/config/validator/reports/strategies/i_query_strategy.hpp"

/**
 * @class BaseStrategy
 * @brief (抽象基类) 为所有具体查询验证策略提供通用的验证逻辑。
 */
class BaseStrategy : public IQueryStrategy {
 public:
  // 模板方法，定义了验证的完整流程
  [[nodiscard]] auto Validate(const toml::table& query_config,
                              const std::string& file_name) const -> bool final;

  ~BaseStrategy() override = default;

 protected:
  // 抽象方法，由每个具体的策略子类实现
  [[nodiscard]] virtual auto ValidateSpecificKeys(
      const toml::table& query_config, const std::string& file_name) const
      -> bool = 0;

 private:
  // 通用验证规则
  static auto ValidateCommonRules(const toml::table& query_config,
                                  const std::string& file_name) -> bool;
  static auto ValidateStatisticsItems(const toml::table& query_config,
                                      const std::string& file_name) -> bool;
  static auto IsValidHexColor(const std::string& color_string) -> bool;
};

#endif  // CONFIG_VALIDATOR_REPORTS_STRATEGIES_BASE_STRATEGY_H_

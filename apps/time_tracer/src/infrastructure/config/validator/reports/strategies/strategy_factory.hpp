// infrastructure/config/validator/reports/strategies/strategy_factory.hpp
#ifndef CONFIG_VALIDATOR_REPORTS_STRATEGIES_STRATEGY_FACTORY_H_
#define CONFIG_VALIDATOR_REPORTS_STRATEGIES_STRATEGY_FACTORY_H_

#include <memory>
#include <string>

#include "infrastructure/config/validator/reports/strategies/i_query_strategy.hpp"

/**
 * @class StrategyFactory
 * @brief (工厂) 根据文件名创建并返回适当的验证策略实例。
 */
class StrategyFactory {
 public:
  /**
   * @brief 根据文件名创建并返回一个唯一的验证策略实例。
   * @param file_name 用于决定使用哪种验证策略的文件名。
   * @return 返回一个指向 IQueryStrategy 的 unique_ptr；
   * 如果找不到匹配的策略，则返回 nullptr。
   */
  static auto CreateStrategy(const std::string& file_name)
      -> std::unique_ptr<IQueryStrategy>;
};

#endif  // CONFIG_VALIDATOR_REPORTS_STRATEGIES_STRATEGY_FACTORY_H_
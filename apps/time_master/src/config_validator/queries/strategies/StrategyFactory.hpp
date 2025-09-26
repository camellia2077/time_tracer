// config_validator/queries/strategies/StrategyFactory.hpp
#ifndef STRATEGY_FACTORY_HPP
#define STRATEGY_FACTORY_HPP

#include "IQueryStrategy.hpp"
#include <string>
#include <memory>

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
    static std::unique_ptr<IQueryStrategy> createStrategy(const std::string& file_name);
};

#endif // STRATEGY_FACTORY_HPP
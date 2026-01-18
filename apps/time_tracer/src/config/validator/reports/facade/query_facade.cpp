// config/validator/reports/facade/query_facade.cpp
#include "query_facade.hpp"
#include "config/validator/reports/strategies/strategy_factory.hpp"
#include <iostream>
#include <memory>

bool QueryFacade::validate(
    const std::vector<std::pair<std::string, toml::table>>& query_configs
) const {
    bool all_ok = true;

    if (query_configs.empty()) {
        std::cerr << "[Validator] Warning: No query configuration files were provided for validation." << std::endl;
        return true;
    }

    for (const auto& config_pair : query_configs) {
        const std::string& file_name = config_pair.first;
        const toml::table& config_tbl = config_pair.second;

        // 1. 从工厂获取合适的策略
        std::unique_ptr<IQueryStrategy> strategy = StrategyFactory::createStrategy(file_name);

        // 2. 检查策略是否存在
        if (!strategy) {
            std::cerr << "[Validator] Warning: No validation strategy found for file: " << file_name << std::endl;
            continue; 
        }

        // 3. 执行验证
        if (!strategy->validate(config_tbl, file_name)) {
            all_ok = false;
        }
    }

    if(all_ok) {
        std::cout << "[Validator] All query configuration files are valid." << std::endl;
    }

    return all_ok;
}
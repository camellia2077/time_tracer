// config_validator/reports/facade/QueryFacade.cpp
#include "QueryFacade.hpp"
#include "config_validator/reports/strategies/StrategyFactory.hpp" // [修改] 引入策略工厂
#include <iostream>
#include <memory> // [新增] 为了 std::unique_ptr

bool QueryFacade::validate(
    const std::vector<std::pair<std::string, nlohmann::json>>& query_configs
) const {
    bool all_ok = true;

    if (query_configs.empty()) {
        std::cerr << "[Validator] Warning: No query configuration files were provided for validation." << std::endl;
        return true;
    }

    // [修改] 使用新的策略模式进行验证
    for (const auto& config_pair : query_configs) {
        const std::string& file_name = config_pair.first;
        const nlohmann::json& config_json = config_pair.second;

        // 1. 从工厂获取合适的策略
        std::unique_ptr<IQueryStrategy> strategy = StrategyFactory::createStrategy(file_name);

        // 2. 检查策略是否存在
        if (!strategy) {
            std::cerr << "[Validator] Warning: No validation strategy found for file: " << file_name << std::endl;
            continue; // 或者根据需求将 all_ok 设为 false
        }

        // 3. 执行验证
        if (!strategy->validate(config_json, file_name)) {
            all_ok = false;
        }
    }

    if(all_ok) {
        std::cout << "[Validator] All query configuration files are valid." << std::endl;
    }

    return all_ok;
}
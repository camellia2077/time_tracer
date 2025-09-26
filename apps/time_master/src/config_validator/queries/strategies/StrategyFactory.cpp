// config_validator/queries/strategies/StrategyFactory.cpp
#include "StrategyFactory.hpp"

// [修改] 包含所有具体的策略类头文件，并更新路径
#include "config_validator/queries/strategies/daily/DailyMd.hpp"
#include "config_validator/queries/strategies/daily/DailyTex.hpp"
#include "config_validator/queries/strategies/daily/DailyTyp.hpp"
#include "config_validator/queries/strategies/monthly/Monthly.hpp"
#include "config_validator/queries/strategies/periodic/Periodic.hpp"

#include <memory>
#include <string>

std::unique_ptr<IQueryStrategy> StrategyFactory::createStrategy(const std::string& file_name) {
    if (file_name.find("DayMd") != std::string::npos) {
        return std::make_unique<DailyMd>();
    } else if (file_name.find("DayTex") != std::string::npos) {
        return std::make_unique<DailyTex>();
    } else if (file_name.find("DayTyp") != std::string::npos) {
        return std::make_unique<DailyTyp>();
    } else if (file_name.find("Month") != std::string::npos) {
        return std::make_unique<Monthly>();
    } else if (file_name.find("Period") != std::string::npos) {
        return std::make_unique<Periodic>();
    }

    // 如果没有找到匹配的策略，则返回空指针
    return nullptr;
}
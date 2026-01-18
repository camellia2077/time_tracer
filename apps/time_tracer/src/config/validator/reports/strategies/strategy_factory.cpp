// config/validator/reports/strategies/strategy_factory.cpp
#include "strategy_factory.hpp"

#include "config/validator/reports/strategies/daily/daily_md.hpp"
#include "config/validator/reports/strategies/daily/daily_tex.hpp"
#include "config/validator/reports/strategies/daily/daily_typ.hpp"
#include "config/validator/reports/strategies/monthly/Monthly.hpp"
#include "config/validator/reports/strategies/periodic/Periodic.hpp"

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
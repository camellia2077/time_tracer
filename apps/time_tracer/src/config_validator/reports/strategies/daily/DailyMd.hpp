// config_validator/reports/strategies/daily/DailyMd.hpp
#ifndef DAILY_MD_HPP
#define DAILY_MD_HPP

#include "config_validator/reports/strategies/BaseStrategy.hpp" // [修改] 继承自基类

class DailyMd : public BaseStrategy { // [修改]
protected:
    // [修改] 实现基类的抽象方法，只负责检查特定于 DayMd 的 key
    bool validate_specific_keys(const nlohmann::json& query_json, const std::string& file_name) const override;
};

#endif // DAILY_MD_HPP
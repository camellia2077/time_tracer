// config_validator/queries/strategies/daily/DailyTyp.hpp
#ifndef DAILY_TYP_VALIDATION_STRATEGY_HPP
#define DAILY_TYP_VALIDATION_STRATEGY_HPP

#include "config_validator/queries/strategies/BaseStrategy.hpp"

/**
 * @class DailyTyp
 * @brief (具体策略) 实现了针对 DayTyp 类型报表配置的验证逻辑。
 */
class DailyTyp : public BaseStrategy { // [修改]
protected:
    // [修改] 实现基类的抽象方法，只负责检查特定于 DayTyp 的 key
    bool validate_specific_keys(const nlohmann::json& query_json, const std::string& file_name) const override;
};

#endif // DAILY_TYP_VALIDATION_STRATEGY_HPP
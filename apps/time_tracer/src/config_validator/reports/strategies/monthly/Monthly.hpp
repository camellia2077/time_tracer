// config_validator/reports/strategies/monthly/Monthly.hpp
#ifndef MONTHLY_HPP
#define MONTHLY_HPP

#include "config_validator/reports/strategies/BaseStrategy.hpp" // [修改] 继承自基类

/**
 * @class Monthly
 * @brief (具体策略) 实现了针对月度报表配置的验证逻辑。
 */
class Monthly : public BaseStrategy { // [修改]
protected:
    // [修改] 实现基类的抽象方法，只负责检查特定于 Month 的 key
    bool validate_specific_keys(const nlohmann::json& query_json, const std::string& file_name) const override;
};

#endif // MONTHLY_HPP
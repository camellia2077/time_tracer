// config_validator/queries/strategies/periodic/Periodic.hpp
#ifndef PERIODIC_HPP
#define PERIODIC_HPP

#include "config_validator/queries/strategies/BaseStrategy.hpp." // [修改] 继承自基类

/**
 * @class Periodic
 * @brief (具体策略) 实现了针对周期性报表配置的验证逻辑。
 */
class Periodic : public BaseStrategy { // [修改]
protected:
    // [修改] 实现基类的抽象方法，只负责检查特定于 Period 的 key
    bool validate_specific_keys(const nlohmann::json& query_json, const std::string& file_name) const override;
};

#endif // PERIODIC_HPP
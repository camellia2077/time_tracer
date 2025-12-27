// config_validator/reprocessing/rules/DurationRule.hpp
#ifndef DURATION_RULE_HPP
#define DURATION_RULE_HPP

#include <nlohmann/json.hpp>

// [修改] 类名从 DurationRules 改为 DurationRule
class DurationRule {
public:
    bool validate(const nlohmann::json& duration_json) const;
};

#endif // DURATION_RULE_HPP
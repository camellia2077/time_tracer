// config_validator/reprocessing/rules/MainRule.hpp
#ifndef MAIN_RULE_HPP
#define MAIN_RULE_HPP

#include <string>
#include <nlohmann/json.hpp>

// [修改] 类名从 MainRules 改为 MainRule
class MainRule {
public:
    bool validate(const nlohmann::json& main_json, std::string& out_mappings_path, std::string& out_duration_rules_path) const;
};

#endif // MAIN_RULE_HPP
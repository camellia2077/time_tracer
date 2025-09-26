// config_validator/reprocessing/rules/MappingRule.hpp
#ifndef MAPPING_RULE_HPP
#define MAPPING_RULE_HPP

#include <nlohmann/json.hpp>

// [修改] 类名从 MappingRules 改为 MappingRule
class MappingRule {
public:
    bool validate(const nlohmann::json& mappings_json) const;
};

#endif // MAPPING_RULE_HPP
// config/validator/converter/rules/mapping_rule.hpp
#ifndef CONFIG_VALIDATOR_CONVERTER_RULES_MAPPING_RULE_HPP_
#define CONFIG_VALIDATOR_CONVERTER_RULES_MAPPING_RULE_HPP_

#include <toml++/toml.h> 

class MappingRule {
public:
    bool validate(const toml::table& mappings_tbl) const;
};

#endif // CONFIG_VALIDATOR_CONVERTER_RULES_MAPPING_RULE_HPP_
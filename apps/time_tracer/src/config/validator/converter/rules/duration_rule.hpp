// config/validator/converter/rules/duration_rule.hpp
#ifndef CONFIG_VALIDATOR_CONVERTER_RULES_DURATION_RULE_HPP_
#define CONFIG_VALIDATOR_CONVERTER_RULES_DURATION_RULE_HPP_

#include <toml++/toml.h> 

class DurationRule {
public:
    bool validate(const toml::table& duration_tbl) const;
};

#endif // CONFIG_VALIDATOR_CONVERTER_RULES_DURATION_RULE_HPP_
// config/validator/converter/rules/DurationRule.hpp
#ifndef CONFIG_VALIDATOR_CONVERTER_RULES_DURATION_RULE_HPP_
#define CONFIG_VALIDATOR_CONVERTER_RULES_DURATION_RULE_HPP_

#include <toml++/toml.h> // [修改]

class DurationRule {
public:
    // [修改] 参数类型改为 toml::table
    bool validate(const toml::table& duration_tbl) const;
};

#endif // CONFIG_VALIDATOR_CONVERTER_RULES_DURATION_RULE_HPP_
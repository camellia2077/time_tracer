// config/validator/converter/rules/main_rule.hpp
#ifndef CONFIG_VALIDATOR_CONVERTER_RULES_MAIN_RULE_HPP_
#define CONFIG_VALIDATOR_CONVERTER_RULES_MAIN_RULE_HPP_

#include <string>
#include <toml++/toml.h>

class MainRule {
public:
    bool validate(const toml::table& main_tbl, std::string& out_mappings_path, std::string& out_duration_rules_path) const;
};

#endif // CONFIG_VALIDATOR_CONVERTER_RULES_MAIN_RULE_HPP_
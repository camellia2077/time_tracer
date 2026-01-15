// config/validator/converter/rules/MainRule.hpp
#ifndef MAIN_RULE_HPP
#define MAIN_RULE_HPP

#include <string>
#include <toml++/toml.h> // [修改]

class MainRule {
public:
    // [修改] 参数类型改为 toml::table
    bool validate(const toml::table& main_tbl, std::string& out_mappings_path, std::string& out_duration_rules_path) const;
};

#endif // MAIN_RULE_HPP
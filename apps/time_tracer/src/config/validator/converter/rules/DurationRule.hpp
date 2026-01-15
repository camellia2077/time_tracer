// config/validator/converter/rules/DurationRule.hpp
#ifndef DURATION_RULE_HPP
#define DURATION_RULE_HPP

#include <toml++/toml.h> // [修改]

class DurationRule {
public:
    // [修改] 参数类型改为 toml::table
    bool validate(const toml::table& duration_tbl) const;
};

#endif // DURATION_RULE_HPP
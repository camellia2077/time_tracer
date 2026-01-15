// config/validator/converter/rules/MappingRule.hpp
#ifndef MAPPING_RULE_HPP
#define MAPPING_RULE_HPP

#include <toml++/toml.h> // [修改]

class MappingRule {
public:
    // [修改] 参数类型改为 toml::table
    bool validate(const toml::table& mappings_tbl) const;
};

#endif // MAPPING_RULE_HPP
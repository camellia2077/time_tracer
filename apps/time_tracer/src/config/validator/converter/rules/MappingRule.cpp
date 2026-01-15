// config/validator/converter/rules/MappingRule.cpp
#include "MappingRule.hpp"
#include <iostream>

// [修复] 参数类型改为 const toml::table&
bool MappingRule::validate(const toml::table& mappings_tbl) const {
    // [修复] 使用 toml++ API
    if (!mappings_tbl.contains("text_mappings") || !mappings_tbl["text_mappings"].is_table()) {
        std::cerr << "[Validator] Error: Mappings config must contain a 'text_mappings' table." << std::endl;
        return false;
    }
    return true;
}
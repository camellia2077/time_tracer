// config/validator/converter/rules/mapping_rule.cpp
#include "mapping_rule.hpp"

#include <iostream>

bool MappingRule::validate(const toml::table& mappings_tbl) {
  if (!mappings_tbl.contains("text_mappings") ||
      !mappings_tbl["text_mappings"].is_table()) {
    std::cerr << "[Validator] Error: Mappings config must contain a "
                 "'text_mappings' table."
              << std::endl;
    return false;
  }
  return true;
}
// infrastructure/config/validator/converter/rules/mapping_rule.cpp
#include "infrastructure/config/validator/converter/rules/mapping_rule.hpp"

#include <iostream>

auto MappingRule::Validate(const toml::table& mappings_tbl) -> bool {
  if (!mappings_tbl.contains("text_mappings") ||
      !mappings_tbl["text_mappings"].is_table()) {
    std::cerr << "[Validator] Error: Mappings config must contain a "
                 "'text_mappings' table."
              << std::endl;
    return false;
  }
  return true;
}
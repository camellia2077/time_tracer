// domain/types/converter_config.hpp
#ifndef DOMAIN_TYPES_CONVERTER_CONFIG_H_
#define DOMAIN_TYPES_CONVERTER_CONFIG_H_

#include <string>
#include <unordered_map>
#include <vector>

struct ConverterConfig {
  std::string remark_prefix;
  std::vector<std::string> header_order;
  std::vector<std::string> wake_keywords;

  std::string generated_sleep_project_path = "sleep_night";

  std::unordered_map<std::string, std::string> top_parent_mapping;
  std::unordered_map<std::string, std::string> text_mapping;

  std::unordered_map<std::string, std::string> initial_top_parents;
};

namespace tracer::core::domain::types {

#include "domain/detail/converter_config_contract.inc"

}  // namespace tracer::core::domain::types

#endif  // DOMAIN_TYPES_CONVERTER_CONFIG_H_

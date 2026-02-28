// domain/types/converter_config.hpp
#ifndef DOMAIN_TYPES_CONVERTER_CONFIG_H_
#define DOMAIN_TYPES_CONVERTER_CONFIG_H_

#include <map>
#include <string>
#include <unordered_map>
#include <vector>

struct DurationMappingRule {
  int less_than_minutes = 0;
  std::string value;
};

struct ConverterConfig {
  std::string remark_prefix;
  std::vector<std::string> header_order;
  std::vector<std::string> wake_keywords;

  std::string generated_sleep_project_path = "sleep_night";

  std::unordered_map<std::string, std::string> top_parent_mapping;
  std::unordered_map<std::string, std::string> text_mapping;
  std::unordered_map<std::string, std::string> text_duration_mapping;

  std::unordered_map<std::string, std::vector<DurationMappingRule>>
      duration_mappings;

  std::unordered_map<std::string, std::string> initial_top_parents;
};

#endif  // DOMAIN_TYPES_CONVERTER_CONFIG_H_

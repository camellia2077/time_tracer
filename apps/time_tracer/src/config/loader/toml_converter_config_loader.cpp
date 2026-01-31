// config/loader/toml_converter_config_loader.cpp
#include "toml_converter_config_loader.hpp"

#include <algorithm>
#include <iostream>

#include "common/ansi_colors.hpp"

TomlConverterConfigLoader::TomlConverterConfigLoader(
    const toml::table& config_table)
    : toml_source_(config_table) {}

auto TomlConverterConfigLoader::load(ConverterConfig& config) -> bool {
  try {
    // remark_prefix
    if (auto val = toml_source_["remark_prefix"].value<std::string>()) {
      config.remark_prefix = *val;
    }

    // header_order (Array of strings)
    if (const toml::array* arr = toml_source_["header_order"].as_array()) {
      config.header_order.clear();  // 清空默认值（如果有）
      for (const auto& elem : *arr) {
        if (auto s = elem.value<std::string>()) {
          config.header_order.push_back(*s);
        }
      }
    }

    // wake_keywords (Array of strings)
    if (const toml::array* arr = toml_source_["wake_keywords"].as_array()) {
      config.wake_keywords.clear();
      for (const auto& elem : *arr) {
        if (auto s = elem.value<std::string>()) {
          config.wake_keywords.push_back(*s);
        }
      }
    }

    // top_parent_mapping (Table: key -> string)
    if (const toml::table* tbl =
            toml_source_["top_parent_mapping"].as_table()) {
      for (const auto& [k, v] : *tbl) {
        if (auto s = v.value<std::string>()) {
          // [修复] 显式转换为 std::string
          config.top_parent_mapping[std::string(k.str())] = *s;
        }
      }
    }

    // text_mappings
    if (const toml::table* tbl = toml_source_["text_mappings"].as_table()) {
      for (const auto& [k, v] : *tbl) {
        if (auto s = v.value<std::string>()) {
          config.text_mapping[std::string(k.str())] = *s;
        }
      }
    }

    // text_duration_mappings
    if (const toml::table* tbl =
            toml_source_["text_duration_mappings"].as_table()) {
      for (const auto& [k, v] : *tbl) {
        if (auto s = v.value<std::string>()) {
          // [修复] 成员变量名为 text_duration_mapping (单数)
          config.text_duration_mapping[std::string(k.str())] = *s;
        }
      }
    }

    // duration_mappings (Table of Arrays of Inline Tables)
    if (const toml::table* duration_tbl =
            toml_source_["duration_mappings"].as_table()) {
      for (const auto& [event_key, rules_node] : *duration_tbl) {
        if (const toml::array* rules_arr = rules_node.as_array()) {
          // [修复] 使用正确的结构体名称 DurationMappingRule
          std::vector<DurationMappingRule> rules;

          for (const auto& rule_node : *rules_arr) {
            if (const toml::table* rule_tbl = rule_node.as_table()) {
              DurationMappingRule rule;
              rule.less_than_minutes =
                  rule_tbl->get("less_than_minutes")->value_or(0);
              rule.value = rule_tbl->get("value")->value_or("");
              rules.push_back(rule);
            }
          }
          // 排序逻辑
          std::ranges::sort(rules,
                            [](const DurationMappingRule& a,
                               const DurationMappingRule& b) -> bool {
                              return a.less_than_minutes < b.less_than_minutes;
                            });

          // [修复] 直接赋值给 map
          config.duration_mappings[std::string(event_key.str())] = rules;
        }
      }
    }

  } catch (const std::exception& e) {
    std::cerr << RED_COLOR
              << "Error parsing configuration TOML via Loader: " << e.what()
              << RESET_COLOR << std::endl;
    return false;
  }
  return true;
}
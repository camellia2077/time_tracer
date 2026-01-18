// config/loader/toml_converter_config_loader.cpp
#include "toml_converter_config_loader.hpp"
#include "common/ansi_colors.hpp"
#include <iostream>
#include <algorithm>

TomlConverterConfigLoader::TomlConverterConfigLoader(const toml::table& config_table)
    : toml_source_(config_table) {}

bool TomlConverterConfigLoader::load(ConverterConfig& config) {
    try {
        // remark_prefix
        if (auto val = toml_source_["remark_prefix"].value<std::string>()) {
            config.setRemarkPrefix(*val);
        }
        
        // header_order (Array of strings)
        if (const toml::array* arr = toml_source_["header_order"].as_array()) {
            std::vector<std::string> vec;
            for (const auto& elem : *arr) {
                if (auto s = elem.value<std::string>()) vec.push_back(*s);
            }
            config.setHeaderOrder(vec);
        }
        
        // wake_keywords (Array of strings)
        if (const toml::array* arr = toml_source_["wake_keywords"].as_array()) {
            std::vector<std::string> vec;
            for (const auto& elem : *arr) {
                if (auto s = elem.value<std::string>()) vec.push_back(*s);
            }
            config.setWakeKeywords(vec);
        }
        
        // top_parent_mapping (Table: key -> string)
        if (const toml::table* tbl = toml_source_["top_parent_mapping"].as_table()) {
            std::unordered_map<std::string, std::string> mapping;
            for (const auto& [k, v] : *tbl) {
                if (auto s = v.value<std::string>()) mapping[k.str()] = *s;
            }
            config.setTopParentMapping(mapping);
        }

        // text_mappings
        if (const toml::table* tbl = toml_source_["text_mappings"].as_table()) {
            std::unordered_map<std::string, std::string> mapping;
            for (const auto& [k, v] : *tbl) {
                if (auto s = v.value<std::string>()) mapping[k.str()] = *s;
            }
            config.setTextMapping(mapping);
        }
        
        // text_duration_mappings
        if (const toml::table* tbl = toml_source_["text_duration_mappings"].as_table()) {
            std::unordered_map<std::string, std::string> mapping;
            for (const auto& [k, v] : *tbl) {
                if (auto s = v.value<std::string>()) mapping[k.str()] = *s;
            }
            config.setTextDurationMapping(mapping);
        }

        // duration_mappings (Table of Arrays of Inline Tables)
        // TOML 结构: 
        // [duration_mappings]
        // rest = [ { less_than_minutes = 10, value = "rest_short" }, ... ]
        if (const toml::table* duration_tbl = toml_source_["duration_mappings"].as_table()) {
            std::unordered_map<std::string, std::vector<DurationRule>> loaded_rules;
            
            for (const auto& [event_key, rules_node] : *duration_tbl) {
                if (const toml::array* rules_arr = rules_node.as_array()) {
                    std::vector<DurationRule> rules;
                    for (const auto& rule_node : *rules_arr) {
                        if (const toml::table* rule_tbl = rule_node.as_table()) {
                             DurationRule rule;
                             rule.less_than_minutes = rule_tbl->get("less_than_minutes")->value_or(0);
                             rule.value = rule_tbl->get("value")->value_or("");
                             rules.push_back(rule);
                        }
                    }
                    // 排序逻辑
                    std::sort(rules.begin(), rules.end(), [](const DurationRule& a, const DurationRule& b) {
                        return a.less_than_minutes < b.less_than_minutes;
                    });
                    loaded_rules[event_key.str()] = rules;
                }
            }
            config.setDurationMappings(loaded_rules);
        }

    } catch (const std::exception& e) {
        std::cerr << RED_COLOR << "Error parsing configuration TOML via Loader: " << e.what() << RESET_COLOR << std::endl;
        return false;
    }
    return true;
}
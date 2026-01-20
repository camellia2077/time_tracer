// config/loader/converter_config_loader.cpp
#include "converter_config_loader.hpp"
#include "toml_loader_utils.hpp" // 使用 read_toml
#include "common/ansi_colors.hpp"
#include "io/core/file_system_helper.hpp"
#include <iostream>
#include <stdexcept>
#include <algorithm>

namespace fs = std::filesystem;
using namespace TomlLoaderUtils;

void ConverterConfigLoader::merge_toml_table(toml::table& target, const toml::table& source) {
    for (const auto& [key, val] : source) {
        if (target.contains(key)) {
            if (target[key].is_table() && val.is_table()) {
                merge_toml_table(*target[key].as_table(), *val.as_table());
            } else {
                target.insert_or_assign(key, val);
            }
        } else {
            target.insert(key, val);
        }
    }
}

toml::table ConverterConfigLoader::load_merged_toml(const fs::path& main_config_path) {
    if (!FileSystemHelper::exists(main_config_path)) {
        throw std::runtime_error("Converter config file not found: " + main_config_path.string());
    }

    toml::table main_tbl = read_toml(main_config_path);
    fs::path config_dir = main_config_path.parent_path();

    // 合并 mappings_config_path
    if (auto path_node = main_tbl["mappings_config_path"].value<std::string>()) {
        fs::path map_path = config_dir / *path_node;
        if (FileSystemHelper::exists(map_path)) {
            auto mapping_tbl = read_toml(map_path);
            if (mapping_tbl.contains("text_mappings")) {
                if (!main_tbl.contains("text_mappings")) main_tbl.insert("text_mappings", toml::table{});
                merge_toml_table(*main_tbl["text_mappings"].as_table(), *mapping_tbl["text_mappings"].as_table());
            }
            if (mapping_tbl.contains("text_duration_mappings")) { 
                if (!main_tbl.contains("text_duration_mappings")) main_tbl.insert("text_duration_mappings", toml::table{});
                merge_toml_table(*main_tbl["text_duration_mappings"].as_table(), *mapping_tbl["text_duration_mappings"].as_table());
            }
        }
    }

    // 合并 duration_rules_config_path
    if (auto path_node = main_tbl["duration_rules_config_path"].value<std::string>()) {
        fs::path rule_path = config_dir / *path_node;
        if (FileSystemHelper::exists(rule_path)) {
            auto rules_tbl = read_toml(rule_path);
            if (rules_tbl.contains("duration_mappings")) {
                if (!main_tbl.contains("duration_mappings")) main_tbl.insert("duration_mappings", toml::table{});
                merge_toml_table(*main_tbl["duration_mappings"].as_table(), *rules_tbl["duration_mappings"].as_table());
            }
            if (rules_tbl.contains("text_duration_mappings")) {
                if (!main_tbl.contains("text_duration_mappings")) main_tbl.insert("text_duration_mappings", toml::table{});
                merge_toml_table(*main_tbl["text_duration_mappings"].as_table(), *rules_tbl["text_duration_mappings"].as_table());
            }
        }
    }

    return main_tbl;
}

void ConverterConfigLoader::parse_toml_to_struct(const toml::table& tbl, ConverterConfig& config) {
    // 1. 基础配置
    if (auto val = tbl["remark_prefix"].value<std::string>()) {
        config.remark_prefix = *val;
    }

    if (const toml::array* arr = tbl["header_order"].as_array()) {
        for (const auto& elem : *arr) {
            if (auto s = elem.value<std::string>()) config.header_order.push_back(*s);
        }
    }

    if (const toml::array* arr = tbl["wake_keywords"].as_array()) {
        for (const auto& elem : *arr) {
            if (auto s = elem.value<std::string>()) config.wake_keywords.push_back(*s);
        }
    }

    // 2. [新增] 自动生成活动的配置
    if (const toml::table* gen_tbl = tbl["generated_activities"].as_table()) {
        if (auto val = gen_tbl->get("sleep_project_path")->value<std::string>()) {
            config.generated_sleep_project_path = *val;
        }
    }

    // 3. 映射表
    auto load_map = [&](const std::string& key, std::unordered_map<std::string, std::string>& target) {
        if (const toml::table* map_tbl = tbl[key].as_table()) {
            for (const auto& [k, v] : *map_tbl) {
                if (auto s = v.value<std::string>()) target[std::string(k.str())] = *s;
            }
        }
    };

    load_map("top_parent_mapping", config.top_parent_mapping);
    load_map("text_mappings", config.text_mapping);
    load_map("text_duration_mappings", config.text_duration_mapping);

    // 4. 时长规则
    if (const toml::table* duration_tbl = tbl["duration_mappings"].as_table()) {
        for (const auto& [event_key, rules_node] : *duration_tbl) {
            if (const toml::array* rules_arr = rules_node.as_array()) {
                std::vector<DurationMappingRule> rules;
                for (const auto& rule_node : *rules_arr) {
                    if (const toml::table* rule_tbl = rule_node.as_table()) {
                        DurationMappingRule rule;
                        rule.less_than_minutes = rule_tbl->get("less_than_minutes")->value_or(0);
                        rule.value = rule_tbl->get("value")->value_or("");
                        rules.push_back(rule);
                    }
                }
                std::sort(rules.begin(), rules.end(), [](const DurationMappingRule& a, const DurationMappingRule& b) {
                    return a.less_than_minutes < b.less_than_minutes;
                });
                config.duration_mappings[std::string(event_key.str())] = rules;
            }
        }
    }
}

ConverterConfig ConverterConfigLoader::load_from_file(const fs::path& main_config_path) {
    toml::table merged_toml = load_merged_toml(main_config_path);
    ConverterConfig config;
    parse_toml_to_struct(merged_toml, config);
    return config;
}
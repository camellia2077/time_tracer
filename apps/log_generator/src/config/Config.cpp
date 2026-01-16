// config/Config.cpp
#include "Config.hpp"
#include <iostream>

// --- Public Method Implementation ---

std::optional<TomlConfigData> ConfigLoader::load_from_content(
    const std::string& settings_content,
    const std::string& mapping_content) {
    
    TomlConfigData config_data;
    
    // 解析顺序：映射 -> 设置
    if (!_parse_mapping_keys(mapping_content, config_data) || 
        !_parse_settings(settings_content, config_data)) { 
        return std::nullopt;
    }
    
    return config_data;
}

bool ConfigLoader::_parse_mapping_keys(const std::string& content, TomlConfigData& config_data) {
    try {
        toml::table tbl = toml::parse(content);
        
        if (auto mapping_node = tbl.get_as<toml::table>("text_mappings")) {
            for (auto&& [key, value] : *mapping_node) {
                // [设计说明] 收集 Key 作为生成器的活动种子
                config_data.mapped_activities.push_back(std::string(key.str()));
            }
            return true;
        } else {
            std::cerr << "Error: [text_mappings] section not found in mapping config.\n";
            return false;
        }
    } catch (const toml::parse_error& e) {
        std::cerr << "TOML Parse Error in mapping config: " << e.description() << "\n";
        return false;
    }
}


bool ConfigLoader::_parse_settings(const std::string& content, TomlConfigData& config_data) {
    try {
        toml::table tbl = toml::parse(content);
        _load_daily_remarks(tbl, config_data);
        _load_activity_remarks(tbl, config_data);
        _load_wake_keywords(tbl, config_data);
        return true;
    } catch (const toml::parse_error& e) {
        std::cerr << "TOML Parse Error in settings config: " << e.description() << "\n";
        return false;
    }
}


void ConfigLoader::_load_daily_remarks(const toml::table& data, TomlConfigData& config) {
    if (auto node = data.get_as<toml::table>("daily_remarks")) {
        DailyRemarkConfig remarks;
        remarks.prefix = (*node)["prefix"].value_or("");
        remarks.generation_chance = (*node)["generation_chance"].value_or(0.5);
        
        // [新增] 读取 max_lines 配置
        // toml++ 的整数通常读取为 int64_t，我们需要转换并验证
        int64_t lines = (*node)["max_lines"].value_or(1);
        // 确保至少为 1
        remarks.max_lines = static_cast<int>(lines < 1 ? 1 : lines);

        if (auto arr = (*node).get_as<toml::array>("contents")) {
            for (auto&& v : *arr) remarks.contents.push_back(v.value_or(""));
            config.remarks.emplace(remarks);
        }
    }
}

void ConfigLoader::_load_activity_remarks(const toml::table& data, TomlConfigData& config) {
    if (auto node = data.get_as<toml::table>("activity_remarks")) {
        ActivityRemarkConfig ar;
        ar.generation_chance = (*node)["generation_chance"].value_or(0.5);
        if (auto arr = (*node).get_as<toml::array>("contents")) {
            for (auto&& v : *arr) ar.contents.push_back(v.value_or(""));
            config.activity_remarks.emplace(ar);
        }
    }
}

void ConfigLoader::_load_wake_keywords(const toml::table& data, TomlConfigData& config) {
    if (auto arr = data.get_as<toml::array>("wake_keywords")) {
        for (auto&& v : *arr) config.wake_keywords.push_back(v.value_or(""));
    } else {
        config.wake_keywords.push_back("起床");
    }
}
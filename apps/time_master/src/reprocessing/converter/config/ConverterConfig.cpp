// reprocessing/converter/config/ConverterConfig.cpp
#include "ConverterConfig.hpp"
#include "common/AnsiColors.hpp"
#include <nlohmann/json.hpp>
#include <fstream>
#include <iostream>
#include <filesystem> // [新增]
#include <algorithm>

namespace fs = std::filesystem;

// [新增] 辅助函数，用于加载一个 JSON 文件
static bool loadJsonFile(const fs::path& file_path, nlohmann::json& out_json) {
    std::ifstream ifs(file_path);
    if (!ifs.is_open()) {
        std::cerr << RED_COLOR << "Error: Could not open config file: " << file_path << RESET_COLOR << std::endl;
        return false;
    }
    try {
        ifs >> out_json;
    } catch (const std::exception& e) {
        std::cerr << RED_COLOR << "Error parsing JSON from " << file_path << ": " << e.what() << RESET_COLOR << std::endl;
        return false;
    }
    return true;
}


bool ConverterConfig::load(const std::string& main_config_path) {
    fs::path main_path = main_config_path;
    fs::path config_dir = main_path.parent_path();

    nlohmann::json main_json;
    if (!loadJsonFile(main_path, main_json)) {
        return false;
    }

    try {
        // 从主配置文件加载基本设置
        if (main_json.contains("remark_prefix")) remark_prefix_ = main_json["remark_prefix"].get<std::string>();
        if (main_json.contains("header_order")) header_order_ = main_json["header_order"].get<std::vector<std::string>>();
        if (main_json.contains("wake_keywords")) wake_keywords_ = main_json["wake_keywords"].get<std::vector<std::string>>();
        if (main_json.contains("topParentMapping")) topParentMapping_ = main_json["topParentMapping"].get<std::unordered_map<std::string, std::string>>();

        // [核心修改] 加载拆分出去的配置文件
        if (main_json.contains("mappings_config_path")) {
            fs::path mappings_path = config_dir / main_json["mappings_config_path"].get<std::string>();
            nlohmann::json mappings_json;
            if (loadJsonFile(mappings_path, mappings_json)) {
                if (mappings_json.contains("text_mappings")) {
                    text_mapping_ = mappings_json["text_mappings"].get<std::unordered_map<std::string, std::string>>();
                }
            }
        }
        
        if (main_json.contains("duration_rules_config_path")) {
            fs::path duration_path = config_dir / main_json["duration_rules_config_path"].get<std::string>();
            nlohmann::json duration_json;
            if (loadJsonFile(duration_path, duration_json)) {
                if (duration_json.contains("text_duration_mappings")) {
                     text_duration_mapping_ = duration_json["text_duration_mappings"].get<std::unordered_map<std::string, std::string>>();
                }
                if (duration_json.contains("duration_mappings") && duration_json["duration_mappings"].is_object()) {
                    const auto& duration_rules_json = duration_json["duration_mappings"];
                    for (auto& [event_key, rules_json] : duration_rules_json.items()) {
                        std::vector<DurationRule> rules;
                        for (const auto& rule_json : rules_json) {
                            rules.push_back({
                                rule_json.at("less_than_minutes").get<int>(),
                                rule_json.at("value").get<std::string>()
                            });
                        }
                        std::sort(rules.begin(), rules.end(), [](const DurationRule& a, const DurationRule& b) {
                            return a.less_than_minutes < b.less_than_minutes;
                        });
                        duration_mappings_[event_key] = rules;
                    }
                }
            }
        }

    } catch (const std::exception& e) {
        std::cerr << RED_COLOR << "Error processing content from configuration files: " << e.what() << RESET_COLOR << std::endl;
        return false;
    }
    return true;
}


const std::string& ConverterConfig::getRemarkPrefix() const { return remark_prefix_; }
const std::vector<std::string>& ConverterConfig::getHeaderOrder() const { return header_order_; }
const std::unordered_map<std::string, std::string>& ConverterConfig::getTextMapping() const { return text_mapping_; }
const std::unordered_map<std::string, std::string>& ConverterConfig::getTextDurationMapping() const { return text_duration_mapping_; }
const std::unordered_map<std::string, std::vector<DurationRule>>& ConverterConfig::getDurationMappings() const { return duration_mappings_; }
const std::vector<std::string>& ConverterConfig::getWakeKeywords() const { return wake_keywords_; }
const std::unordered_map<std::string, std::string>& ConverterConfig::getTopParentMapping() const { return topParentMapping_; }
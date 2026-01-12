// converter/config/ConverterConfig.cpp
#include "ConverterConfig.hpp"
#include <iostream>
#include <algorithm>
#include "common/AnsiColors.hpp"

bool ConverterConfig::load(const nlohmann::json& config_json) {
    try {
        if (config_json.contains("remark_prefix")) 
            remark_prefix_ = config_json["remark_prefix"].get<std::string>();
        
        if (config_json.contains("header_order")) 
            header_order_ = config_json["header_order"].get<std::vector<std::string>>();
        
        if (config_json.contains("wake_keywords")) 
            wake_keywords_ = config_json["wake_keywords"].get<std::vector<std::string>>();
        
        if (config_json.contains("top_parent_mapping")) 
            topParentMapping_ = config_json["top_parent_mapping"].get<std::unordered_map<std::string, std::string>>();

        // 假设传入的 JSON 已经合并了 text_mappings
        if (config_json.contains("text_mappings")) {
            text_mapping_ = config_json["text_mappings"].get<std::unordered_map<std::string, std::string>>();
        }
        
        // 假设传入的 JSON 已经合并了 duration_mappings
        if (config_json.contains("text_duration_mappings")) {
             text_duration_mapping_ = config_json["text_duration_mappings"].get<std::unordered_map<std::string, std::string>>();
        }

        if (config_json.contains("duration_mappings") && config_json["duration_mappings"].is_object()) {
            const auto& duration_rules_json = config_json["duration_mappings"];
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

    } catch (const std::exception& e) {
        std::cerr << RED_COLOR << "Error parsing configuration JSON: " << e.what() << RESET_COLOR << std::endl;
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
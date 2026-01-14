// converter/config/JsonConverterConfigLoader.cpp
#include "JsonConverterConfigLoader.hpp"
#include "common/AnsiColors.hpp"
#include <iostream>
#include <algorithm>

JsonConverterConfigLoader::JsonConverterConfigLoader(const nlohmann::json& config_json)
    : json_source_(config_json) {}

bool JsonConverterConfigLoader::load(ConverterConfig& config) {
    try {
        if (json_source_.contains("remark_prefix")) 
            config.setRemarkPrefix(json_source_["remark_prefix"].get<std::string>());
        
        if (json_source_.contains("header_order")) 
            config.setHeaderOrder(json_source_["header_order"].get<std::vector<std::string>>());
        
        if (json_source_.contains("wake_keywords")) 
            config.setWakeKeywords(json_source_["wake_keywords"].get<std::vector<std::string>>());
        
        if (json_source_.contains("top_parent_mapping")) 
            config.setTopParentMapping(json_source_["top_parent_mapping"].get<std::unordered_map<std::string, std::string>>());

        // 处理 text_mappings
        if (json_source_.contains("text_mappings")) {
            config.setTextMapping(json_source_["text_mappings"].get<std::unordered_map<std::string, std::string>>());
        }
        
        // 处理 text_duration_mappings
        if (json_source_.contains("text_duration_mappings")) {
             config.setTextDurationMapping(json_source_["text_duration_mappings"].get<std::unordered_map<std::string, std::string>>());
        }

        // 处理 duration_mappings (包含复杂的排序逻辑)
        if (json_source_.contains("duration_mappings") && json_source_["duration_mappings"].is_object()) {
            std::unordered_map<std::string, std::vector<DurationRule>> loaded_rules;
            const auto& duration_rules_json = json_source_["duration_mappings"];
            
            for (auto& [event_key, rules_json] : duration_rules_json.items()) {
                std::vector<DurationRule> rules;
                for (const auto& rule_json : rules_json) {
                    rules.push_back({
                        rule_json.at("less_than_minutes").get<int>(),
                        rule_json.at("value").get<std::string>()
                    });
                }
                // 排序逻辑保持不变
                std::sort(rules.begin(), rules.end(), [](const DurationRule& a, const DurationRule& b) {
                    return a.less_than_minutes < b.less_than_minutes;
                });
                loaded_rules[event_key] = rules;
            }
            config.setDurationMappings(loaded_rules);
        }

    } catch (const std::exception& e) {
        std::cerr << RED_COLOR << "Error parsing configuration JSON via Loader: " << e.what() << RESET_COLOR << std::endl;
        return false;
    }
    return true;
}
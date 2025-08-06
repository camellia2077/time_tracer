#include "common/pch.h"
// reprocessing/input_transfer/internal/ConverterConfig.cpp
#include <fstream>
#include <iostream>
#include <algorithm>
#include <nlohmann/json.hpp>

#include "ConverterConfig.h"
#include "common/common_utils.h"

bool ConverterConfig::load(const std::string& filepath) {
    std::ifstream config_ifs(filepath);
    if (!config_ifs.is_open()) {
        std::cerr << RED_COLOR << "Error: Could not open mapping file: " << filepath << RESET_COLOR << std::endl;
        return false;
    }

    nlohmann::json j;
    try {
        config_ifs >> j;
    } catch (const std::exception& e) {
        std::cerr << RED_COLOR << "Error parsing config JSON: " << e.what() << RESET_COLOR << std::endl;
        return false;
    }

    try {
        if (j.contains("remark_prefix")) remark_prefix_ = j["remark_prefix"].get<std::string>();
        if (j.contains("header_order")) header_order_ = j["header_order"].get<std::vector<std::string>>();
        if (j.contains("text_mappings")) text_mapping_ = j["text_mappings"].get<std::unordered_map<std::string, std::string>>();
        if (j.contains("text_duration_mappings")) text_duration_mapping_ = j["text_duration_mappings"].get<std::unordered_map<std::string, std::string>>();

        if (j.contains("duration_mappings") && j["duration_mappings"].is_object()) {
            const auto& duration_json = j["duration_mappings"];
            for (auto& [event_key, rules_json] : duration_json.items()) {
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
        std::cerr << RED_COLOR << "Error processing content from " << filepath << ": " << e.what() << RESET_COLOR << std::endl;
        return false;
    }
    return true;
}

const std::string& ConverterConfig::getRemarkPrefix() const { return remark_prefix_; }
const std::vector<std::string>& ConverterConfig::getHeaderOrder() const { return header_order_; }
const std::unordered_map<std::string, std::string>& ConverterConfig::getTextMapping() const { return text_mapping_; }
const std::unordered_map<std::string, std::string>& ConverterConfig::getTextDurationMapping() const { return text_duration_mapping_; }
const std::unordered_map<std::string, std::vector<DurationRule>>& ConverterConfig::getDurationMappings() const { return duration_mappings_; }
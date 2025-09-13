// reprocessing/validator/source_txt/pipelines/SourceValidatorConfig.cpp
#include "SourceValidatorConfig.hpp"
#include "common/AnsiColors.hpp"
#include <fstream>
#include <iostream>

SourceValidatorConfig::SourceValidatorConfig(const std::string& config_filename) {
    _load(config_filename);
}

void SourceValidatorConfig::_load(const std::string& config_filename) {
    std::ifstream ifs(config_filename);
    if (!ifs.is_open()) {
        std::cerr << RED_COLOR << "Error: Could not open source validator config file: " << config_filename << RESET_COLOR << std::endl;
        return;
    }
    try {
        nlohmann::json j;
        ifs >> j;
        if (j.contains("remark_prefix")) {
            remark_prefix_ = j["remark_prefix"].get<std::string>();
        }
        if (j.contains("text_mappings") && j["text_mappings"].is_object()) {
            for (auto& [key, value] : j["text_mappings"].items()) {
                valid_event_keywords_.insert(key);
            }
        }
        if (j.contains("text_duration_mappings") && j["text_duration_mappings"].is_object()) {
            for (auto& [key, value] : j["text_duration_mappings"].items()) {
                valid_event_keywords_.insert(key);
            }
        }
        if (j.contains("wake_keywords") && j["wake_keywords"].is_array()) {
            for (const auto& keyword : j["wake_keywords"]) {
                wake_keywords_.insert(keyword.get<std::string>());
            }
        }
    } catch (const std::exception& e) {
        std::cerr << RED_COLOR << "Error processing source validator config JSON: " << e.what() << RESET_COLOR << std::endl;
    }
}

const std::string& SourceValidatorConfig::get_remark_prefix() const { return remark_prefix_; }
const std::unordered_set<std::string>& SourceValidatorConfig::get_valid_event_keywords() const { return valid_event_keywords_; }
const std::unordered_set<std::string>& SourceValidatorConfig::get_wake_keywords() const { return wake_keywords_; }
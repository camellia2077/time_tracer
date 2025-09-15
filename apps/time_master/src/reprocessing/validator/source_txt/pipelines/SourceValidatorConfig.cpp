// reprocessing/validator/source_txt/pipelines/SourceValidatorConfig.cpp
#include "SourceValidatorConfig.hpp"
#include "common/AnsiColors.hpp"
#include <fstream>
#include <iostream>
#include <filesystem> 

namespace fs = std::filesystem;

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

SourceValidatorConfig::SourceValidatorConfig(const std::string& config_filename) {
    _load(config_filename);
}

void SourceValidatorConfig::_load(const std::string& main_config_path) {
    fs::path main_path = main_config_path;
    fs::path config_dir = main_path.parent_path();

    nlohmann::json main_json;
    if (!loadJsonFile(main_path, main_json)) {
        return;
    }

    try {

        if (main_json.contains("remark_prefix")) {
            remark_prefix_ = main_json["remark_prefix"].get<std::string>();
        }
        if (main_json.contains("wake_keywords") && main_json["wake_keywords"].is_array()) {
            for (const auto& keyword : main_json["wake_keywords"]) {
                wake_keywords_.insert(keyword.get<std::string>());
            }
        }
        if (main_json.contains("mappings_config_path")) {
            fs::path mappings_path = config_dir / main_json["mappings_config_path"].get<std::string>();
            nlohmann::json mappings_json;
            if (loadJsonFile(mappings_path, mappings_json) && mappings_json.contains("text_mappings")) {
                for (auto& [key, value] : mappings_json["text_mappings"].items()) {
                    valid_event_keywords_.insert(key);
                }
            }
        }
        
        if (main_json.contains("duration_rules_config_path")) {
            fs::path duration_path = config_dir / main_json["duration_rules_config_path"].get<std::string>();
            nlohmann::json duration_json;
            if (loadJsonFile(duration_path, duration_json) && duration_json.contains("text_duration_mappings")) {
                 for (auto& [key, value] : duration_json["text_duration_mappings"].items()) {
                    valid_event_keywords_.insert(key);
                }
            }
        }

    } catch (const std::exception& e) {
        std::cerr << RED_COLOR << "Error processing source validator config JSON: " << e.what() << RESET_COLOR << std::endl;
    }
}

const std::string& SourceValidatorConfig::get_remark_prefix() const { return remark_prefix_; }
const std::unordered_set<std::string>& SourceValidatorConfig::get_valid_event_keywords() const { return valid_event_keywords_; }
const std::unordered_set<std::string>& SourceValidatorConfig::get_wake_keywords() const { return wake_keywords_; }
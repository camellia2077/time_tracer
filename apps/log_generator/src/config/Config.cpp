// config/Config.cpp
#include "Config.h"
#include <iostream>
#include <fstream>
#include <stdexcept>

using json = nlohmann::json;

// --- Public Method Implementation ---

std::optional<JsonConfigData> ConfigLoader::load_json_configurations(const std::string& activities_filename, const std::string& settings_filename) {
    JsonConfigData config_data;

    if (!_load_activities(activities_filename, config_data)) {
        return std::nullopt; // 如果活动加载失败，则提前退出
    }

    if (!_load_settings(settings_filename, config_data)) {
        return std::nullopt; // 如果其他设置加载失败，则提前退出
    }
    
    return config_data;
}


// --- Private Helper Methods Implementation ---

bool ConfigLoader::_parse_json_from_file(const std::string& filename, json& json_data) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open configuration file '" << filename << "'.\n";
        return false;
    }

    try {
        json_data = json::parse(file);
    } catch (const json::parse_error& e) {
        std::cerr << "Error: Failed to parse JSON from '" << filename << "'. Detail: " << e.what() << '\n';
        return false;
    }
    return true;
}


bool ConfigLoader::_load_activities(const std::string& filename, JsonConfigData& config_data) {
    json activities_data;
    if (!_parse_json_from_file(filename, activities_data)) {
        return false;
    }

    try {
        if (activities_data.contains("common_activities") && activities_data["common_activities"].is_array() && !activities_data["common_activities"].empty()) {
            config_data.activities = activities_data["common_activities"].get<std::vector<std::string>>();
            std::cout << "Successfully loaded " << config_data.activities.size() << " activities from '" << filename << "'.\n";
            return true;
        } else {
            std::cerr << "Error: JSON file '" << filename << "' must contain a non-empty 'common_activities' array.\n";
            return false;
        }
    } catch (const json::type_error& e) {
        std::cerr << "Error: JSON type error in '" << filename << "'. Detail: " << e.what() << '\n';
        return false;
    }
}


bool ConfigLoader::_load_settings(const std::string& filename, JsonConfigData& config_data) {
    json settings_json;
    if (!_parse_json_from_file(filename, settings_json)) {
        return false;
    }

    try {
        _load_daily_remarks(settings_json, filename, config_data);
        _load_activity_remarks(settings_json, filename, config_data);
        _load_wake_keywords(settings_json, filename, config_data);
    } catch (const json::type_error& e) {
        std::cerr << "Error: JSON type error in '" << filename << "'. Detail: " << e.what() << '\n';
        return false;
    } catch (const std::exception& e) {
        std::cerr << "Error: An unexpected error occurred while processing '" << filename << "'. Detail: " << e.what() << '\n';
        return false;
    }
    return true;
}


void ConfigLoader::_load_daily_remarks(const json& data, const std::string& filename, JsonConfigData& config) {
    if (data.contains("daily_remarks") && data["daily_remarks"].is_object()) {
        const auto& remarks_json = data["daily_remarks"];
        DailyRemarkConfig remarks;
        bool prefix_ok = remarks_json.contains("prefix") && remarks_json["prefix"].is_string();
        bool contents_ok = remarks_json.contains("contents") && remarks_json["contents"].is_array() && !remarks_json["contents"].empty();

        if (prefix_ok && contents_ok) {
            remarks.prefix = remarks_json["prefix"].get<std::string>();
            remarks.contents = remarks_json["contents"].get<std::vector<std::string>>();
            
            if (remarks_json.contains("generation_chance") && remarks_json["generation_chance"].is_number()) {
                double chance = remarks_json["generation_chance"].get<double>();
                if (chance >= 0.0 && chance <= 1.0) {
                    remarks.generation_chance = chance;
                } else {
                    std::cerr << "Warning: 'generation_chance' in '" << filename << "' must be between 0.0 and 1.0. Using default.\n";
                }
            }
            config.remarks.emplace(remarks);
            std::cout << "Successfully loaded " << remarks.contents.size() << " daily remarks.\n";
        } else {
            std::cerr << "Warning: 'daily_remarks' in '" << filename << "' is incomplete. This feature will be disabled.\n";
        }
    }
}


void ConfigLoader::_load_activity_remarks(const json& data, const std::string& filename, JsonConfigData& config) {
    if (data.contains("activity_remarks") && data["activity_remarks"].is_object()) {
        const auto& ar_json = data["activity_remarks"];
        if (ar_json.contains("contents") && ar_json["contents"].is_array() && !ar_json["contents"].empty()) {
            ActivityRemarkConfig activity_remarks;
            activity_remarks.contents = ar_json["contents"].get<std::vector<std::string>>();
            if (ar_json.contains("generation_chance") && ar_json["generation_chance"].is_number()) {
                activity_remarks.generation_chance = ar_json["generation_chance"].get<double>();
            }
            config.activity_remarks.emplace(activity_remarks);
            std::cout << "Successfully loaded " << activity_remarks.contents.size() << " activity remarks.\n";
        } else {
            std::cerr << "Warning: 'activity_remarks' in '" << filename << "' is missing a non-empty 'contents' array. This feature will be disabled.\n";
        }
    }
}


void ConfigLoader::_load_wake_keywords(const json& data, const std::string& filename, JsonConfigData& config) {
    if (data.contains("wake_keywords") && data["wake_keywords"].is_array() && !data["wake_keywords"].empty()) {
        config.wake_keywords = data["wake_keywords"].get<std::vector<std::string>>();
        std::cout << "Successfully loaded " << config.wake_keywords.size() << " wake keywords.\n";
    } else {
        std::cerr << "Warning: 'wake_keywords' array not found or is empty in '" << filename << "'. Using default '起床'.\n";
        config.wake_keywords.push_back("起床");
    }
}
#include "Config.h"

#include <nlohmann/json.hpp>
#include <iostream>
#include <fstream>
#include <stdexcept>

using json = nlohmann::json;

namespace ConfigLoader {

    std::optional<JsonConfigData> load_json_configurations(const std::string& activities_filename, const std::string& remarks_filename) {
        JsonConfigData config_data;

        // 1. Load activities
        std::ifstream activities_file(activities_filename);
        if (!activities_file.is_open()) {
            std::cerr << "Error: Could not open configuration file '" << activities_filename << "'.\n";
            return std::nullopt;
        }

        try {
            json activities_data = json::parse(activities_file);
            activities_file.close();

            if (activities_data.contains("common_activities") && activities_data["common_activities"].is_array() && !activities_data["common_activities"].empty()) {
                config_data.activities = activities_data["common_activities"].get<std::vector<std::string>>();
                std::cout << "Successfully loaded " << config_data.activities.size() << " activities from '" << activities_filename << "'.\n";
            } else {
                std::cerr << "Error: JSON file '" << activities_filename << "' must contain a non-empty 'common_activities' array.\n";
                return std::nullopt;
            }
        }
        catch (const json::parse_error& e) {
            std::cerr << "Error: Failed to parse JSON from '" << activities_filename << "'. Detail: " << e.what() << '\n';
            if (activities_file.is_open()) activities_file.close();
            return std::nullopt;
        }

        // 2. Load remarks and other configurations
        std::ifstream remarks_file(remarks_filename);
        if (!remarks_file.is_open()) {
            std::cerr << "Error: Could not open configuration file '" << remarks_filename << "'.\n";
            return std::nullopt;
        }

        try {
            json data = json::parse(remarks_file);
            remarks_file.close();

            if (data.contains("daily_remarks") && data["daily_remarks"].is_object()) {
                const auto& remarks_json = data["daily_remarks"];
                DailyRemarkConfig remarks;
                bool prefix_ok = false;
                bool contents_ok = false;

                if (remarks_json.contains("prefix") && remarks_json["prefix"].is_string()) {
                    remarks.prefix = remarks_json["prefix"].get<std::string>();
                    prefix_ok = true;
                } else {
                    std::cerr << "Warning: 'daily_remarks' object in '" << remarks_filename << "' is missing a 'prefix' string. This feature will be disabled.\n";
                }

                if (remarks_json.contains("contents") && remarks_json["contents"].is_array() && !remarks_json["contents"].empty()) {
                    remarks.contents = remarks_json["contents"].get<std::vector<std::string>>();
                    contents_ok = true;
                } else {
                    std::cerr << "Warning: 'daily_remarks' object in '" << remarks_filename << "' is missing a non-empty 'contents' array. This feature will be disabled.\n";
                }

                if (remarks_json.contains("generation_chance") && remarks_json["generation_chance"].is_number()) {
                    double chance = remarks_json["generation_chance"].get<double>();
                    if (chance >= 0.0 && chance <= 1.0) {
                        remarks.generation_chance = chance;
                    } else {
                        std::cerr << "Warning: 'generation_chance' in '" << remarks_filename << "' must be between 0.0 and 1.0. Using default of " << remarks.generation_chance << ".\n";
                    }
                }

                if (prefix_ok && contents_ok) {
                    config_data.remarks.emplace(remarks);
                    std::cout << "Successfully loaded " << remarks.contents.size() << " daily remarks with a " << (remarks.generation_chance * 100) << "% generation chance.\n";
                }
            }

            // 新增：加载活动备注
            if (data.contains("activity_remarks") && data["activity_remarks"].is_object()) {
                const auto& activity_remarks_json = data["activity_remarks"];
                ActivityRemarkConfig activity_remarks;
                if (activity_remarks_json.contains("contents") && activity_remarks_json["contents"].is_array() && !activity_remarks_json["contents"].empty()) {
                    activity_remarks.contents = activity_remarks_json["contents"].get<std::vector<std::string>>();
                    
                    if (activity_remarks_json.contains("generation_chance") && activity_remarks_json["generation_chance"].is_number()) {
                        activity_remarks.generation_chance = activity_remarks_json["generation_chance"].get<double>();
                    }
                    config_data.activity_remarks.emplace(activity_remarks);
                     std::cout << "Successfully loaded " << activity_remarks.contents.size() << " activity remarks with a " << (activity_remarks.generation_chance * 100) << "% generation chance.\n";
                } else {
                     std::cerr << "Warning: 'activity_remarks' object in '" << remarks_filename << "' is missing a non-empty 'contents' array. This feature will be disabled.\n";
                }
            }
            
            // 新增：加载起床关键词
            if (data.contains("wake_keywords") && data["wake_keywords"].is_array() && !data["wake_keywords"].empty()) {
                config_data.wake_keywords = data["wake_keywords"].get<std::vector<std::string>>();
                std::cout << "Successfully loaded " << config_data.wake_keywords.size() << " wake keywords.\n";
            } else {
                std::cerr << "Warning: 'wake_keywords' array not found or is empty in '" << remarks_filename << "'. Using default '起床'.\n";
                config_data.wake_keywords.push_back("起床");
            }
        }
        catch (const json::parse_error& e) {
            std::cerr << "Error: Failed to parse JSON from '" << remarks_filename << "'. Detail: " << e.what() << '\n';
            if (remarks_file.is_open()) remarks_file.close();
            return std::nullopt;
        }
        catch (const json::type_error& e) {
            std::cerr << "Error: JSON type error in '" << remarks_filename << "'. Detail: " << e.what() << '\n';
            if (remarks_file.is_open()) remarks_file.close();
            return std::nullopt;
        }
        catch (const std::exception& e) {
            std::cerr << "Error: An unexpected error occurred while processing '" << remarks_filename << "'. Detail: " << e.what() << '\n';
            if (remarks_file.is_open()) remarks_file.close();
            return std::nullopt;
        }
        
        return config_data;
    }

}
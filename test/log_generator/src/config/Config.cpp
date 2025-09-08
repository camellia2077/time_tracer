#include "Config.h"

#include <nlohmann/json.hpp>
#include <iostream>
#include <fstream>
#include <stdexcept>

using json = nlohmann::json;

namespace ConfigLoader {

    std::optional<JsonConfigData> load_json_configurations(const std::string& json_filename) {
        std::ifstream f(json_filename);
        if (!f.is_open()) {
            std::cerr << "Error: Could not open configuration file '" << json_filename << "'.\n";
            return std::nullopt;
        }

        try {
            json data = json::parse(f);
            f.close();

            JsonConfigData config_data;

            if (data.contains("common_activities") && data["common_activities"].is_array() && !data["common_activities"].empty()) {
                config_data.activities = data["common_activities"].get<std::vector<std::string>>();
                std::cout << "Successfully loaded " << config_data.activities.size() << " activities from '" << json_filename << "'.\n";
            } else {
                std::cerr << "Error: JSON file '" << json_filename << "' must contain a non-empty 'common_activities' array.\n";
                return std::nullopt;
            }

            if (data.contains("daily_remarks") && data["daily_remarks"].is_object()) {
                const auto& remarks_json = data["daily_remarks"];
                DailyRemarkConfig remarks;
                bool prefix_ok = false;
                bool contents_ok = false;

                if (remarks_json.contains("prefix") && remarks_json["prefix"].is_string()) {
                    remarks.prefix = remarks_json["prefix"].get<std::string>();
                    prefix_ok = true;
                } else {
                    std::cerr << "Warning: 'daily_remarks' object in '" << json_filename << "' is missing a 'prefix' string. This feature will be disabled.\n";
                }

                if (remarks_json.contains("contents") && remarks_json["contents"].is_array() && !remarks_json["contents"].empty()) {
                    remarks.contents = remarks_json["contents"].get<std::vector<std::string>>();
                    contents_ok = true;
                } else {
                    std::cerr << "Warning: 'daily_remarks' object in '" << json_filename << "' is missing a non-empty 'contents' array. This feature will be disabled.\n";
                }

                if (remarks_json.contains("generation_chance") && remarks_json["generation_chance"].is_number()) {
                    double chance = remarks_json["generation_chance"].get<double>();
                    if (chance >= 0.0 && chance <= 1.0) {
                        remarks.generation_chance = chance;
                    } else {
                        std::cerr << "Warning: 'generation_chance' in '" << json_filename << "' must be between 0.0 and 1.0. Using default of " << remarks.generation_chance << ".\n";
                    }
                }

                if (prefix_ok && contents_ok) {
                    config_data.remarks.emplace(remarks);
                    std::cout << "Successfully loaded " << remarks.contents.size() << " daily remarks with a " << (remarks.generation_chance * 100) << "% generation chance.\n";
                }
            }

            return config_data;
        }
        catch (const json::parse_error& e) {
            std::cerr << "Error: Failed to parse JSON from '" << json_filename << "'. Detail: " << e.what() << '\n';
            if (f.is_open()) f.close();
            return std::nullopt;
        }
        catch (const json::type_error& e) {
            std::cerr << "Error: JSON type error in '" << json_filename << "'. Detail: " << e.what() << '\n';
            if (f.is_open()) f.close();
            return std::nullopt;
        }
        catch (const std::exception& e) {
            std::cerr << "Error: An unexpected error occurred while processing '" << json_filename << "'. Detail: " << e.what() << '\n';
            if (f.is_open()) f.close();
            return std::nullopt;
        }
    }

}
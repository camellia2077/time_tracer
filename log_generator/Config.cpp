#include "Config.h"
#include "Utils.h" // 需要用到 Utils 中的工具函数声明
#include <nlohmann/json.hpp>
#include <iostream>
#include <fstream>
#include <stdexcept>

using json = nlohmann::json;

namespace ConfigLoader {

    std::optional<JsonConfigData> load_json_configurations(const std::string& json_filename) {
        std::ifstream f(json_filename);
        if (!f.is_open()) {
            std::cerr << Utils::ConsoleColors::red << "Error: Could not open configuration file '" << json_filename << "'." << Utils::ConsoleColors::reset << '\n';
            return std::nullopt;
        }

        try {
            json data = json::parse(f);
            f.close();

            JsonConfigData config_data;

            // 1. Load "common_activities" (mandatory)
            if (data.contains("common_activities") && data["common_activities"].is_array() && !data["common_activities"].empty()) {
                config_data.activities = data["common_activities"].get<std::vector<std::string>>();
                std::cout << Utils::ConsoleColors::green << "Successfully loaded " << Utils::ConsoleColors::reset << config_data.activities.size() << " activities from '" << json_filename << "'." << '\n';
            } else {
                std::cerr << Utils::ConsoleColors::red << "Error: " << Utils::ConsoleColors::reset << "JSON file '" << json_filename << "' must contain a non-empty 'common_activities' array." << '\n';
                return std::nullopt;
            }

            // 2. Load "daily_remarks" (optional)
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
                    std::cout << Utils::ConsoleColors::green << "Successfully loaded " << Utils::ConsoleColors::reset << remarks.contents.size() << " daily remarks with a " << (remarks.generation_chance * 100) << "% generation chance." << '\n';
                }
            }

            return config_data;
        }
        // 【修正】补上缺失的 catch 块
        catch (const json::parse_error& e) {
            std::cerr << Utils::ConsoleColors::red << "Error: " << Utils::ConsoleColors::reset << "Failed to parse JSON from '" << json_filename << "'. Detail: " << e.what() << '\n';
            if (f.is_open()) f.close();
            return std::nullopt;
        }
        catch (const json::type_error& e) {
            std::cerr << Utils::ConsoleColors::red << "Error: " << Utils::ConsoleColors::reset << "JSON type error in '" << json_filename << "'. Detail: " << e.what() << '\n';
            if (f.is_open()) f.close();
            return std::nullopt;
        }
        catch (const std::exception& e) {
            std::cerr << Utils::ConsoleColors::red << "Error: " << Utils::ConsoleColors::reset << "An unexpected error occurred while processing '" << json_filename << "'. Detail: " << e.what() << '\n';
            if (f.is_open()) f.close();
            return std::nullopt;
        }
    }

    // 【修正】将 parse_arguments 函数移动到正确的位置
    std::optional<Config> parse_arguments(int argc, char* argv[]) {
        if (argc != 4) {
            Utils::print_usage(argv[0]);
            return std::nullopt;
        }

        Config config;
        try {
            config.start_year = std::stoi(argv[1]);
            config.end_year = std::stoi(argv[2]);
            config.items_per_day = std::stoi(argv[3]);
        }
        catch (const std::invalid_argument&) {
            std::cerr << Utils::ConsoleColors::red << "Error: Invalid argument. All arguments must be integers." << Utils::ConsoleColors::reset << '\n';
            Utils::print_usage(argv[0]);
            return std::nullopt;
        }
        catch (const std::out_of_range&) {
            std::cerr << Utils::ConsoleColors::red << "Error: Argument out of range." << Utils::ConsoleColors::reset << '\n';
            Utils::print_usage(argv[0]);
            return std::nullopt;
        }

        if (config.start_year <= 0 || config.end_year <= 0 || config.items_per_day <= 0) {
            std::cerr << Utils::ConsoleColors::red << "Error: Years and <items_per_day> must be positive integers." << Utils::ConsoleColors::reset << '\n';
            Utils::print_usage(argv[0]);
            return std::nullopt;
        }

        if (config.end_year < config.start_year) {
            std::cerr << Utils::ConsoleColors::red << "Error: <end_year> cannot be earlier than <start_year>." << Utils::ConsoleColors::reset << '\n';
            Utils::print_usage(argv[0]);
            return std::nullopt;
        }

        return config;
    }
}
// config_validator/queries/pipelines/QueryConfigValidator.cpp
#include "QueryConfigValidator.hpp"
#include <iostream>
#include <set>
#include <regex>
#include <string>

using json = nlohmann::json;

// Using a regular expression to validate hexadecimal color codes
bool QueryConfigValidator::is_valid_hex_color(const std::string& color_string) const {
    static const std::regex hex_color_regex(R"(^#[0-9a-fA-F]{6}$)");
    return std::regex_match(color_string, hex_color_regex);
}

bool QueryConfigValidator::validate(const json& query_json, const std::string& file_name) const {
    bool all_valid = true;

    // --- [CORE FIX] ---
    // Define separate sets of required keys for each report type. This is the main fix.
    const std::set<std::string> daily_keys = {
        "date_label", "total_time_label", "status_label", "sleep_label", "exercise_label", 
        "getup_time_label", "remark_label", "statistics_label", "all_activities_label", 
        "sleep_time_label", "activity_remark_label", "activity_connector", 
        "anaerobic_time_label", "cardio_time_label", "grooming_time_label"
    };

    const std::set<std::string> monthly_keys = {
        "report_title", "actual_days_label", "total_time_label", "invalid_format_message"
    };

    const std::set<std::string> periodic_keys = {
        "report_title_prefix", "report_title_days", "report_title_date_separator", 
        "total_time_label", "actual_days_label", "invalid_days_message"
    };

    // Rule 1: Check for the presence of required keys based on file name
    if (file_name.find("Day") != std::string::npos) {
        for (const auto& key : daily_keys) {
            if (!query_json.contains(key)) {
                std::cerr << "[Validator] Error in " << file_name << ": Missing daily report key '" << key << "'." << std::endl;
                all_valid = false;
            }
        }
        // Day-specific file checks
        if (file_name.find("Tex") != std::string::npos && !query_json.contains("no_records_message")) {
             std::cerr << "[Validator] Error in " << file_name << ": Missing key 'no_records_message'." << std::endl;
             all_valid = false;
        } else if (file_name.find("Md") != std::string::npos && !query_json.contains("no_records")) {
             std::cerr << "[Validator] Error in " << file_name << ": Missing key 'no_records'." << std::endl;
             all_valid = false;
        }

    } else if (file_name.find("Month") != std::string::npos) {
        for (const auto& key : monthly_keys) {
            if (!query_json.contains(key)) {
                std::cerr << "[Validator] Error in " << file_name << ": Missing monthly report key '" << key << "'." << std::endl;
                all_valid = false;
            }
        }
    } else if (file_name.find("Period") != std::string::npos) {
        for (const auto& key : periodic_keys) {
            if (!query_json.contains(key)) {
                std::cerr << "[Validator] Error in " << file_name << ": Missing periodic report key '" << key << "'." << std::endl;
                all_valid = false;
            }
        }
    }

    if (!all_valid) {
        return false; // Stop validation if required keys are missing
    }

    // Further checks for non-string values and formats
    const std::set<std::string> numeric_keys = {
        "base_font_size", "report_title_font_size", "category_title_font_size",
        "margin_in", "margin_top_cm", "margin_bottom_cm", "margin_left_cm", "margin_right_cm",
        "list_top_sep_pt", "list_item_sep_ex", "line_spacing_em"
    };

    for (auto it = query_json.begin(); it != query_json.end(); ++it) {
        const std::string& key = it.key();
        const auto& value = it.value();

        if (value.is_null()) {
            std::cerr << "[Validator] Error in " << file_name << ": Key '" << key << "' has a null value." << std::endl;
            all_valid = false;
        } else if (numeric_keys.count(key) && !value.is_number()) {
            std::cerr << "[Validator] Error in " << file_name << ": Key '" << key << "' must have a numeric value." << std::endl;
            all_valid = false;
        } else if (key == "keyword_colors") {
            if (!value.is_object()) {
                 std::cerr << "[Validator] Error in " << file_name << ": 'keyword_colors' must be an object." << std::endl;
                 all_valid = false;
            } else {
                for (auto color_it = value.begin(); color_it != value.end(); ++color_it) {
                    if (!color_it.value().is_string() || !is_valid_hex_color(color_it.value().get<std::string>())) {
                        std::cerr << "[Validator] Error in " << file_name << ": Color for '" << color_it.key()
                                  << "' is not a valid hex string (e.g., #RRGGBB)." << std::endl;
                        all_valid = false;
                    }
                }
            }
        } else if (value.is_string() && value.get<std::string>().empty()) {
            std::cerr << "[Validator] Error in " << file_name << ": Key '" << key << "' has an empty string value." << std::endl;
            all_valid = false;
        }
    }

    return all_valid;
}
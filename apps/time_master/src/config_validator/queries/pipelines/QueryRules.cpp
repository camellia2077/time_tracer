// config_validator/queries/pipelines/QueryRules.cpp
#include "QueryRules.hpp"
#include <iostream>
#include <set>
#include <regex>
#include <string>

using json = nlohmann::json;

// Using a regular expression to validate hexadecimal color codes
bool QueryRules::is_valid_hex_color(const std::string& color_string) const {
    static const std::regex hex_color_regex(R"(^#[0-9a-fA-F]{6}$)");
    return std::regex_match(color_string, hex_color_regex);
}

bool QueryRules::validate(const json& query_json, const std::string& file_name) const {
    bool all_valid = true;

    // --- [核心修改] 为不同类型的日报配置文件定义独立的验证规则 ---
    const std::set<std::string> daily_md_keys = {
        "title_prefix", "date_label", "total_time_label", "status_label", "sleep_label", "exercise_label",
        "getup_time_label", "remark_label", "statistics_label", "all_activities_label",
        "activity_remark_label", "activity_connector", "statistics_items", "no_records"
    };

    const std::set<std::string> daily_tex_keys = {
        "report_title", "date_label", "total_time_label", "status_label", "sleep_label", "exercise_label",
        "getup_time_label", "remark_label", "no_records_message", "statistics_label",
        "all_activities_label", "activity_remark_label", "activity_connector", "keyword_colors",
        "statistics_items"
    };

    const std::set<std::string> daily_typ_keys = {
        "title_prefix", "date_label", "total_time_label", "status_label", "sleep_label",
        "getup_time_label", "remark_label", "exercise_label", "no_records", "statistics_label",
        "all_activities_label", "activity_remark_label", "activity_connector", "keyword_colors",
        "statistics_items"
    };

    const std::set<std::string> monthly_keys = {
        "report_title", "actual_days_label", "total_time_label", "invalid_format_message"
    };

    const std::set<std::string> periodic_keys = {
        "report_title_prefix", "report_title_days", "report_title_date_separator",
        "total_time_label", "actual_days_label", "invalid_days_message"
    };

    // --- [核心修改] 根据文件名应用不同的验证规则 ---
    if (file_name.find("DayMd") != std::string::npos) {
        for (const auto& key : daily_md_keys) {
            if (!query_json.contains(key)) {
                std::cerr << "[Validator] Error in " << file_name << ": Missing daily report key '" << key << "'." << std::endl;
                all_valid = false;
            }
        }
    } else if (file_name.find("DayTex") != std::string::npos) {
        for (const auto& key : daily_tex_keys) {
            if (!query_json.contains(key)) {
                std::cerr << "[Validator] Error in " << file_name << ": Missing daily report key '" << key << "'." << std::endl;
                all_valid = false;
            }
        }
    } else if (file_name.find("DayTyp") != std::string::npos) {
        for (const auto& key : daily_typ_keys) {
            if (!query_json.contains(key)) {
                std::cerr << "[Validator] Error in " << file_name << ": Missing daily report key '" << key << "'." << std::endl;
                all_valid = false;
            }
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
    
    // [核心修改] 将 total_exercise_time 加入验证
    if (file_name.find("DayMd") != std::string::npos || file_name.find("DayTex") != std::string::npos || file_name.find("DayTyp") != std::string::npos) {
        const auto& stats_items = query_json["statistics_items"];
        if (!stats_items.is_object()) {
            std::cerr << "[Validator] Error in " << file_name << ": 'statistics_items' must be an object." << std::endl;
            all_valid = false;
        } else {
            // [新增] 为日报验证 statistics_items 是否包含所有必需的键
            const std::set<std::string> required_stats_keys = {
                "sleep_time", "total_exercise_time", "anaerobic_time", "cardio_time",
                "grooming_time", "recreation_time", "zhihu_time", "bilibili_time", "douyin_time"
            };

            for(const auto& required_key : required_stats_keys) {
                if(stats_items.find(required_key) == stats_items.end()) {
                    std::cerr << "[Validator] Error in " << file_name << ": 'statistics_items' is missing required key '" << required_key << "'." << std::endl;
                    all_valid = false;
                }
            }

            for (const auto& [key, item] : stats_items.items()) {
                if (!item.is_object()) {
                    std::cerr << "[Validator] Error in " << file_name << ": Item '" << key << "' in 'statistics_items' must be an object." << std::endl;
                    all_valid = false;
                    continue;
                }
                if (!item.contains("label") || !item["label"].is_string()) {
                    std::cerr << "[Validator] Error in " << file_name << ": Item '" << key << "' in 'statistics_items' must have a 'label' of type string." << std::endl;
                    all_valid = false;
                }
                if (!item.contains("show") || !item["show"].is_boolean()) {
                    std::cerr << "[Validator] Error in " << file_name << ": Item '" << key << "' in 'statistics_items' must have a 'show' of type boolean." << std::endl;
                    all_valid = false;
                }
            }
        }
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
            // "remark" is allowed to be empty
            if (key != "remark") {
                std::cerr << "[Validator] Error in " << file_name << ": Key '" << key << "' has an empty string value." << std::endl;
                all_valid = false;
            }
        }
    }

    return all_valid;
}
// config_validator/reports/strategies/BaseStrategy.cpp
#include "BaseStrategy.hpp"
#include <iostream>
#include <set>
#include <regex>
#include <string>

using json = nlohmann::json;

// --- 模板方法实现 ---
bool BaseStrategy::validate(const json& query_json, const std::string& file_name) const {
    // 步骤1：执行子类实现的特定key验证
    if (!validate_specific_keys(query_json, file_name)) {
        return false;
    }
    // 步骤2：执行通用的验证规则
    if (!validate_common_rules(query_json, file_name)) {
        return false;
    }
    // (可选) 步骤3：对日报类型执行 statistics_items 的验证
    if (file_name.find("Day") != std::string::npos) {
        if (!validate_statistics_items(query_json, file_name)) {
            return false;
        }
    }
    return true;
}

// --- 通用验证逻辑实现 (从旧的 QueryRules.cpp 迁移而来) ---
bool BaseStrategy::validate_common_rules(const json& query_json, const std::string& file_name) const {
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
            return false;
        }
        if (numeric_keys.count(key) && !value.is_number()) {
            std::cerr << "[Validator] Error in " << file_name << ": Key '" << key << "' must have a numeric value." << std::endl;
            return false;
        }
        if (key == "keyword_colors") {
            if (!value.is_object()) {
                 std::cerr << "[Validator] Error in " << file_name << ": 'keyword_colors' must be an object." << std::endl;
                 return false;
            }
            for (auto color_it = value.begin(); color_it != value.end(); ++color_it) {
                if (!color_it.value().is_string() || !is_valid_hex_color(color_it.value().get<std::string>())) {
                    std::cerr << "[Validator] Error in " << file_name << ": Color for '" << color_it.key()
                              << "' is not a valid hex string (e.g., #RRGGBB)." << std::endl;
                    return false;
                }
            }
        }
        if (value.is_string() && value.get<std::string>().empty() && key != "remark") {
            std::cerr << "[Validator] Error in " << file_name << ": Key '" << key << "' has an empty string value." << std::endl;
            return false;
        }
    }
    return true;
}

bool BaseStrategy::validate_statistics_items(const json& query_json, const std::string& file_name) const {
    if (!query_json.contains("statistics_items")) return true; // 如果没有这个字段，则跳过

    const auto& stats_items = query_json["statistics_items"];
    if (!stats_items.is_object()) {
        std::cerr << "[Validator] Error in " << file_name << ": 'statistics_items' must be an object." << std::endl;
        return false;
    }

    const std::set<std::string> required_stats_keys = {
        "sleep_time", "total_exercise_time", "anaerobic_time", "cardio_time",
        "grooming_time", "recreation_time", "zhihu_time", "bilibili_time", "douyin_time"
    };

    for(const auto& required_key : required_stats_keys) {
        if(stats_items.find(required_key) == stats_items.end()) {
            std::cerr << "[Validator] Error in " << file_name << ": 'statistics_items' is missing required key '" << required_key << "'." << std::endl;
            return false;
        }
    }

    for (const auto& [key, item] : stats_items.items()) {
        if (!item.is_object() || !item.contains("label") || !item.contains("show")) {
            std::cerr << "[Validator] Error in " << file_name << ": Item '" << key << "' in 'statistics_items' must be an object with 'label' and 'show' keys." << std::endl;
            return false;
        }
        if (!item["label"].is_string() || !item["show"].is_boolean()) {
            std::cerr << "[Validator] Error in " << file_name << ": In 'statistics_items', '" << key << "' must have a string 'label' and a boolean 'show'." << std::endl;
            return false;
        }
    }
    return true;
}

bool BaseStrategy::is_valid_hex_color(const std::string& color_string) const {
    static const std::regex hex_color_regex(R"(^#[0-9a-fA-F]{6}$)");
    return std::regex_match(color_string, hex_color_regex);
}
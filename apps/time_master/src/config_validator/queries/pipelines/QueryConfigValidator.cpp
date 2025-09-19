// config_validator/queries/pipelines/QueryConfigValidator.cpp
#include "QueryConfigValidator.hpp"
#include <iostream>
#include <set>
#include <regex>

using json = nlohmann::json;

// 使用正则表达式验证十六进制颜色代码
bool QueryConfigValidator::is_valid_hex_color(const std::string& color_string) const {
    static const std::regex hex_color_regex(R"(^#[0-9a-fA-F]{6}$)");
    return std::regex_match(color_string, hex_color_regex);
}

bool QueryConfigValidator::validate(const json& query_json, const std::string& file_name) const {
    // 需要进行数字类型检查的键
    const std::set<std::string> numeric_keys = {
        "BaseFontSize", "ReportTitleFontSize", "CategoryTitleFontSize",
        "Margin_in", "Margin_top_cm", "Margin_bottom_cm", "Margin_left_cm", "Margin_right_cm",
        "ListTopSep_pt", "ListItemSep_ex", "LineSpacing_em"
    };

    bool all_valid = true;

    for (auto it = query_json.begin(); it != query_json.end(); ++it) {
        const std::string& key = it.key();
        const auto& value = it.value();

        // 规则3：检查值是否为空（null）
        if (value.is_null()) {
            std::cerr << "[Validator] Error in " << file_name << ": Key '" << key << "' has a null value." << std::endl;
            all_valid = false;
            continue;
        }

        // 规则1：检查数字类型
        if (numeric_keys.count(key)) {
            if (!value.is_number()) {
                std::cerr << "[Validator] Error in " << file_name << ": Key '" << key << "' must have a numeric (integer or float) value." << std::endl;
                all_valid = false;
            }
        }
        // 规则2：检查颜色格式
        else if (key == "KeywordColors") {
            if (!value.is_object()) {
                std::cerr << "[Validator] Error in " << file_name << ": Key 'KeywordColors' must be an object." << std::endl;
                all_valid = false;
                continue;
            }
            for (auto color_it = value.begin(); color_it != value.end(); ++color_it) {
                if (!color_it.value().is_string() || !is_valid_hex_color(color_it.value().get<std::string>())) {
                    std::cerr << "[Validator] Error in " << file_name << ": Color value for '" << color_it.key()
                              << "' is not a valid hex color string (e.g., #RRGGBB)." << std::endl;
                    all_valid = false;
                }
            }
        }
        // 规则3：检查其他键的值是否为空字符串
        else {
            if (value.is_string() && value.get<std::string>().empty()) {
                std::cerr << "[Validator] Error in " << file_name << ": Key '" << key << "' has an empty string value." << std::endl;
                all_valid = false;
            }
        }
    }

    return all_valid;
}
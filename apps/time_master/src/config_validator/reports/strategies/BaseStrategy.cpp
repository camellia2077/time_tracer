// config_validator/reports/strategies/BaseStrategy.cpp
#include "BaseStrategy.hpp"
#include <iostream>
#include <set>
#include <regex>
#include <string>

using json = nlohmann::json;

// [新增] 静态辅助函数：递归验证统计项数组
static bool validate_stats_recursive(const json& items_array, const std::string& file_name, const std::string& context = "root") {
    if (!items_array.is_array()) {
        std::cerr << "[Validator] Error in " << file_name << ": 'statistics_items' (or sub_items) in " << context << " must be an array." << std::endl;
        return false;
    }

    for (size_t i = 0; i < items_array.size(); ++i) {
        const auto& item = items_array[i];
        std::string current_context = context + "[" + std::to_string(i) + "]";

        if (!item.is_object()) {
            std::cerr << "[Validator] Error in " << file_name << ": Item " << current_context << " must be an object." << std::endl;
            return false;
        }

        // 1. 验证必需字段: label (string), show (bool)
        if (!item.contains("label") || !item["label"].is_string()) {
            std::cerr << "[Validator] Error in " << file_name << ": Item " << current_context << " must contain a string 'label'." << std::endl;
            return false;
        }
        if (!item.contains("show") || !item["show"].is_boolean()) {
            std::cerr << "[Validator] Error in " << file_name << ": Item " << current_context << " must contain a boolean 'show'." << std::endl;
            return false;
        }

        // 2. 验证可选字段: db_column (string)
        if (item.contains("db_column") && !item["db_column"].is_string()) {
            std::cerr << "[Validator] Error in " << file_name << ": 'db_column' in " << current_context << " must be a string." << std::endl;
            return false;
        }

        // 3. 递归验证 sub_items
        if (item.contains("sub_items")) {
            if (!validate_stats_recursive(item["sub_items"], file_name, current_context + ".sub_items")) {
                return false;
            }
        }
    }
    return true;
}

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
    // 步骤3：对日报类型执行 statistics_items 的验证
    if (file_name.find("Day") != std::string::npos) {
        if (!validate_statistics_items(query_json, file_name)) {
            return false;
        }
    }
    return true;
}

bool BaseStrategy::validate_common_rules(const json& query_json, const std::string& file_name) const {
    // ... (此部分代码保持不变) ...
    const std::set<std::string> numeric_keys = {
        "base_font_size", "report_title_font_size", "category_title_font_size",
        "margin_in", "margin_top_cm", "margin_bottom_cm", "margin_left_cm", "margin_right_cm",
        "list_top_sep_pt", "list_item_sep_ex", "line_spacing_em"
    };

    for (auto it = query_json.begin(); it != query_json.end(); ++it) {
        const std::string& key = it.key();
        const auto& value = it.value();
        
        // ... (跳过 statistics_items 的通用检查，因为它由专用函数处理)
        if (key == "statistics_items") continue; 

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

// [修改] 更新后的验证逻辑
bool BaseStrategy::validate_statistics_items(const json& query_json, const std::string& file_name) const {
    if (!query_json.contains("statistics_items")) return true; 

    const auto& stats_items = query_json["statistics_items"];

    // [核心修改] 调用递归验证函数，并不再强制检查特定的硬编码 Key (如 sleep_time)
    // 因为现在的配置是完全动态的，用户可以随意增删统计项。
    return validate_stats_recursive(stats_items, file_name);
}

bool BaseStrategy::is_valid_hex_color(const std::string& color_string) const {
    static const std::regex hex_color_regex(R"(^#[0-9a-fA-F]{6}$)");
    return std::regex_match(color_string, hex_color_regex);
}
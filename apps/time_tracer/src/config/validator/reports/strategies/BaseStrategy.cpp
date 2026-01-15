// config/validator/reports/strategies/BaseStrategy.cpp
#include "BaseStrategy.hpp"
#include <iostream>
#include <set>
#include <regex>
#include <string>

// [修改] 静态辅助函数：递归验证统计项数组 (TOML 版本)
static bool validate_stats_recursive(const toml::array& items_array, const std::string& file_name, const std::string& context = "root") {
    for (size_t i = 0; i < items_array.size(); ++i) {
        const auto& item_node = items_array[i];
        std::string current_context = context + "[" + std::to_string(i) + "]";

        if (!item_node.is_table()) {
            std::cerr << "[Validator] Error in " << file_name << ": Item " << current_context << " must be a table." << std::endl;
            return false;
        }

        const auto& item = *item_node.as_table();

        // 1. 验证必需字段: label (string), show (bool/optional - TOML defaults apply but strict check here if needed)
        if (!item.contains("label") || !item["label"].is_string()) {
            std::cerr << "[Validator] Error in " << file_name << ": Item " << current_context << " must contain a string 'label'." << std::endl;
            return false;
        }
        // show 是可选的在Loader中，但在Validator中如果存在必须是bool? 或者这里放宽检查
        if (item.contains("show") && !item["show"].is_boolean()) {
             std::cerr << "[Validator] Error in " << file_name << ": Item " << current_context << " 'show' must be a boolean." << std::endl;
             return false;
        }

        // 2. 验证可选字段: db_column (string)
        if (item.contains("db_column") && !item["db_column"].is_string()) {
            std::cerr << "[Validator] Error in " << file_name << ": 'db_column' in " << current_context << " must be a string." << std::endl;
            return false;
        }

        // 3. 递归验证 sub_items
        if (item.contains("sub_items")) {
            if (const toml::array* sub_arr = item["sub_items"].as_array()) {
                if (!validate_stats_recursive(*sub_arr, file_name, current_context + ".sub_items")) {
                    return false;
                }
            } else {
                 std::cerr << "[Validator] Error in " << file_name << ": 'sub_items' in " << current_context << " must be an array." << std::endl;
                 return false;
            }
        }
    }
    return true;
}

// --- 模板方法实现 ---
bool BaseStrategy::validate(const toml::table& query_config, const std::string& file_name) const {
    // 步骤1：执行子类实现的特定key验证
    if (!validate_specific_keys(query_config, file_name)) {
        return false;
    }
    // 步骤2：执行通用的验证规则
    if (!validate_common_rules(query_config, file_name)) {
        return false;
    }
    // 步骤3：对日报类型执行 statistics_items 的验证
    if (file_name.find("Day") != std::string::npos) {
        if (!validate_statistics_items(query_config, file_name)) {
            return false;
        }
    }
    return true;
}

bool BaseStrategy::validate_common_rules(const toml::table& query_config, const std::string& file_name) const {
    const std::set<std::string> numeric_keys = {
        "base_font_size", "report_title_font_size", "category_title_font_size",
        "margin_in", "margin_top_cm", "margin_bottom_cm", "margin_left_cm", "margin_right_cm",
        "list_top_sep_pt", "list_item_sep_ex", "line_spacing_em"
    };

    for (const auto& [key, node] : query_config) {
        std::string key_str(key.str());
        
        // 跳过 statistics_items
        if (key_str == "statistics_items") continue; 

        // TOML++ node 检查
        if (numeric_keys.count(key_str) && !node.is_number()) {
            std::cerr << "[Validator] Error in " << file_name << ": Key '" << key_str << "' must have a numeric value." << std::endl;
            return false;
        }

        if (key_str == "keyword_colors") {
            if (!node.is_table()) {
                 std::cerr << "[Validator] Error in " << file_name << ": 'keyword_colors' must be a table." << std::endl;
                 return false;
            }
            const auto& colors = *node.as_table();
            for (const auto& [color_key, color_val] : colors) {
                if (!color_val.is_string() || !is_valid_hex_color(color_val.value<std::string>().value_or(""))) {
                    std::cerr << "[Validator] Error in " << file_name << ": Color for '" << color_key.str()
                              << "' is not a valid hex string (e.g., #RRGGBB)." << std::endl;
                    return false;
                }
            }
        }
        
        if (node.is_string()) {
            const std::string& val_str = *node.value<std::string>();
            if (val_str.empty() && key_str != "remark" && key_str != "db_column") {
                 // 注意：remark允许为空，db_column也可能为空
            }
        }
    }
    return true;
}

bool BaseStrategy::validate_statistics_items(const toml::table& query_config, const std::string& file_name) const {
    if (!query_config.contains("statistics_items")) return true; 

    const auto& items_node = query_config["statistics_items"];
    if (!items_node.is_array()) {
        std::cerr << "[Validator] Error in " << file_name << ": 'statistics_items' must be an array." << std::endl;
        return false;
    }

    return validate_stats_recursive(*items_node.as_array(), file_name);
}

bool BaseStrategy::is_valid_hex_color(const std::string& color_string) const {
    static const std::regex hex_color_regex(R"(^#[0-9a-fA-F]{6}$)");
    return std::regex_match(color_string, hex_color_regex);
}
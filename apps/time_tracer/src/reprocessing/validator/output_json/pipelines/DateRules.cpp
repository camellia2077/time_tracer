// reprocessing/validator/output_json/pipelines/DateRules.cpp
#include "DateRules.hpp"
#include <map>
#include <string>
#include <iostream>

using json = nlohmann::json;

namespace {
    bool is_leap(int year) { return (year % 4 == 0 && (year % 100 != 0 || year % 400 == 0)); }
    int days_in_month(int year, int month) {
        if (month < 1 || month > 12) return 0;
        if (month == 2) return is_leap(year) ? 29 : 28;
        if (month == 4 || month == 6 || month == 9 || month == 11) return 30;
        return 31;
    }
}

// [核心修改] 实现不同的检查模式逻辑
void validateDateContinuity(const json& days_array, std::set<Error>& errors, DateCheckMode mode) {
    if (mode == DateCheckMode::None) return; // 如果是 None，直接返回
    if (days_array.empty()) return;
    
    const auto& first_day_headers = days_array[0].value("headers", json::object());
    std::string first_date = first_day_headers.value("date", "");
    
    // [修改] 检查长度：YYYY-MM-DD 长度为 10
    if (first_date.length() != 10) return;
    
    // [修改] 提取 YYYY-MM ("2025-01")
    std::string yyyy_mm = first_date.substr(0, 7);
    int year = std::stoi(first_date.substr(0, 4));
    // [修改] 月份索引变为 5
    int month = std::stoi(first_date.substr(5, 2));

    std::map<std::string, std::set<int>> month_day_map;
    for (const auto& day : days_array) {
        const auto& headers = day.value("headers", json::object());
        std::string date_str = headers.value("date", "");
        
        // [修改] 匹配 YYYY-MM 前缀，长度 10
        if (date_str.rfind(yyyy_mm, 0) == 0 && date_str.length() == 10) {
            // [修改] 日期索引变为 8 (2025-01-01)
            month_day_map[yyyy_mm].insert(std::stoi(date_str.substr(8, 2)));
        }
    }

    const auto& days_found = month_day_map[yyyy_mm];
    int num_days_in_this_month = days_in_month(year, month);
    
    int check_until = num_days_in_this_month; // 默认为 Full 模式：检查到月底

    if (mode == DateCheckMode::Continuity) {
        // 如果是 Continuity 模式：只检查到当前发现的最大日期
        if (days_found.empty()) return;
        check_until = *days_found.rbegin(); 
    }

    for (int d = 1; d <= check_until; ++d) {
        if (days_found.find(d) == days_found.end()) {
            // [修改] 构造缺失日期的字符串 YYYY-MM-DD
            std::string missing_date_str = yyyy_mm + "-" + (d < 10 ? "0" : "") + std::to_string(d);
            std::string error_msg = "Missing date detected in month " + yyyy_mm + ": " + missing_date_str;
            
            if (mode == DateCheckMode::Continuity) {
                error_msg += " (Continuity Check)";
            } else {
                error_msg += " (Completeness Check)";
            }
            
            errors.insert({0, error_msg, ErrorType::DateContinuity});
        }
    }
}
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
    if (first_date.length() != 8) return;
    
    std::string yyyymm = first_date.substr(0, 6);
    int year = std::stoi(yyyymm.substr(0, 4));
    int month = std::stoi(yyyymm.substr(4, 2));

    std::map<std::string, std::set<int>> month_day_map;
    for (const auto& day : days_array) {
        const auto& headers = day.value("headers", json::object());
        std::string date_str = headers.value("date", "");
        if (date_str.rfind(yyyymm, 0) == 0 && date_str.length() == 8) {
            month_day_map[yyyymm].insert(std::stoi(date_str.substr(6, 2)));
        }
    }

    const auto& days_found = month_day_map[yyyymm];
    int num_days_in_this_month = days_in_month(year, month);
    
    // [核心逻辑变化]
    int check_until = num_days_in_this_month; // 默认为 Full 模式：检查到月底

    if (mode == DateCheckMode::Continuity) {
        // 如果是 Continuity 模式：只检查到当前发现的最大日期
        if (days_found.empty()) return;
        check_until = *days_found.rbegin(); 
    }

    for (int d = 1; d <= check_until; ++d) {
        if (days_found.find(d) == days_found.end()) {
            std::string missing_date_str = yyyymm + (d < 10 ? "0" : "") + std::to_string(d);
            std::string error_msg = "Missing date detected in month " + yyyymm.substr(0,4) + "-" + yyyymm.substr(4,2) + ": " + missing_date_str;
            
            // 稍微区分一下报错信息，方便调试
            if (mode == DateCheckMode::Continuity) {
                error_msg += " (Continuity Check)";
            } else {
                error_msg += " (Completeness Check)";
            }
            
            errors.insert({0, error_msg, ErrorType::DateContinuity});
        }
    }
}
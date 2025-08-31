// reprocessing/validator/internal/JsonValidator.cpp
#include "JsonValidator.hpp"
#include <fstream>
#include <iostream>
#include <map>
#include <algorithm>
#include <vector>

using json = nlohmann::json;

JsonValidator::JsonValidator(bool enable_day_count_check)
    : check_day_count_enabled_(enable_day_count_check) {}

bool JsonValidator::validate(const std::string& file_path, std::set<Error>& errors) {
    std::ifstream file(file_path);
    if (!file.is_open()) {
        errors.insert({0, "Could not open JSON file: " + file_path, ErrorType::FileAccess});
        return false;
    }

    json days_array;
    try {
        file >> days_array;
        if (!days_array.is_array()) {
            errors.insert({0, "JSON root is not an array. The file should contain an array of day objects.", ErrorType::Structural});
            return false;
        }
    } catch (const json::parse_error& e) {
        errors.insert({0, "Failed to parse JSON file: " + std::string(e.what()), ErrorType::Structural});
        return false;
    }

    if (days_array.empty()) {
        return true; // 空文件是有效的
    }

    if (check_day_count_enabled_) {
        validateDateContinuity(days_array, errors);
    }

    for (const auto& day_object : days_array) {
        if (!day_object.is_object()) continue;
        validateTimeContinuity(day_object, errors);
        validateHighLevelRules(day_object, errors);
    }
    
    return errors.empty();
}

namespace {
    bool is_leap(int year) { return (year % 4 == 0 && (year % 100 != 0 || year % 400 == 0)); }
    int days_in_month(int year, int month) {
        if (month < 1 || month > 12) return 0;
        if (month == 2) return is_leap(year) ? 29 : 28;
        if (month == 4 || month == 6 || month == 9 || month == 11) return 30;
        return 31;
    }
}

void JsonValidator::validateDateContinuity(const json& days_array, std::set<Error>& errors) {
    if (days_array.empty()) return;
    
    // [修复] 从 Headers 对象中获取日期
    const auto& first_day_headers = days_array[0].value("Headers", json::object());
    std::string first_date = first_day_headers.value("Date", "");
    if (first_date.length() != 8) return;
    
    std::string yyyymm = first_date.substr(0, 6);
    int year = std::stoi(yyyymm.substr(0, 4));
    int month = std::stoi(yyyymm.substr(4, 2));

    std::map<std::string, std::set<int>> month_day_map;
    for (const auto& day : days_array) {
        const auto& headers = day.value("Headers", json::object());
        std::string date_str = headers.value("Date", "");
        if (date_str.rfind(yyyymm, 0) == 0 && date_str.length() == 8) {
            month_day_map[yyyymm].insert(std::stoi(date_str.substr(6, 2)));
        }
    }

    const auto& days_found = month_day_map[yyyymm];
    int num_days_in_this_month = days_in_month(year, month);
            
    for (int d = 1; d <= num_days_in_this_month; ++d) {
        if (days_found.find(d) == days_found.end()) {
            std::string missing_date_str = yyyymm + (d < 10 ? "0" : "") + std::to_string(d);
            errors.insert({0, "Missing date detected in month " + yyyymm.substr(0,4) + "-" + yyyymm.substr(4,2) + ": " + missing_date_str, ErrorType::DateContinuity});
        }
    }
}

void JsonValidator::validateTimeContinuity(const json& day_object, std::set<Error>& errors) {
    if (!day_object.contains("Activities") || !day_object["Activities"].is_array() || day_object["Activities"].empty()) {
        return;
    }

    // [修复] 从 Headers 对象中获取元数据
    const auto& headers = day_object.value("Headers", json::object());
    const auto& activities = day_object["Activities"];
    
    std::string last_end_time = headers.value("Getup", "00:00");
    if (last_end_time == "Null") {
        if (!activities.empty()) {
            last_end_time = activities[0].value("startTime", "");
        }
    }

    for (const auto& activity : activities) {
        std::string start_time = activity.value("startTime", "");
        if (start_time != last_end_time) {
            std::string date_str = headers.value("Date", "[Unknown Date]");
            std::string error_msg = "Time discontinuity in file for date " + date_str + ". Expected start time " + last_end_time + ", but got " + start_time + ".";
            errors.insert({0, error_msg, ErrorType::TimeDiscontinuity});
        }
        last_end_time = activity.value("endTime", "");
    }
}

void JsonValidator::validateHighLevelRules(const json& day_object, std::set<Error>& errors) {
    // [修复] 从 Headers 对象中获取元数据
    const auto& headers = day_object.value("Headers", json::object());
    bool sleep_status = headers.value("Sleep", false);

    if (sleep_status) {
        if (!day_object.contains("Activities") || !day_object["Activities"].is_array() || day_object["Activities"].empty()) {
             std::string date_str = headers.value("Date", "[Unknown Date]");
             errors.insert({0, "In file for date " + date_str + ": Last activity must be 'sleep' when Sleep is True, but no activities found.", ErrorType::MissingSleepNight});
             return;
        }
        const auto& last_activity = day_object["Activities"].back();
        std::string title = last_activity.value("activity", json::object()).value("title", "");
        if (title != "sleep") {
            std::string date_str = headers.value("Date", "[Unknown Date]");
            errors.insert({0, "In file for date " + date_str + ": The last activity must be 'sleep' when Sleep is True.", ErrorType::MissingSleepNight});
        }
    }
}
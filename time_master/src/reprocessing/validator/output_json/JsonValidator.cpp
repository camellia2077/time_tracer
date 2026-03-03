// reprocessing/validator/output_json/JsonValidator.cpp
#include "JsonValidator.hpp"
#include <fstream>
#include <iostream>
#include <map>
#include <algorithm>
#include <vector>

using json = nlohmann::json;

// 构造函数，初始化是否检查日期计数的标志
JsonValidator::JsonValidator(bool enable_day_count_check)
    : check_day_count_enabled_(enable_day_count_check) {}

// validate是公开的核心方法，负责协调所有验证步骤
bool JsonValidator::validate(const std::string& file_path, std::set<Error>& errors) {
    // 1. 尝试打开文件
    std::ifstream file(file_path);
    if (!file.is_open()) {
        errors.insert({0, "Could not open JSON file: " + file_path, ErrorType::FileAccess});
        return false;
    }

    // 2. 解析JSON
    json days_array;
    try {
        file >> days_array; // 从文件流读取并解析JSON
        // 验证根结构必须是数组
        if (!days_array.is_array()) {
            errors.insert({0, "JSON root is not an array. The file should contain an array of day objects.", ErrorType::Structural});
            return false;
        }
    } catch (const json::parse_error& e) {
        // 如果解析失败，记录错误并返回
        errors.insert({0, "Failed to parse JSON file: " + std::string(e.what()), ErrorType::Structural});
        return false;
    }

    // 如果文件是空的（或只包含一个空数组），视为有效
    if (days_array.empty()) {
        return true; 
    }

    // 3. (可选) 如果启用了日期计数检查，则执行日期连续性验证
    if (check_day_count_enabled_) {
        validateDateContinuity(days_array, errors);
    }

    // 4. 遍历每一天的对象，执行针对单日的验证
    for (const auto& day_object : days_array) {
        if (!day_object.is_object()) continue; // 跳过非对象元素
        validateTimeContinuity(day_object, errors); // 验证时间连续性
        validateHighLevelRules(day_object, errors);   // 验证高级业务规则
    }
    
    // 5. 如果错误集合为空，则表示验证通过
    return errors.empty();
}

// 匿名空间，包含仅在此文件内使用的辅助函数
namespace {
    // 判断是否为闰年
    bool is_leap(int year) { return (year % 4 == 0 && (year % 100 != 0 || year % 400 == 0)); }
    // 获取指定年月的天数
    int days_in_month(int year, int month) {
        if (month < 1 || month > 12) return 0;
        if (month == 2) return is_leap(year) ? 29 : 28;
        if (month == 4 || month == 6 || month == 9 || month == 11) return 30;
        return 31;
    }
}

// 验证日期的连续性
void JsonValidator::validateDateContinuity(const json& days_array, std::set<Error>& errors) {
    if (days_array.empty()) return;
    
    // 从第一个"天"对象中获取"Headers"，并从中提取日期
    const auto& first_day_headers = days_array[0].value("Headers", json::object());
    std::string first_date = first_day_headers.value("Date", "");
    if (first_date.length() != 8) return; // 如果日期格式不正确，则无法继续
    
    // 提取年和月 (YYYYMM)
    std::string yyyymm = first_date.substr(0, 6);
    int year = std::stoi(yyyymm.substr(0, 4));
    int month = std::stoi(yyyymm.substr(4, 2));

    // 使用 map 来存储每个月份出现过的所有日期
    std::map<std::string, std::set<int>> month_day_map;
    for (const auto& day : days_array) {
        const auto& headers = day.value("Headers", json::object());
        std::string date_str = headers.value("Date", "");
        // 确保日期属于同一个月份，并且格式正确
        if (date_str.rfind(yyyymm, 0) == 0 && date_str.length() == 8) {
            month_day_map[yyyymm].insert(std::stoi(date_str.substr(6, 2)));
        }
    }

    // 获取当月应该有的总天数
    const auto& days_found = month_day_map[yyyymm];
    int num_days_in_this_month = days_in_month(year, month);
            
    // 遍历从1到该月最后一天的所有日期
    for (int d = 1; d <= num_days_in_this_month; ++d) {
        // 如果在集合中找不到某一天，则记录为缺失日期错误
        if (days_found.find(d) == days_found.end()) {
            std::string missing_date_str = yyyymm + (d < 10 ? "0" : "") + std::to_string(d);
            errors.insert({0, "Missing date detected in month " + yyyymm.substr(0,4) + "-" + yyyymm.substr(4,2) + ": " + missing_date_str, ErrorType::DateContinuity});
        }
    }
}

// 验证一天内活动的时间是否连续
void JsonValidator::validateTimeContinuity(const json& day_object, std::set<Error>& errors) {
    if (!day_object.contains("Activities") || !day_object["Activities"].is_array() || day_object["Activities"].empty()) {
        return; // 如果没有活动，则无需检查
    }

    const auto& headers = day_object.value("Headers", json::object());
    const auto& activities = day_object["Activities"];
    
    // 将上一个活动的结束时间初始化为当天的起床时间
    std::string last_end_time = headers.value("Getup", "00:00");
    if (last_end_time == "Null") { // 处理 "Getup" 为 "Null" 的情况
        if (!activities.empty()) {
            // 如果没有起床时间，则使用第一个活动的开始时间作为初始值
            last_end_time = activities[0].value("startTime", "");
        }
    }

    // 遍历所有活动
    for (const auto& activity : activities) {
        std::string start_time = activity.value("startTime", "");
        // 检查当前活动的开始时间是否等于上一个活动的结束时间
        if (start_time != last_end_time) {
            std::string date_str = headers.value("Date", "[Unknown Date]");
            std::string error_msg = "Time discontinuity in file for date " + date_str + ". Expected start time " + last_end_time + ", but got " + start_time + ".";
            errors.insert({0, error_msg, ErrorType::TimeDiscontinuity});
        }
        // 更新 last_end_time 为当前活动的结束时间，为下一次循环做准备
        last_end_time = activity.value("endTime", "");
    }
}

// 验证高级业务规则
void JsonValidator::validateHighLevelRules(const json& day_object, std::set<Error>& errors) {
    const auto& headers = day_object.value("Headers", json::object());
    bool sleep_status = headers.value("Sleep", false); // 获取 Sleep 状态

    // 如果 Sleep 状态为 true
    if (sleep_status) {
        // 检查是否存在活动列表
        if (!day_object.contains("Activities") || !day_object["Activities"].is_array() || day_object["Activities"].empty()) {
             std::string date_str = headers.value("Date", "[Unknown Date]");
             errors.insert({0, "In file for date " + date_str + ": Last activity must be 'sleep' when Sleep is True, but no activities found.", ErrorType::MissingSleepNight});
             return;
        }
        // 获取最后一个活动
        const auto& last_activity = day_object["Activities"].back();
        std::string title = last_activity.value("activity", json::object()).value("top_parents", "");
        // 验证最后一个活动的标题是否为 "sleep"
        if (title != "sleep") {
            std::string date_str = headers.value("Date", "[Unknown Date]");
            errors.insert({0, "In file for date " + date_str + ": The last activity must be 'sleep' when Sleep is True.", ErrorType::MissingSleepNight});
        }
    }
}
// converter/convert/pipelines/DayStats.cpp
#include "DayStats.hpp"
#include "StatsRules.hpp"
#include <iomanip>
#include <sstream>
#include <ctime>
#include <stdexcept>
#include <algorithm> // 必须包含，用于 std::remove

namespace {
    // [核心修改] 这里的逻辑大大简化，因为输入已经是标准格式
    long long string_to_time_t(const std::string& datetime_str) {
        // datetime_str 格式预期: "YYYY-MM-DD HH:MM" (长度 16)
        if (datetime_str.length() < 16) {
            return 0;
        }
        std::tm t = {};
        
        // 直接解析，不需要 substr 拼接
        std::stringstream ss(datetime_str);
        ss >> std::get_time(&t, "%Y-%m-%d %H:%M");

        if (ss.fail()) {
            return 0;
        }
        return std::mktime(&t);
    }
}

int DayStats::calculateDurationSeconds(const std::string& startTimeStr, const std::string& endTimeStr) const {
    if (startTimeStr.length() != 5 || endTimeStr.length() != 5) return 0;
    try {
        int startHour = std::stoi(startTimeStr.substr(0, 2));
        int startMin = std::stoi(startTimeStr.substr(3, 2));
        int endHour = std::stoi(endTimeStr.substr(0, 2));
        int endMin = std::stoi(endTimeStr.substr(3, 2));
        int startTimeInSeconds = (startHour * 60 + startMin) * 60;
        int endTimeInSeconds = (endHour * 60 + endMin) * 60;
        if (endTimeInSeconds < startTimeInSeconds) {
            endTimeInSeconds += 24 * 60 * 60;
        }
        return endTimeInSeconds - startTimeInSeconds;
    } catch (const std::exception&) {
        return 0;
    }
}

long long DayStats::timeStringToTimestamp(const std::string& date, const std::string& time, bool is_end_time, long long start_timestamp_for_end) const {
    // date 格式现在是 YYYY-MM-DD (10 chars), time 是 HH:MM (5 chars)
    if (date.length() != 10 || time.length() != 5) return 0;
    
    std::string datetime_str = date + " " + time; // "2025-01-01 07:00"
    long long timestamp = string_to_time_t(datetime_str);
    
    if (is_end_time && timestamp < start_timestamp_for_end) {
        timestamp += 24 * 60 * 60;
    }
    return timestamp;
}


void DayStats::calculate_stats(InputData& day) {
    day.activityCount = day.processedActivities.size();
    day.generatedStats = {}; 
    day.hasStudyActivity = false;
    day.hasExerciseActivity = false;

    long long activity_sequence = 1;
    long long date_as_long = 0;
    try {
        // [核心修改] 为了生成 logical_id，需要去掉连字符
        std::string temp_date = day.date; // "2025-01-01"
        temp_date.erase(std::remove(temp_date.begin(), temp_date.end(), '-'), temp_date.end());
        // 变成 "20250101"，用于数据库 ID
        date_as_long = std::stoll(temp_date); 
    } catch (const std::invalid_argument&) {
        return;
    }

    for (auto& activity : day.processedActivities) {
        // ID 格式保持: 202501010001
        activity.logical_id = date_as_long * 10000 + activity_sequence++;
        activity.durationSeconds = calculateDurationSeconds(activity.startTime, activity.endTime);
        activity.start_timestamp = timeStringToTimestamp(day.date, activity.startTime, false, 0);
        activity.end_timestamp = timeStringToTimestamp(day.date, activity.endTime, true, activity.start_timestamp);

        if (activity.project_path.rfind("study", 0) == 0) { 
            day.hasStudyActivity = true;
        }
        if (activity.project_path.rfind("exercise", 0) == 0) {
            day.hasExerciseActivity = true;
        }
        if (activity.project_path == "sleep_night") {
            day.generatedStats.sleepNightTime += activity.durationSeconds;
        }
        if (activity.project_path == "sleep_day") {
            day.generatedStats.sleepDayTime += activity.durationSeconds;
        }

        for (const auto& rule : StatsRules::rules) {
            if (activity.project_path.find(rule.match_path) == 0) {
                (day.generatedStats.*(rule.member)) += activity.durationSeconds;
            }
        }
    }

    day.generatedStats.sleepTotalTime = day.generatedStats.sleepNightTime + day.generatedStats.sleepDayTime;
}
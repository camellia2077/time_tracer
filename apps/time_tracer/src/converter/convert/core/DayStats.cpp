// converter/convert/core/DayStats.cpp
#include "DayStats.hpp"
#include "StatsRules.hpp"
#include <iomanip>
#include <sstream>
#include <ctime>
#include <stdexcept>
#include <algorithm> 

namespace {
    long long string_to_time_t(const std::string& datetime_str) {
        if (datetime_str.length() < 16) {
            return 0;
        }
        std::tm t = {};
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
    if (date.length() != 10 || time.length() != 5) return 0;
    
    std::string datetime_str = date + " " + time; 
    long long timestamp = string_to_time_t(datetime_str);
    
    if (is_end_time && timestamp < start_timestamp_for_end) {
        timestamp += 24 * 60 * 60;
    }
    return timestamp;
}


void DayStats::calculate_stats(DailyLog& day) {
    day.activityCount = day.processedActivities.size();
    day.stats = {}; // [适配] generatedStats -> stats
    day.hasStudyActivity = false;
    day.hasExerciseActivity = false;

    long long activity_sequence = 1;
    long long date_as_long = 0;
    try {
        std::string temp_date = day.date; 
        temp_date.erase(std::remove(temp_date.begin(), temp_date.end(), '-'), temp_date.end());
        date_as_long = std::stoll(temp_date); 
    } catch (const std::invalid_argument&) {
        return;
    }

    for (auto& activity : day.processedActivities) {
        activity.logical_id = date_as_long * 10000 + activity_sequence++;
        // [适配] startTime/endTime -> start_time_str/end_time_str
        activity.duration_seconds = calculateDurationSeconds(activity.start_time_str, activity.end_time_str);
        
        activity.start_timestamp = timeStringToTimestamp(day.date, activity.start_time_str, false, 0);
        activity.end_timestamp = timeStringToTimestamp(day.date, activity.end_time_str, true, activity.start_timestamp);

        if (activity.project_path.rfind("study", 0) == 0) { 
            day.hasStudyActivity = true;
        }
        if (activity.project_path.rfind("exercise", 0) == 0) {
            day.hasExerciseActivity = true;
        }
        
        // [适配] 字段名 generatedStats -> stats，以及驼峰转 snake_case
        if (activity.project_path == "sleep_night") {
            day.stats.sleep_night_time += activity.duration_seconds;
        }
        if (activity.project_path == "sleep_day") {
            day.stats.sleep_day_time += activity.duration_seconds;
        }

        // 使用规则反射进行统计
        for (const auto& rule : StatsRules::rules) {
            if (activity.project_path.find(rule.match_path) == 0) {
                (day.stats.*(rule.member)) += activity.duration_seconds;
            }
        }
    }

    day.stats.sleep_total_time = day.stats.sleep_night_time + day.stats.sleep_day_time;
}
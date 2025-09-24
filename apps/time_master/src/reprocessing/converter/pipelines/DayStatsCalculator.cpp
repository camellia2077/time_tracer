// reprocessing/converter/pipelines/DayStatsCalculator.cpp
#include "DayStatsCalculator.hpp"
#include "GeneratedStatsRules.hpp"
#include <iomanip>
#include <sstream>
#include <ctime>
#include <stdexcept>
#include <algorithm>

namespace {
    long long string_to_time_t(const std::string& datetime_str) {
        if (datetime_str.length() < 14) {
            return 0;
        }
        std::tm t = {};
        std::string formatted_datetime = datetime_str.substr(0, 4) + "-" +
                                         datetime_str.substr(4, 2) + "-" +
                                         datetime_str.substr(6, 2) +
                                         datetime_str.substr(8);

        std::stringstream ss(formatted_datetime);
        ss >> std::get_time(&t, "%Y-%m-%d %H:%M");

        if (ss.fail()) {
            return 0;
        }
        return std::mktime(&t);
    }
}

int DayStatsCalculator::calculateDurationSeconds(const std::string& startTimeStr, const std::string& endTimeStr) const {
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

long long DayStatsCalculator::timeStringToTimestamp(const std::string& date, const std::string& time, bool is_end_time, long long start_timestamp_for_end) const {
    if (date.length() != 8 || time.length() != 5) return 0;
    std::string datetime_str = date + " " + time;
    long long timestamp = string_to_time_t(datetime_str);
    if (is_end_time && timestamp < start_timestamp_for_end) {
        timestamp += 24 * 60 * 60;
    }
    return timestamp;
}


void DayStatsCalculator::calculate_stats(InputData& day) {
    day.activityCount = day.processedActivities.size();
    day.generatedStats = {}; // 重置
    day.hasStudyActivity = false;
    day.hasExerciseActivity = false;

    long long activity_sequence = 1;
    long long date_as_long = 0;
    try {
        date_as_long = std::stoll(day.date);
    } catch (const std::invalid_argument&) {
        return;
    }


    for (auto& activity : day.processedActivities) {
        activity.logical_id = date_as_long * 10000 + activity_sequence++;
        activity.durationSeconds = calculateDurationSeconds(activity.startTime, activity.endTime);
        activity.start_timestamp = timeStringToTimestamp(day.date, activity.startTime, false, 0);
        activity.end_timestamp = timeStringToTimestamp(day.date, activity.endTime, true, activity.start_timestamp);

        // --- [核心修改] ---
        // 基于 project_path 进行判断
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

        for (const auto& rule : GeneratedStatsRules::rules) {
            // 使用 find 检查 project_path 是否以规则的路径开头
            if (activity.project_path.find(rule.match_path) == 0) {
                (day.generatedStats.*(rule.member)) += activity.durationSeconds;
            }
        }
    }

    day.generatedStats.sleepTotalTime = day.generatedStats.sleepNightTime + day.generatedStats.sleepDayTime;
}
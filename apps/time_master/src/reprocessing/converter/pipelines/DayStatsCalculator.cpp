// reprocessing/converter/pipelines/DayStatsCalculator.cpp
#include "DayStatsCalculator.hpp"
#include <string>
#include <stdexcept>
#include <iomanip> // Required for std::get_time
#include <sstream> // Required for std::stringstream
#include <ctime>   // Required for std::tm, std::mktime
#include <algorithm> // Required for std::find

namespace {
    // Helper function to convert "YYYYMMDD HH:MM" string to a Unix timestamp.
    long long string_to_time_t(const std::string& datetime_str) {
        if (datetime_str.length() < 14) { // Requires "YYYYMMDD HH:MM"
            return 0;
        }
        
        std::tm t = {};
        // Create a stringstream from a formatted date string that std::get_time can parse.
        std::string formatted_datetime = datetime_str.substr(0, 4) + "-" +
                                         datetime_str.substr(4, 2) + "-" +
                                         datetime_str.substr(6, 2) +
                                         datetime_str.substr(8); // The space is at index 8

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
    if (date.length() != 8 || time.length() != 5) {
        return 0;
    }
    std::string datetime_str = date + " " + time;
    long long timestamp = string_to_time_t(datetime_str);

    if (is_end_time && timestamp < start_timestamp_for_end) {
        timestamp += 24 * 60 * 60; // Add one day in seconds if it wraps around midnight
    }
    return timestamp;
}


void DayStatsCalculator::calculate_stats(InputData& day) {
    day.activityCount = day.processedActivities.size();
    day.generatedStats = {};

    day.hasStudyActivity = false;
    day.hasExerciseActivity = false; // 新增：重置运动标志
    long long activity_sequence = 1;
    long long date_as_long = 0;
    try {
        date_as_long = std::stoll(day.date);
    } catch (const std::invalid_argument& e) {
        // Handle error if day.date is not a valid number
        return;
    }


    for (auto& activity : day.processedActivities) {
        activity.logical_id = date_as_long * 10000 + activity_sequence++;
        activity.durationSeconds = calculateDurationSeconds(activity.startTime, activity.endTime);
        activity.start_timestamp = timeStringToTimestamp(day.date, activity.startTime, false, 0);
        
        // Pass the start timestamp to correctly handle end times that cross midnight
        activity.end_timestamp = timeStringToTimestamp(day.date, activity.endTime, true, activity.start_timestamp);

        if (activity.topParent.find("study") != std::string::npos) { 
            day.hasStudyActivity = true;
        }
        
        if (activity.topParent == "sleep") {
            day.generatedStats.sleepTime = activity.durationSeconds;
        }

        // --- [核心修改] 扩展运动统计逻辑 ---
        if (activity.topParent == "exercise") {
            day.hasExerciseActivity = true;
            day.generatedStats.totalExerciseTime += activity.durationSeconds;

            if (!activity.parents.empty()) {
                const std::string& exerciseType = activity.parents[0];
                if (exerciseType == "cardio") {
                    day.generatedStats.cardioTime += activity.durationSeconds;
                } else if (exerciseType == "anaerobic") {
                    day.generatedStats.anaerobicTime += activity.durationSeconds;
                } else if (exerciseType == "both") {
                    day.generatedStats.exerciseBothTime += activity.durationSeconds;
                }
            }
        }
        
        // --- [新增] 累加洗漱时间 ---
        if (activity.topParent == "routine") {
            // 检查 "parents" 数组中是否包含 "grooming"
            if (std::find(activity.parents.begin(), activity.parents.end(), "grooming") != activity.parents.end()) {
                day.generatedStats.groomingTime += activity.durationSeconds;
            }
        }
    }
}
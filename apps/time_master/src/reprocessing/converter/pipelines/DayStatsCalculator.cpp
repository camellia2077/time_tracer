// reprocessing/converter/pipelines/DayStatsCalculator.cpp
#include "DayStatsCalculator.hpp"
#include "GeneratedStatsRules.hpp" // 引入硬编码的规则
#include <string>
#include <stdexcept>
#include <iomanip>
#include <sstream>
#include <ctime>
#include <algorithm>
#include <cstring>

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

        if (activity.topParent.find("study") != std::string::npos) {
            day.hasStudyActivity = true;
        }
        if (activity.topParent == "exercise") {
            day.hasExerciseActivity = true;
        }

        // 使用硬编码的规则进行统计
        for (const auto& rule : GeneratedStatsRules::rules) {
            if (activity.topParent == rule.topParent) {
                bool match = (rule.parents.size() == 0);
                if (!match) {
                    for (const auto& required_parent : rule.parents) {
                        if (std::find(activity.parents.begin(), activity.parents.end(), required_parent) != activity.parents.end()) {
                            match = true;
                            break;
                        }
                    }
                }

                // [核心修改] 使用成员指针直接更新对应的统计字段
                if (match) {
                    (day.generatedStats.*(rule.member)) += activity.durationSeconds;
                }
            }
        }
    }
}
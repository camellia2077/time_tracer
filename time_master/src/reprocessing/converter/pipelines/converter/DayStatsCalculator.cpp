// reprocessing/converter/internal/converter/DayStatsCalculator.cpp
#include "DayStatsCalculator.hpp"
#include <string>
#include <stdexcept>

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

void DayStatsCalculator::calculate_stats(InputData& day) {
    day.activityCount = day.processedActivities.size();
    day.generatedStats = {}; // [新增] 在计算前清空统计数据

    // 重置 hasStudyActivity，重新计算
    day.hasStudyActivity = false;
    for (auto& activity : day.processedActivities) {
        if (activity.top_parent.find("study") != std::string::npos) {
            day.hasStudyActivity = true;
        }
        
        // [修改] 将 sleepTime 存储到新的 GeneratedStats 字段
        if (activity.top_parent == "sleep") {
            day.generatedStats.sleepTime = calculateDurationSeconds(activity.startTime, activity.endTime);
        }
        
        // [新增] 为每个活动计算并存储时长
        activity.durationSeconds = calculateDurationSeconds(activity.startTime, activity.endTime);
    }
}
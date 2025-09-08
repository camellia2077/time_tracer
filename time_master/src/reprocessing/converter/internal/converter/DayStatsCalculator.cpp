// reprocessing/converter/internal/converter/DayStatsCalculator.cpp
#include "DayStatsCalculator.hpp"
#include <string>

void DayStatsCalculator::calculate_stats(InputData& day) {
    day.activityCount = day.processedActivities.size();
    
    // 重置 hasStudyActivity，重新计算
    day.hasStudyActivity = false;
    for (const auto& activity : day.processedActivities) {
        if (activity.top_parent.find("study") != std::string::npos) {
            day.hasStudyActivity = true;
            break;
        }
    }
}
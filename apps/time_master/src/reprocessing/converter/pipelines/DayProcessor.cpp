// reprocessing/converter/pipelines/DayProcessor.cpp
#include "DayProcessor.hpp"
#include "DayStatsCalculator.hpp"
#include "ActivityMapper.hpp" // <-- [新增] 包含 ActivityMapper.hpp

namespace {
    std::string formatTime(const std::string& timeStrHHMM) {
        return (timeStrHHMM.length() == 4) ? timeStrHHMM.substr(0, 2) + ":" + timeStrHHMM.substr(2, 2) : timeStrHHMM;
    }
}

// --- [核心修改] 构造函数初始化新的 config_ 成员 ---
DayProcessor::DayProcessor(const ConverterConfig& config) : config_(config) {}

void DayProcessor::process(InputData& previousDay, InputData& dayToProcess) {
    if (dayToProcess.date.empty()) return;

    // --- [核心修改] 移除 converter_.transform() 调用 ---
    // 1. 直接创建 ActivityMapper 并调用其 map_activities 方法
    ActivityMapper activity_mapper(config_);
    activity_mapper.map_activities(dayToProcess);

    // 2. 如果存在有效的前一天，并且当天 *以起床记录开始*（即非延续日），则生成睡眠活动
    if (!previousDay.date.empty() && !previousDay.rawEvents.empty() && !dayToProcess.getupTime.empty() && !dayToProcess.isContinuation) {
        // 3. 睡眠的开始时间 = 前一天最后一个活动的结束时间
        std::string lastEventTime = formatTime(previousDay.rawEvents.back().endTimeStr);

        // 4. 创建睡眠活动
        Activity sleepActivity;
        sleepActivity.startTime = lastEventTime;
        sleepActivity.endTime = dayToProcess.getupTime;
        sleepActivity.topParent = "sleep";
        sleepActivity.parents = {"night"};

        // 5. 将睡眠活动插入到当天活动列表的 *最前面*
        dayToProcess.processedActivities.insert(dayToProcess.processedActivities.begin(), sleepActivity);

        // 6. 标记当天含有一个睡眠记录
        dayToProcess.hasSleepActivity = true;
    }

    // 处理“延续日”（即源文件中没有日期标记的跨天记录）
    if (dayToProcess.isContinuation && !previousDay.rawEvents.empty()) {
        dayToProcess.getupTime = formatTime(previousDay.rawEvents.back().endTimeStr);
    }

    // 7. 在所有活动（包括新加入的睡眠）都准备好后，再进行最终的统计计算
    DayStatsCalculator stats_calculator;
    stats_calculator.calculate_stats(dayToProcess);
}
// db_inserter/parser/facade/MemoryParserFacade.cpp
#include "MemoryParserFacade.hpp"
#include <iostream>

ParsedData MemoryParserFacade::parse(const std::map<std::string, std::vector<InputData>>& data_map) {
    ParsedData all_data;
    
    for (const auto& [month_str, days] : data_map) {
        for (const auto& input_day : days) {
            // 1. 转换天数据 (DayData)
            DayData day_data;
            day_data.date = input_day.date;
            
            // 解析 YYYY-MM-DD
            if (input_day.date.length() >= 7) {
                try {
                    day_data.year = std::stoi(input_day.date.substr(0, 4));
                    day_data.month = std::stoi(input_day.date.substr(5, 2));
                } catch (...) {
                    day_data.year = 0; day_data.month = 0;
                }
            } else {
                 day_data.year = 0; day_data.month = 0;
            }
            
            day_data.status = input_day.hasStudyActivity ? 1 : 0;
            day_data.sleep = input_day.hasSleepActivity ? 1 : 0;
            day_data.exercise = input_day.hasExerciseActivity ? 1 : 0;
            
            day_data.getup_time = input_day.getupTime.empty() ? "00:00" : input_day.getupTime;
            day_data.remark = input_day.generalRemarks.empty() ? "" : input_day.generalRemarks[0];
            
            // 统计数据映射
            const auto& stats = input_day.generatedStats;
            day_data.total_exercise_time = stats.totalExerciseTime;
            day_data.cardio_time = stats.cardioTime;
            day_data.anaerobic_time = stats.anaerobicTime;
            day_data.gaming_time = stats.gamingTime;
            day_data.grooming_time = stats.groomingTime;
            day_data.toilet_time = stats.toiletTime;
            
            day_data.sleep_night_time = stats.sleepNightTime;
            day_data.sleep_day_time = stats.sleepDayTime;
            day_data.sleep_total_time = stats.sleepTotalTime;
            
            day_data.recreation_time = stats.recreationTime;
            day_data.recreation_zhihu_time = stats.recreationZhihuTime;
            day_data.recreation_bilibili_time = stats.recreationBilibiliTime;
            day_data.recreation_douyin_time = stats.recreationDouyinTime;
            day_data.study_time = stats.studyTime;

            all_data.days.push_back(day_data);

            // 2. 转换记录数据 (TimeRecordInternal)
            for (const auto& activity : input_day.processedActivities) {
                TimeRecordInternal record;
                record.logical_id = activity.logical_id;
                record.start_timestamp = activity.start_timestamp;
                record.end_timestamp = activity.end_timestamp;
                record.date = input_day.date;
                record.start = activity.startTime;
                record.end = activity.endTime;
                record.project_path = activity.project_path;
                record.duration_seconds = activity.durationSeconds;
                
                if (activity.activityRemark.has_value()) {
                    record.activity_remark = activity.activityRemark.value();
                } else {
                    record.activity_remark = std::nullopt;
                }
                
                all_data.records.push_back(record);
            }
        }
    }
    return all_data;
}
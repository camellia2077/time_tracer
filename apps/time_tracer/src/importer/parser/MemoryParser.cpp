// importer/parser/MemoryParser.cpp
#include "MemoryParser.hpp"
#include <iostream>

ParsedData MemoryParser::parse(const std::map<std::string, std::vector<DailyLog>>& data_map) {
    ParsedData all_data;
    
    for (const auto& [month_str, days] : data_map) {
        for (const auto& input_day : days) {
            DayData day_data;
            day_data.date = input_day.date;
            
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

            std::string merged_remark;
            for (size_t i = 0; i < input_day.generalRemarks.size(); ++i) {
                merged_remark += input_day.generalRemarks[i];
                if (i < input_day.generalRemarks.size() - 1) {
                    merged_remark += "\n"; 
                }
            }
            day_data.remark = merged_remark;
            
            // [核心修改] 直接赋值统计数据！
            day_data.stats = input_day.stats;

            all_data.days.push_back(day_data);

            for (const auto& activity : input_day.processedActivities) {
                TimeRecordInternal record;
                
                // [核心修改] 利用 BaseActivityRecord 的赋值
                // record 是 TimeRecordInternal，继承自 BaseActivityRecord
                // activity 也是 BaseActivityRecord
                static_cast<BaseActivityRecord&>(record) = activity;
                
                // 补充特有字段
                record.date = input_day.date;
                
                all_data.records.push_back(record);
            }
        }
    }
    return all_data;
}
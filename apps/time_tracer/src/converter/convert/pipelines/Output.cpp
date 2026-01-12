// converter/convert/pipelines/Output.cpp
#include "Output.hpp"
#include <iostream>
#include <nlohmann/json.hpp>
#include "converter/convert/model/InputData.hpp"
#include "converter/convert/config/ConverterConfig.hpp"
#include <vector>

using json = nlohmann::json;

// [辅助函数] 将单天 InputData 转换为 JSON 对象 (逻辑复用)
json Output::convertDayToJson(const InputData& day) {
    if (day.date.empty()) return json{};

    json day_obj;
    json headers_obj;
    headers_obj["date"] = day.date;
    headers_obj["status"] = static_cast<int>(day.hasStudyActivity);
    headers_obj["exercise"] = static_cast<int>(day.hasExerciseActivity); 
    headers_obj["sleep"] = static_cast<int>(day.hasSleepActivity);
    headers_obj["getup"] = day.isContinuation ? "Null" : (day.getupTime.empty() ? "00:00" : day.getupTime);
    headers_obj["activity_count"] = day.activityCount;

    if (!day.generalRemarks.empty()) {
        std::string full_remark;
        for (size_t i = 0; i < day.generalRemarks.size(); ++i) {
            full_remark += day.generalRemarks[i];
            if (i < day.generalRemarks.size() - 1) {
                full_remark += "\n"; 
            }
        }
        headers_obj["remark"] = full_remark;
    } else {
        headers_obj["remark"] = "";
    }
    
    day_obj["headers"] = headers_obj;
    
    json activities = json::array();
    for (const auto& activity_data : day.processedActivities) {
        json activity_obj;
        activity_obj["logical_id"] = activity_data.logical_id;
        activity_obj["start_timestamp"] = activity_data.start_timestamp;
        activity_obj["end_timestamp"] = activity_data.end_timestamp;
        activity_obj["start_time"] = activity_data.start_time_str;
        activity_obj["end_time"] = activity_data.end_time_str;
        activity_obj["duration_seconds"] = activity_data.duration_seconds;

        if (activity_data.remark.has_value()) {
            activity_obj["activity_remark"] = activity_data.remark.value();
        } else {
            activity_obj["activity_remark"] = nullptr; 
        }

        json activity_details;
        activity_details["project_path"] = activity_data.project_path;
        activity_obj["activity"] = activity_details;
        activities.push_back(activity_obj);
    }
    day_obj["activities"] = activities;

    json generated_stats_obj;
    generated_stats_obj["sleep_night_time"] = day.stats.sleep_night_time;
    generated_stats_obj["sleep_day_time"] = day.stats.sleep_day_time;
    generated_stats_obj["sleep_total_time"] = day.stats.sleep_total_time;
    generated_stats_obj["total_exercise_time"] = day.stats.total_exercise_time;
    generated_stats_obj["cardio_time"] = day.stats.cardio_time;
    generated_stats_obj["anaerobic_time"] = day.stats.anaerobic_time;
    generated_stats_obj["grooming_time"] = day.stats.grooming_time;
    generated_stats_obj["toilet_time"] = day.stats.toilet_time;
    generated_stats_obj["gaming_time"] = day.stats.gaming_time;
    generated_stats_obj["recreation_time"] = day.stats.recreation_time;
    generated_stats_obj["recreation_zhihu_time"] = day.stats.recreation_zhihu_time;
    generated_stats_obj["recreation_bilibili_time"] = day.stats.recreation_bilibili_time;
    generated_stats_obj["recreation_douyin_time"] = day.stats.recreation_douyin_time;
    generated_stats_obj["total_study_time"] = day.stats.study_time;
    
    day_obj["generated_stats"] = generated_stats_obj;

    return day_obj;
}

// [Legacy] 批量写入
void Output::write(std::ostream& outputStream, const std::vector<InputData>& days, const ConverterConfig& /*config*/) {
    beginOutput(outputStream);
    bool isFirst = true;
    for (const auto& day : days) {
        if (day.date.empty()) continue;
        writeDay(outputStream, day, isFirst);
        isFirst = false;
    }
    endOutput(outputStream);
}

// [New] 流式写入实现
void Output::beginOutput(std::ostream& outputStream) {
    outputStream << "[\n";
}

void Output::writeDay(std::ostream& outputStream, const InputData& day, bool isFirstDay) {
    if (day.date.empty()) return;
    
    if (!isFirstDay) {
        outputStream << ",\n";
    }
    
    json day_json = convertDayToJson(day);
    outputStream << day_json.dump(4);
}

void Output::endOutput(std::ostream& outputStream) {
    outputStream << "\n]" << std::endl;
}
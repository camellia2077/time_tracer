// converter/convert/pipelines/Output.cpp
#include "Output.hpp"
#include <iostream>
#include <nlohmann/json.hpp>
#include "converter/convert/model/InputData.hpp"
#include "converter/convert/config/ConverterConfig.hpp"
#include <vector>

using json = nlohmann::json;

void Output::write(std::ostream& outputStream, const std::vector<InputData>& days, const ConverterConfig& /*config*/) {
    if (days.empty()) {
        outputStream << "[]" << std::endl;
        return;
    }

    json root_array = json::array();

    for (const auto& day : days) {
        if (day.date.empty()) continue;

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
            // [适配] BaseActivityRecord 字段
            activity_obj["logical_id"] = activity_data.logical_id;
            activity_obj["start_timestamp"] = activity_data.start_timestamp;
            activity_obj["end_timestamp"] = activity_data.end_timestamp;

            activity_obj["start_time"] = activity_data.start_time_str; // [Modified]
            activity_obj["end_time"] = activity_data.end_time_str;     // [Modified]
            activity_obj["duration_seconds"] = activity_data.duration_seconds; // [Modified]

            if (activity_data.remark.has_value()) {
                activity_obj["activity_remark"] = activity_data.remark.value(); // [Modified]
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
        // [适配] GeneratedStats -> ActivityStats (day.stats) 以及字段名
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

        root_array.push_back(day_obj);
    }
     outputStream << root_array.dump(4) << std::endl;
}
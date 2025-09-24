// reprocessing/converter/pipelines/OutputGenerator.cpp
#include "OutputGenerator.hpp"
#include <iostream>
#include <nlohmann/json.hpp>
#include "reprocessing/converter/model/InputData.hpp"
#include "reprocessing/converter/config/ConverterConfig.hpp"
#include <vector>

using json = nlohmann::json;

void OutputGenerator::write(std::ostream& outputStream, const std::vector<InputData>& days, const ConverterConfig& /*config*/) {
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
            headers_obj["remark"] = day.generalRemarks[0];
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

            activity_obj["start_time"] = activity_data.startTime;
            activity_obj["end_time"] = activity_data.endTime;
            activity_obj["duration_seconds"] = activity_data.durationSeconds;

            if (activity_data.activityRemark.has_value()) {
                activity_obj["activity_remark"] = activity_data.activityRemark.value();
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
        generated_stats_obj["sleep_night_time"] = day.generatedStats.sleepNightTime;
        generated_stats_obj["sleep_day_time"] = day.generatedStats.sleepDayTime;
        generated_stats_obj["sleep_total_time"] = day.generatedStats.sleepTotalTime;
        generated_stats_obj["total_exercise_time"] = day.generatedStats.totalExerciseTime;
        generated_stats_obj["cardio_time"] = day.generatedStats.cardioTime;
        generated_stats_obj["anaerobic_time"] = day.generatedStats.anaerobicTime;
        generated_stats_obj["grooming_time"] = day.generatedStats.groomingTime;
        generated_stats_obj["toilet_time"] = day.generatedStats.toiletTime;
        generated_stats_obj["gaming_time"] = day.generatedStats.gamingTime;
        generated_stats_obj["recreation_time"] = day.generatedStats.recreationTime;
        // --- [新增字段] ---
        generated_stats_obj["recreation_zhihu_time"] = day.generatedStats.recreationZhihuTime;
        generated_stats_obj["recreation_bilibili_time"] = day.generatedStats.recreationBilibiliTime;
        generated_stats_obj["recreation_douyin_time"] = day.generatedStats.recreationDouyinTime;
        
        day_obj["generated_stats"] = generated_stats_obj;

        root_array.push_back(day_obj);
    }
     outputStream << root_array.dump(4) << std::endl;
}
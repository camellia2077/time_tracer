// reprocessing/converter/internal/OutputGenerator.cpp
#include "OutputGenerator.hpp"
#include <fstream>
#include <vector>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

void OutputGenerator::write(std::ostream& outputStream, const std::vector<InputData>& days, const ConverterConfig& config) {
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
        headers_obj["sleep"] = static_cast<int>(day.isContinuation ? false : day.endsWithSleepNight);
        headers_obj["getup"] = day.isContinuation ? "Null" : (day.getupTime.empty() ? "00:00" : day.getupTime);
        headers_obj["activityCount"] = day.activityCount;

        if (!day.generalRemarks.empty()) {
            headers_obj["remark"] = day.generalRemarks[0];
        } else {
            headers_obj["remark"] = "";
        }
        
        day_obj["headers"] = headers_obj;
        
        json activities = json::array();
        for (const auto& activity_data : day.processedActivities) {
            json activity_obj;
            
            // --- [核心修正] 新增以下三行 ---
            activity_obj["logicalId"] = activity_data.logical_id;
            activity_obj["startTimestamp"] = activity_data.start_timestamp;
            activity_obj["endTimestamp"] = activity_data.end_timestamp;

            activity_obj["startTime"] = activity_data.startTime;
            activity_obj["endTime"] = activity_data.endTime;
            activity_obj["durationSeconds"] = activity_data.durationSeconds;

            json activity_details;
            activity_details["topParent"] = activity_data.topParent;
            if (!activity_data.parents.empty()) {
                activity_details["parents"] = activity_data.parents;
            }
            
            activity_obj["activity"] = activity_details;
            activities.push_back(activity_obj);
        }
        day_obj["activities"] = activities;

        json generated_stats_obj;
        generated_stats_obj["sleepTime"] = day.generatedStats.sleepTime;
        day_obj["generatedStats"] = generated_stats_obj;


        root_array.push_back(day_obj);
    }

    outputStream << root_array.dump(4) << std::endl;
}
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
        headers_obj["date"] = day.date; // [核心修改]
        headers_obj["status"] = static_cast<int>(day.hasStudyActivity); // [核心修改]
        headers_obj["sleep"] = static_cast<int>(day.isContinuation ? false : day.endsWithSleepNight); // [核心修改]
        headers_obj["getup"] = day.isContinuation ? "Null" : (day.getupTime.empty() ? "00:00" : day.getupTime); // [核心修改]
        headers_obj["activityCount"] = day.activityCount; // [核心修改]

        if (!day.generalRemarks.empty()) {
            headers_obj["remark"] = day.generalRemarks[0]; // [核心修改]
        } else {
            headers_obj["remark"] = ""; // [核心修改]
        }
        
        day_obj["headers"] = headers_obj; // [核心修改]
        
        json activities = json::array();
        for (const auto& activity_data : day.processedActivities) {
            json activity_obj;
            
            activity_obj["startTime"] = activity_data.startTime;
            activity_obj["endTime"] = activity_data.endTime;

            activity_obj["durationSeconds"] = activity_data.durationSeconds;

            json activity_details;
            activity_details["topParent"] = activity_data.topParent; // [核心修改]
            if (!activity_data.parents.empty()) {
                activity_details["parents"] = activity_data.parents;
            }
            
            activity_obj["activity"] = activity_details; // [核心修改]
            activities.push_back(activity_obj);
        }
        day_obj["activities"] = activities; // [核心修改]

        json generated_stats_obj;
        generated_stats_obj["sleepTime"] = day.generatedStats.sleepTime; // [核心修改]
        day_obj["generatedStats"] = generated_stats_obj; // [核心修改]


        root_array.push_back(day_obj);
    }

    outputStream << root_array.dump(4) << std::endl;
}
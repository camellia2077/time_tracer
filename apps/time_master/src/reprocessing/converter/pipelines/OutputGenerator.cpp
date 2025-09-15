// reprocessing/converter/pipelines/OutputGenerator.cpp
#include "OutputGenerator.hpp"
// ... (includes are unchanged) ...
using json = nlohmann::json;

void OutputGenerator::write(std::ostream& outputStream, const std::vector<InputData>& days, const ConverterConfig& /*config*/) {
    // ... (headers and activities loop setup is unchanged) ...
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
            // ... (other activity fields are unchanged) ...
            activity_obj["logicalId"] = activity_data.logical_id;
            activity_obj["startTimestamp"] = activity_data.start_timestamp;
            activity_obj["endTimestamp"] = activity_data.end_timestamp;

            activity_obj["startTime"] = activity_data.startTime;
            activity_obj["endTime"] = activity_data.endTime;
            activity_obj["durationSeconds"] = activity_data.durationSeconds;

            if (activity_data.activityRemark.has_value()) {
                activity_obj["activityRemark"] = activity_data.activityRemark.value();
            } else {
                activity_obj["activityRemark"] = nullptr; 
            }

            json activity_details;
            // [核心修改]
            activity_details["parent"] = activity_data.parent;
            if (!activity_data.children.empty()) {
                activity_details["children"] = activity_data.children;
            }
            
            activity_obj["activity"] = activity_details;
            activities.push_back(activity_obj);
        }
        day_obj["activities"] = activities;

        json generated_stats_obj;
        generated_stats_obj["sleepTime"] = day.generatedStats.sleepTime;
        generated_stats_obj["totalExerciseTime"] = day.generatedStats.totalExerciseTime;
        generated_stats_obj["cardioTime"] = day.generatedStats.cardioTime;
        generated_stats_obj["anaerobicTime"] = day.generatedStats.anaerobicTime;
        generated_stats_obj["groomingTime"] = day.generatedStats.groomingTime;
        generated_stats_obj["toiletTime"] = day.generatedStats.toiletTime;
        generated_stats_obj["gamingTime"] = day.generatedStats.gamingTime;
        day_obj["generatedStats"] = generated_stats_obj;


        root_array.push_back(day_obj);
    }
     outputStream << root_array.dump(4) << std::endl;
}
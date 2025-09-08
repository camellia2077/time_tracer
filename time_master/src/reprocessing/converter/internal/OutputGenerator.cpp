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

        // [核心修改] 保证 "Headers" 对象先被创建和赋值
        // =================================================================
        json headers_obj;
        headers_obj["Date"] = day.date;
        headers_obj["Status"] = day.hasStudyActivity;
        headers_obj["Sleep"] = day.isContinuation ? false : day.endsWithSleepNight;
        headers_obj["Getup"] = day.isContinuation ? "Null" : (day.getupTime.empty() ? "00:00" : day.getupTime);
        
        if (!day.generalRemarks.empty()) {
            headers_obj["Remark"] = day.generalRemarks[0];
        } else {
            headers_obj["Remark"] = "";
        }
        
        day_obj["Headers"] = headers_obj;
        // =================================================================

        // 之后再创建 "Activities" 数组
        json activities = json::array();
        for (const auto& activity_data : day.processedActivities) {
            json activity_obj;
            
            // [核心修改] 保证 "startTime" 在 "endTime" 之前被赋值
            activity_obj["startTime"] = activity_data.startTime;
            activity_obj["endTime"] = activity_data.endTime;

            json activity_details;
            activity_details["top_parents"] = activity_data.title;
            if (!activity_data.parents.empty()) {
                activity_details["parents"] = activity_data.parents;
            }
            
            activity_obj["activity"] = activity_details;
            activities.push_back(activity_obj);
        }
        day_obj["Activities"] = activities;

        root_array.push_back(day_obj);
    }

    outputStream << root_array.dump(4) << std::endl;
}
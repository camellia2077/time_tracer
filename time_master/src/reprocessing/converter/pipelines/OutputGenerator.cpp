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
        headers_obj["Date"] = day.date;
        // 转换成0或1
        headers_obj["Status"] = static_cast<int>(day.hasStudyActivity);
        headers_obj["Sleep"] = static_cast<int>(day.isContinuation ? false : day.endsWithSleepNight);
        headers_obj["Getup"] = day.isContinuation ? "Null" : (day.getupTime.empty() ? "00:00" : day.getupTime);
        headers_obj["ActivityCount"] = day.activityCount; // [新增] 添加计算字段

        if (!day.generalRemarks.empty()) {
            headers_obj["Remark"] = day.generalRemarks[0];
        } else {
            headers_obj["Remark"] = "";
        }
        
        day_obj["Headers"] = headers_obj;
        
        json activities = json::array();
        for (const auto& activity_data : day.processedActivities) {
            json activity_obj;
            
            activity_obj["startTime"] = activity_data.startTime;
            activity_obj["endTime"] = activity_data.endTime;

            // [新增] 添加时长字段到JSON输出
            activity_obj["durationSeconds"] = activity_data.durationSeconds;

            json activity_details;
            activity_details["top_parent"] = activity_data.top_parent;
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
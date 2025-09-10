// db_inserter/parser/JsonDataParser.cpp
#include "JsonDataParser.hpp"
#include <fstream>
#include <iostream>
#include "common/common_utils.hpp" // For time_str_to_seconds
#include "common/AnsiColors.hpp" // For colored console output

using json = nlohmann::json;

bool JsonDataParser::parse_file(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << RED_COLOR << "Error: Cannot open JSON file " << filename << RESET_COLOR << std::endl;
        return false;
    }

    json days_array;
    try {
        file >> days_array;
        if (!days_array.is_array()) {
            std::cerr << RED_COLOR << "Error: JSON root in " << filename << " is not an array." << RESET_COLOR << std::endl;
            return false;
        }
    } catch (const json::parse_error& e) {
        std::cerr << RED_COLOR << "Error parsing JSON from " << filename << ": " << e.what() << RESET_COLOR << std::endl;
        return false;
    }

    try {
        for (const auto& day_json : days_array) {
            parse_day_object(day_json);
        }
    } catch (const std::exception& e) {
        std::cerr << RED_COLOR << "Error processing data in " << filename << ": " << e.what() << RESET_COLOR << std::endl;
        return false;
    }

    return true;
}

void JsonDataParser::parse_day_object(const json& day_json) {
    try {
        const auto& headers = day_json.at("headers");
        const auto& activities_array = day_json.at("activities");

        DayData day_data;
        day_data.date = headers.at("date");
        day_data.status = headers.at("status");
        day_data.sleep = headers.at("sleep");
        day_data.getup_time = headers.value("getup", "00:00");
        day_data.remark = headers.at("remark");

        if (day_data.date.length() == 8) {
            day_data.year = std::stoi(day_data.date.substr(0, 4));
            day_data.month = std::stoi(day_data.date.substr(4, 2));
        } else {
            day_data.year = 0;
            day_data.month = 0;
        }
        days.push_back(day_data);

        if (activities_array.is_array()) {
            for (const auto& activity_json : activities_array) {
                process_activity(activity_json, day_data.date);
            }
        }

    } catch (const json::out_of_range& e) {
        std::string date_str = day_json.value("headers", json::object()).value("date", "[Unknown Date]");
        throw std::runtime_error("Required JSON key not found for date " + date_str + ": " + e.what());
    }
}

void JsonDataParser::process_activity(const json& activity_json, const std::string& date) {
    try {
        TimeRecordInternal record;

        // --- [核心修改] 解析新增的字段 ---
        record.logical_id = activity_json.at("logicalId");
        record.start_timestamp = activity_json.at("startTimestamp");
        record.end_timestamp = activity_json.at("endTimestamp");

        record.date = date;
        record.start = activity_json.at("startTime");
        record.end = activity_json.at("endTime");
        record.duration_seconds = activity_json.at("durationSeconds");
        
        const auto& activity_details = activity_json.at("activity");
        std::string title = activity_details.at("topParent");
        
        std::string project_path = title;

        if (activity_details.contains("parents") && activity_details["parents"].is_array()) {   
            for (const auto& parent_json : activity_details["parents"]) {
                std::string parent_name = parent_json.get<std::string>();
                std::string parent_path = project_path; 
                project_path += "_" + parent_name;      
                parent_child_pairs.insert({project_path, parent_path});
            }
        }

        record.project_path = project_path;
        records.push_back(record);

    } catch (const json::out_of_range& e) {
        std::string date_str = activity_json.value("date", "[Unknown Date]");
        throw std::runtime_error("Required JSON key not found in activity for date " + date_str + ": " + e.what());
    }
}
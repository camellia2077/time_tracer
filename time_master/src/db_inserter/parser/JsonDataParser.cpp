// db_inserter/parser/JsonDataParser.cpp
#include "JsonDataParser.hpp"
#include <fstream>
#include <iostream>
#include "common/common_utils.hpp" // For time_str_to_seconds

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
    if (!day_json.contains("Headers") || !day_json.contains("Activities")) return;

    const auto& headers = day_json["Headers"];
    DayData day_data;
    day_data.date = headers.value("Date", "");
    day_data.status = headers.value("Status", false) ? "True" : "False";
    day_data.sleep = headers.value("Sleep", false) ? "True" : "False";
    day_data.getup_time = headers.value("Getup", "00:00");
    day_data.remark = headers.value("Remark", "");

    if (day_data.date.length() == 8) {
        day_data.year = std::stoi(day_data.date.substr(0, 4));
        day_data.month = std::stoi(day_data.date.substr(4, 2));
    } else {
        day_data.year = 0;
        day_data.month = 0;
    }
    days.push_back(day_data);

    const auto& activities = day_json["Activities"];
    if (activities.is_array()) {
        for (const auto& activity_json : activities) {
            process_activity(activity_json, day_data.date);
        }
    }
}

void JsonDataParser::process_activity(const json& activity_json, const std::string& date) {
    std::string start_time = activity_json.value("startTime", "");
    std::string end_time = activity_json.value("endTime", "");
    
    const auto& activity_details = activity_json.value("activity", json::object());
    std::string title = activity_details.value("title", "unknown");
    
    std::string project_path = title;
    std::string parent_path = title;

    if (activity_details.contains("parents") && activity_details["parents"].is_array()) {
        for (const auto& parent_json : activity_details["parents"]) {
            std::string parent_name = parent_json.get<std::string>();
            std::string child_path = project_path;
            project_path += "_" + parent_name;
            parent_child_pairs.insert({child_path, project_path});
        }
    }
    
    // 我们需要反转路径以匹配旧的数据库结构 `child -> parent`
    // 例如, "game_steam" 的父级是 "game"
    std::vector<std::string> parts = split_string(project_path, '_');
    if(parts.size() > 1){
        for(size_t i = 0; i < parts.size() - 1; ++i){
            std::string child = parts[0];
            for(size_t j = 1; j <= i; ++j) child += "_" + parts[j];
            std::string parent = child + "_" + parts[i+1];
            parent_child_pairs.insert({child, parent});
        }
    }


    TimeRecordInternal record;
    record.date = date;
    record.start = start_time;
    record.end = end_time;
    record.project_path = project_path;
    record.duration_seconds = calculate_duration_seconds(start_time, end_time);
    records.push_back(record);
}

int JsonDataParser::calculate_duration_seconds(const std::string& start_time, const std::string& end_time) {
    int start_seconds = time_str_to_seconds(start_time);
    int end_seconds = time_str_to_seconds(end_time);
    if (end_seconds < start_seconds) {
        return (end_seconds + 24 * 3600) - start_seconds;
    }
    return end_seconds - start_seconds;
}
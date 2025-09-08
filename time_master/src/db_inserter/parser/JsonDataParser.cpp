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
    if (!day_json.contains("Headers") || !day_json.contains("Activities")) return;

    const auto& headers = day_json["Headers"];
    DayData day_data;
    day_data.date = headers.value("Date", "");
    day_data.status = headers.value("Status", 0); // [修改] 直接将JSON值赋给 int
    day_data.sleep = headers.value("Sleep", 0);   // [修改] 直接将JSON值赋给 int
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
    // 注意top_parent需要为单数,以匹配json的命名
    std::string title = activity_details.value("top_parent", "unknown_top_parent");
    
    std::string project_path = title;

    if (activity_details.contains("parents") && activity_details["parents"].is_array()) {
        for (const auto& parent_json : activity_details["parents"]) {
            std::string parent_name = parent_json.get<std::string>();

            // --- 父子关系构建逻辑 ---
            // 在每次迭代中，我们将当前的 `project_path` 视为父级。
            // 例如，如果当前 `project_path` 是 "game"，它就是下一级的父级。
            std::string parent_path = project_path; 

            // 然后，我们将新的部分（如 "steam"）追加到路径末尾，形成一个新的、更具体的子级路径。
            // 例如，`project_path` 变为 "game_steam"。
            project_path += "_" + parent_name;      

            // 我们将这个关系 {子级, 父级} 存入数据库。
            // 例如，对于 "game_steam_overwatch"，数据库会存储以下关系：
            // 1. { "game_steam", "game" }  (game_steam 是 game 的子级)
            // 2. { "game_steam_overwatch", "game_steam" } (game_steam_overwatch 是 game_steam 的子级)
            // 这样就确保了层级关系的正确性：范围大的为父，范围小的为子。
            parent_child_pairs.insert({project_path, parent_path});
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
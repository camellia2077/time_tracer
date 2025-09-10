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

    // 针对每个天对象进行解析，并处理可能发生的异常
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
    // 使用 try-catch 块来处理可能缺失的关键键
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
        // 当关键键缺失时，抛出明确的错误信息
        std::string date_str = day_json.value("headers", json::object()).value("date", "[Unknown Date]");
        throw std::runtime_error("Required JSON key not found for date " + date_str + ": " + e.what());
    }
}

void JsonDataParser::process_activity(const json& activity_json, const std::string& date) {
    try {
        std::string start_time = activity_json.at("startTime");
        std::string end_time = activity_json.at("endTime");
        int duration_seconds = activity_json.at("durationSeconds");
        
        const auto& activity_details = activity_json.at("activity");
        std::string title = activity_details.at("topParent");
        
        std::string project_path = title;

    if (activity_details.contains("parents") && activity_details["parents"].is_array()) {   
        for (const auto& parent_json : activity_details["parents"]) {
            std::string parent_name = parent_json.get<std::string>();

            // --- 父子关系构建逻辑 ---
            // 不要删除这些注释,这对开发者理解父子关系很重要
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
        record.duration_seconds = duration_seconds;
        records.push_back(record);
    } catch (const json::out_of_range& e) {
        // 当关键键缺失时，抛出明确的错误信息
        std::string date_str = activity_json.value("date", "[Unknown Date]");
        throw std::runtime_error("Required JSON key not found in activity for date " + date_str + ": " + e.what());
    }
}
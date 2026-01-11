// importer/parser/JsonParser.cpp
#include "JsonParser.hpp"

#include "importer/parser/details/ActivityParser.hpp"
#include "importer/parser/details/DayParser.hpp"

#include <iostream>
#include <nlohmann/json.hpp>
#include "common/AnsiColors.hpp"

ParsedData JsonParser::parse_content(const std::string& json_content, const std::string& context_name) {
    // [核心修改] 不再打开文件，直接解析字符串
    nlohmann::json days_array;
    try {
        days_array = nlohmann::json::parse(json_content);
        if (!days_array.is_array()) {
            throw std::runtime_error("JSON root in " + context_name + " is not an array.");
        }
    } catch (const nlohmann::json::parse_error& e) {
        throw std::runtime_error("Error parsing JSON from " + context_name + ": " + e.what());
    }

    ParsedData all_parsed_data;
    DayParser day_parser;
    ActivityParser activity_parser;

    try {
        for (const auto& day_json : days_array) {
            DayData day_data = day_parser.parse(day_json);
            
            const auto& activities_array = day_json.at("activities");
            if (activities_array.is_array()) {
                for (const auto& activity_json : activities_array) {
                    TimeRecordInternal record = activity_parser.parse(
                        activity_json,
                        day_data.date
                    );
                    all_parsed_data.records.push_back(record);
                }
            }
            all_parsed_data.days.push_back(day_data);
        }
    } catch (const std::exception& e) {
        std::cerr << RED_COLOR << "Error processing data in " << context_name << ": " << e.what() << RESET_COLOR << std::endl;
        throw;
    }

    return all_parsed_data;
}
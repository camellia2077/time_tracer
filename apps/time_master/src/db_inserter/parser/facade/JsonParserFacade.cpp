// db_inserter/parser/facade/JsonParserFacade.cpp
#include "JsonParserFacade.hpp" // <-- Updated include to match class name

#include "db_inserter/parser/pipelines/ActivityParser.hpp"
#include "db_inserter/parser/pipelines/DayParser.hpp"

#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>
#include "common/AnsiColors.hpp"

// --- [FIX] Updated method to belong to JsonParserFacade ---
ParsedData JsonParserFacade::parse_file(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open JSON file " + filename);
    }

    nlohmann::json days_array;
    try {
        file >> days_array;
        if (!days_array.is_array()) {
            throw std::runtime_error("JSON root in " + filename + " is not an array.");
        }
    } catch (const nlohmann::json::parse_error& e) {
        throw std::runtime_error("Error parsing JSON from " + filename + ": " + e.what());
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
                        day_data.date,
                        all_parsed_data.parent_child_pairs
                    );
                    all_parsed_data.records.push_back(record);
                }
            }
            all_parsed_data.days.push_back(day_data);
        }
    } catch (const std::exception& e) {
        std::cerr << RED_COLOR << "Error processing data in " << filename << ": " << e.what() << RESET_COLOR << std::endl;
        throw;
    }

    return all_parsed_data;
}
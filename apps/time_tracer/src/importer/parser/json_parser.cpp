// importer/parser/json_parser.cpp
#include "json_parser.hpp"
#include "importer/parser/memory_parser.hpp" 
#include "serializer/json_serializer.hpp"

#include <iostream>
#include <map>
#include "common/ansi_colors.hpp"

using namespace serializer;

ParsedData JsonParser::parse_content(const std::string& json_content, const std::string& context_name) {
    try {
        nlohmann::json days_array = nlohmann::json::parse(json_content);
        if (!days_array.is_array()) {
            throw std::runtime_error("JSON root in " + context_name + " is not an array.");
        }

        // 1. JSON -> DailyLog (Using Serializer)
        std::vector<DailyLog> logs = JsonSerializer::deserializeDays(days_array);
        
        // 2. DailyLog -> ParsedData (Using MemoryParser)
        // MemoryParser expects a map<YYYY-MM, vector<DailyLog>>, so we group them first.
        std::map<std::string, std::vector<DailyLog>> grouped_logs;
        for (auto& log : logs) {
            if (log.date.length() >= 7) {
                std::string month_key = log.date.substr(0, 7);
                grouped_logs[month_key].push_back(std::move(log));
            }
        }
        
        MemoryParser memory_parser;
        return memory_parser.parse(grouped_logs);

    } catch (const std::exception& e) {
        std::cerr << RED_COLOR << "Error processing data in " << context_name << ": " << e.what() << RESET_COLOR << std::endl;
        throw;
    }
}
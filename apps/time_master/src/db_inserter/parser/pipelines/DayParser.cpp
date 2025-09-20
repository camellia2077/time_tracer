// db_inserter/parser/pipelines/DayParser.cpp
#include "DayParser.hpp"
#include <stdexcept>

DayData DayParser::parse(const nlohmann::json& day_json) const {
    try {
        const auto& headers = day_json.at("headers");
        const auto& generated_stats = day_json.at("generatedStats");

        DayData day_data;
        day_data.date = headers.at("date");
        day_data.status = headers.at("status");
        day_data.sleep = headers.at("sleep");
        day_data.getup_time = headers.value("getup", "00:00");
        day_data.remark = headers.at("remark");
        day_data.exercise = headers.value("exercise", 0);
        day_data.total_exercise_time = generated_stats.value("totalExerciseTime", 0);
        day_data.cardio_time = generated_stats.value("cardioTime", 0);
        day_data.anaerobic_time = generated_stats.value("anaerobicTime", 0);
        // [删除] day_data.exercise_both_time = generated_stats.value("exerciseBothTime", 0);
        
        // [新增] 解析新的统计数据
        day_data.gaming_time = generated_stats.value("gamingTime", 0);
        day_data.grooming_time = generated_stats.value("groomingTime", 0);
        day_data.toilet_time = generated_stats.value("toiletTime", 0);


        if (day_data.date.length() == 8) {
            day_data.year = std::stoi(day_data.date.substr(0, 4));
            day_data.month = std::stoi(day_data.date.substr(4, 2));
        } else {
            day_data.year = 0;
            day_data.month = 0;
        }
        return day_data;

    } catch (const nlohmann::json::out_of_range& e) {
        std::string date_str = day_json.value("headers", nlohmann::json::object()).value("date", "[Unknown Date]");
        throw std::runtime_error("Required JSON key not found for day " + date_str + ": " + e.what());
    }
}
// importer/parser/details/DayParser.cpp
#include "DayParser.hpp"
#include <stdexcept>

DayData DayParser::parse(const nlohmann::json& day_json) const {
    try {
        const auto& headers = day_json.at("headers");
        const auto& generated_stats = day_json.at("generated_stats");

        DayData day_data;
        // 这里获取到的现在是 "YYYY-MM-DD" 格式
        day_data.date = headers.at("date");
        day_data.status = headers.at("status");
        day_data.sleep = headers.at("sleep");
        day_data.getup_time = headers.value("getup", "00:00");
        day_data.remark = headers.at("remark");
        day_data.exercise = headers.value("exercise", 0);
        day_data.total_exercise_time = generated_stats.value("total_exercise_time", 0);
        day_data.anaerobic_time = generated_stats.value("anaerobic_time", 0);
        day_data.cardio_time = generated_stats.value("cardio_time", 0);
        
        day_data.gaming_time = generated_stats.value("gaming_time", 0);
        day_data.grooming_time = generated_stats.value("grooming_time", 0);
        day_data.toilet_time = generated_stats.value("toilet_time", 0);

        day_data.sleep_night_time = generated_stats.value("sleep_night_time", 0);
        day_data.sleep_day_time = generated_stats.value("sleep_day_time", 0);
        day_data.sleep_total_time = generated_stats.value("sleep_total_time", 0);

        // --- [新增] 解析娱乐时间数据 ---
        day_data.recreation_time = generated_stats.value("recreation_time", 0);
        day_data.recreation_zhihu_time = generated_stats.value("recreation_zhihu_time", 0);
        day_data.recreation_bilibili_time = generated_stats.value("recreation_bilibili_time", 0);
        day_data.recreation_douyin_time = generated_stats.value("recreation_douyin_time", 0);
        
        // [新增] 从 JSON 解析学习时间
        day_data.study_time = generated_stats.value("total_study_time", 0);

        // --- [核心修改] 适配 YYYY-MM-DD 格式 ---
        if (day_data.date.length() == 10) {
            // YYYY-MM-DD
            // 0123456789
            day_data.year = std::stoi(day_data.date.substr(0, 4));
            // 月份从索引 5 开始，取 2 位
            day_data.month = std::stoi(day_data.date.substr(5, 2));
        } else {
            // 如果日期格式不符合预期，保留为0（或者可以抛出异常）
            day_data.year = 0;
            day_data.month = 0;
        }
        return day_data;

    } catch (const nlohmann::json::out_of_range& e) {
        std::string date_str = day_json.value("headers", nlohmann::json::object()).value("date", "[Unknown Date]");
        throw std::runtime_error("Required JSON key not found for day " + date_str + ": " + e.what());
    }
}
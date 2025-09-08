#pragma once
#include <vector>
#include <string>
#include <random>

/**
 * @class EventGenerator
 * @brief 专门负责生成一天内的所有事件。
 */
class EventGenerator {
public:
    EventGenerator(int items_per_day, const std::vector<std::string>& activities, std::mt19937& gen);

    // 生成一天的所有事件
    void generate_events_for_day(std::string& log_content);

private:
    int items_per_day_;
    const std::vector<std::string>& common_activities_;
    std::mt19937& gen_;
    std::uniform_int_distribution<> dis_minute_;
    std::uniform_int_distribution<> dis_activity_selector_;
};
#pragma once
#include "config/Config.h" // 引入Config头文件
#include <vector>
#include <string>
#include <random>
#include <optional>

/**
 * @class EventGenerator
 * @brief 专门负责生成一天内的所有事件。
 */
class EventGenerator {
public:
    EventGenerator(int items_per_day,
                   const std::vector<std::string>& activities,
                   const std::optional<ActivityRemarkConfig>& remark_config, // 新增
                   const std::vector<std::string>& wake_keywords,
                   std::mt19937& gen);

    // 生成一天的所有事件
    void generate_events_for_day(std::string& log_content, bool is_nosleep_day); // 修改

private:
    int items_per_day_;
    const std::vector<std::string>& common_activities_;
    const std::optional<ActivityRemarkConfig>& remark_config_; // 新增
    const std::vector<std::string>& wake_keywords_;
    std::mt19937& gen_;
    std::uniform_int_distribution<> dis_minute_;
    std::uniform_int_distribution<> dis_activity_selector_;
    std::uniform_int_distribution<> dis_wake_keyword_selector_;
    
    // 新增：备注生成相关的成员
    std::bernoulli_distribution should_generate_remark_;
    const std::vector<std::string> remark_delimiters_ = {"//", "#", ";"};
    size_t remark_content_idx_ = 0;
    size_t remark_delimiter_idx_ = 0;
};
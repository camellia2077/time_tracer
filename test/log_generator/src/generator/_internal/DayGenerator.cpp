#include "DayGenerator.h"
#include <format>
#include <iterator>

DayGenerator::DayGenerator(int items_per_day,
                           const std::vector<std::string>& activities,
                           const std::optional<DailyRemarkConfig>& remark_config,
                           const std::optional<ActivityRemarkConfig>& activity_remark_config,
                           const std::vector<std::string>& wake_keywords,
                           std::mt19937& gen)
{
    // 创建并持有子系统的实例
    remark_generator_ = std::make_unique<RemarkGenerator>(remark_config, gen);
    event_generator_ = std::make_unique<EventGenerator>(items_per_day, activities, activity_remark_config, wake_keywords, gen);
}

/**
 * @brief 为单日生成完整的日志条目（日期头、可选的备注和事件列表）。
 * @param log_content 将生成的日志内容追加到的字符串。
 * @param year 年份。
 * @param month 月份。
 * @param day 日期。
 */
void DayGenerator::generate_for_day(std::string& log_content, int year, int month, int day) {
    // 1. 生成日期标题
    std::format_to(std::back_inserter(log_content), "{:02}{:02}\n", month, day);

    // 2. 委托给 RemarkGenerator，尝试生成备注
    if (auto remark = remark_generator_->try_generate()) {
        std::format_to(std::back_inserter(log_content), "{}\n", *remark);
    }

    // 3. 委托给 EventGenerator，生成当天的所有活动
    event_generator_->generate_events_for_day(log_content);
}
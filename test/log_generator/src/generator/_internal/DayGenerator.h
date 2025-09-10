#pragma once
#include "RemarkGenerator.h"
#include "EventGenerator.h"
#include <string>
#include <memory>

/**
 * @class DayGenerator
 * @brief (内部组件) 专门负责生成一天完整的日志内容。
 *
 * 这个类协调 RemarkGenerator 和 EventGenerator，
 * 为指定的一天生成包括日期标题、可选备注和所有事件的完整文本块。
 */
class DayGenerator {
public:
    DayGenerator(int items_per_day,
                 const std::vector<std::string>& activities,
                 const std::optional<DailyRemarkConfig>& remark_config,
                 std::mt19937& gen);

    // 生成一天的完整日志
    void generate_for_day(std::string& log_content, int year, int month, int day);

private:
    // DayGenerator 持有其他子生成器的所有权
    std::unique_ptr<RemarkGenerator> remark_generator_;
    std::unique_ptr<EventGenerator> event_generator_;
};
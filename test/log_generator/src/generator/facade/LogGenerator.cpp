#include "LogGenerator.h"
#include <format>
#include <iterator>

LogGenerator::LogGenerator(int items_per_day,
                         const std::vector<std::string>& activities,
                         const std::optional<DailyRemarkConfig>& remark_config,
                         const std::optional<ActivityRemarkConfig>& activity_remark_config)
    : gen_(std::random_device{}())
{
    day_generator_ = std::make_unique<DayGenerator>(items_per_day, activities, remark_config, activity_remark_config, gen_);
}

// --- [核心修改] 更新 generate_for_month 的逻辑 ---
std::string LogGenerator::generate_for_month(int year, int month, int days_in_month) {
    std::string log_content;
    log_content.reserve((days_in_month + 1) * 280); 

    // 1. 如果是1月份，则首先生成前一年的年份标题和12月31日的内容
    if (month == 1) {
        // 为上一年的数据添加年份标题
        std::format_to(std::back_inserter(log_content), "y{}\n\n", year - 1);
        day_generator_->generate_for_day(log_content, year - 1, 12, 31);
        log_content += '\n';
    }

    // 2. 为当前要生成的月份添加年份标题
    std::format_to(std::back_inserter(log_content), "y{}\n\n", year);

    // 3. 循环委托 DayGenerator 生成当月的所有天
    for (int day = 1; day <= days_in_month; ++day) {
        if (day > 1) {
            log_content += '\n';
        }
        day_generator_->generate_for_day(log_content, year, month, day);
    }
    return log_content;
}
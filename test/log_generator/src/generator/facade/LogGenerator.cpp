#include "LogGenerator.h"
#include <format>
#include <iterator>

// [核心修改] 移除了所有内部类的定义，使该文件只关注 Facade 的实现

LogGenerator::LogGenerator(int items_per_day,
                         const std::vector<std::string>& activities,
                         const std::optional<DailyRemarkConfig>& remark_config)
    : items_per_day_(items_per_day),
      gen_(std::random_device{}())
{
    // Facade 创建并持有所有子系统的实例
    remark_generator_ = std::make_unique<RemarkGenerator>(remark_config, gen_);
    event_generator_ = std::make_unique<EventGenerator>(items_per_day, activities, gen_);
}

std::string LogGenerator::generate_for_month(int year, int month, int days_in_month) {
    std::string log_content;
    log_content.reserve(days_in_month * (items_per_day_ * 25 + 30) + 10);

    // 1. 生成年份标题
    std::format_to(std::back_inserter(log_content), "y{}\n\n", year);

    for (int day = 1; day <= days_in_month; ++day) {
        if (day > 1) {
            log_content += '\n';
        }

        // 2. 生成日期标题
        std::format_to(std::back_inserter(log_content), "{:02}{:02}\n", month, day);

        // 3. 委托给 RemarkGenerator
        if (auto remark = remark_generator_->try_generate()) {
            std::format_to(std::back_inserter(log_content), "{}\n", *remark);
        }

        // 4. 委托给 EventGenerator
        event_generator_->generate_events_for_day(log_content);
    }
    return log_content;
}
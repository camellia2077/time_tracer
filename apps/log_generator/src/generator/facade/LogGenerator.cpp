#include "LogGenerator.h"
#include <format>
#include <iterator>

LogGenerator::LogGenerator(const Config& config,
                         const std::vector<std::string>& activities,
                         const std::optional<DailyRemarkConfig>& remark_config,
                         const std::optional<ActivityRemarkConfig>& activity_remark_config,
                         const std::vector<std::string>& wake_keywords)
    : gen_(std::random_device{}()) // 初始化随机数引擎
{
    // 初始化 DayGenerator
    day_generator_ = std::make_unique<DayGenerator>(
        config.items_per_day, 
        activities, 
        remark_config, 
        activity_remark_config, 
        wake_keywords, 
        gen_
    );
    
    // [新增] 初始化 SleepScheduler，将策略逻辑委托给它
    // 注意：gen_ 的引用传递给了 scheduler，共享同一个随机源
    sleep_scheduler_ = std::make_unique<SleepScheduler>(config.enable_nosleep, gen_);
}

void LogGenerator::generate_for_month(int year, int month, int days_in_month, std::string& buffer) {
    buffer.clear(); // 复用内存
    
    // 确保容量足够，避免频繁 realloc
    size_t needed_capacity = (days_in_month + 1) * 300;
    if (buffer.capacity() < needed_capacity) {
        buffer.reserve(needed_capacity);
    }

    // 处理 1 月份的特殊情况：写入上一年的 12 月 31 日
    if (month == 1) {
        std::format_to(std::back_inserter(buffer), "y{}\n\n", year - 1);
        // 通常除夕/跨年夜不强制通宵，这里传 false，或者也可以询问 scheduler
        day_generator_->generate_for_day(buffer, 12, 31, false); 
        buffer.push_back('\n');
    }

    std::format_to(std::back_inserter(buffer), "y{}\n\n", year);

    // [核心修改] 1. 每个月开始前，重置调度器状态
    // 这取代了原来手动重置 is_in_nosleep_block_ 等变量的代码
    sleep_scheduler_->reset_for_new_month();

    // [核心修改] 2. 生成第 1 天
    // 通过调度器查询是否通宵，替代硬编码
    bool is_nosleep = sleep_scheduler_->determine_if_nosleep(1, days_in_month);
    day_generator_->generate_for_day(buffer, month, 1, is_nosleep);

    // [核心修改] 3. 循环生成剩余天数
    for (int day = 2; day <= days_in_month; ++day) {
        buffer.push_back('\n');

        // 直接询问策略类，无需在此处维护复杂状态
        is_nosleep = sleep_scheduler_->determine_if_nosleep(day, days_in_month);
        
        day_generator_->generate_for_day(buffer, month, day, is_nosleep);
    }
}
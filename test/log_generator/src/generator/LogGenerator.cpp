#include "LogGenerator.h"
#include <format> 
#include <cmath>
#include <iterator>

LogGenerator::LogGenerator(int items_per_day,
                         const std::vector<std::string>& activities,
                         const std::optional<DailyRemarkConfig>& remark_config)
    : items_per_day_(items_per_day),
    common_activities_(activities),
    remark_config_(remark_config),
    gen_(std::random_device{}()),
    dis_minute_(0, 59),
    dis_activity_selector_(0, static_cast<int>(activities.size()) - 1) {
    
        if (remark_config_ && !remark_config_->contents.empty()) {
            dis_remark_selector_ = std::make_unique<std::uniform_int_distribution<>>(
                0, static_cast<int>(remark_config_->contents.size()) - 1
            );
            dis_remark_should_generate_ = std::make_unique<std::bernoulli_distribution>(remark_config_->generation_chance);
        }
    }

// [修改] 函数实现新增 year 参数，并在文件开头添加年份
std::string LogGenerator::generate_for_month(int year, int month, int days_in_month) {
    std::string log_content;
    // 预留空间时，为年份行增加一点额外容量
    log_content.reserve(days_in_month * (items_per_day_ * 25 + 30) + 10); 

    // [核心修改] 在文件最开始处添加年份信息
    std::format_to(std::back_inserter(log_content), "y{}\n\n", year);

    for (int day = 1; day <= days_in_month; ++day) {
        if (day > 1) {
            log_content += '\n';
        }

        std::format_to(std::back_inserter(log_content), "{:02}{:02}\n", month, day);
        
        if (remark_config_ && dis_remark_selector_ && dis_remark_should_generate_ && (*dis_remark_should_generate_)(gen_)) {
            const std::string& random_content = remark_config_->contents[(*dis_remark_selector_)(gen_)];
            std::format_to(std::back_inserter(log_content), "{}{}\n", remark_config_->prefix, random_content);
        }
        
        for (int i = 0; i < items_per_day_; ++i) {
            int display_hour_final;
            int event_minute_final;
            std::string event_text_to_use_final;

            if (i == 0) {
                event_text_to_use_final = "起床";
                display_hour_final = 6;
                event_minute_final = dis_minute_(gen_);
            }
            else {
                double progress_ratio = (items_per_day_ > 1) ? static_cast<double>(i) / (items_per_day_ - 1) : 1.0;
                int logical_event_hour = 6 + static_cast<int>(std::round(progress_ratio * 19.0));
                if (logical_event_hour > 25) logical_event_hour = 25;
                display_hour_final = (logical_event_hour >= 24) ? logical_event_hour - 24 : logical_event_hour;
                event_minute_final = dis_minute_(gen_);
                event_text_to_use_final = common_activities_[dis_activity_selector_(gen_)];
            }
            std::format_to(std::back_inserter(log_content), "{:02}{:02}{}\n", display_hour_final, event_minute_final, event_text_to_use_final);
        }
    }
    return log_content;
}
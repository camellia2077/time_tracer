#include "LogGenerator.h"
#include "Utils.h" // 需要用到 Utils 中的工具函数声明
#include <sstream>
#include <cmath>

LogGenerator::LogGenerator(int items_per_day,
                         const std::vector<std::string>& activities,
                         const std::optional<DailyRemarkConfig>& remark_config)
    : items_per_day_(items_per_day),
    common_activities_(activities),
    remark_config_(remark_config),
    gen_(std::random_device{}()),
    dis_minute_(0, 59),
    dis_activity_selector_(0, static_cast<int>(activities.size()) - 1) {
    
        // Initialize the remark selector and probability distribution only if remarks are configured.
        if (remark_config_ && !remark_config_->contents.empty()) {
            dis_remark_selector_ = std::make_unique<std::uniform_int_distribution<>>(
                0, static_cast<int>(remark_config_->contents.size()) - 1
            );
            // NEW: Initialize a Bernoulli distribution for the generation chance.
            dis_remark_should_generate_ = std::make_unique<std::bernoulli_distribution>(remark_config_->generation_chance);
        }
    }

// 【修正】在这里加上了 LogGenerator:: 来指定这是 LogGenerator 类的成员函数
void LogGenerator::generate_for_month(std::ostream& outStream, int month, int days_in_month) {
    std::ostringstream log_stream;

    for (int day = 1; day <= days_in_month; ++day) {
        if (day > 1) {
            log_stream << '\n';
        }

        log_stream << Utils::format_two_digits(month) << Utils::format_two_digits(day) << '\n';
        
        // MODIFIED: Add the daily remark line if configured AND the probability check passes.
        if (remark_config_ && dis_remark_selector_ && dis_remark_should_generate_ && (*dis_remark_should_generate_)(gen_)) {
            const std::string& random_content = remark_config_->contents[(*dis_remark_selector_)(gen_)];
            log_stream << remark_config_->prefix << random_content << '\n';
        }
        
        // This daily generation logic is preserved from the original version.
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
            log_stream << Utils::format_two_digits(display_hour_final) << Utils::format_two_digits(event_minute_final) << event_text_to_use_final << '\n';
        }
    }
    // Write the generated content for the whole month at once.
    outStream << log_stream.str();
}

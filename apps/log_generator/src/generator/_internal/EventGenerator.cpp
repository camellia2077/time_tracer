#include "EventGenerator.h"
#include <format>
#include <cmath>
#include <iterator>

EventGenerator::EventGenerator(int items_per_day,
                               const std::vector<std::string>& activities,
                               const std::optional<ActivityRemarkConfig>& remark_config,
                               const std::vector<std::string>& wake_keywords,
                               std::mt19937& gen)
    : items_per_day_(items_per_day),
      common_activities_(activities),
      remark_config_(remark_config),
      wake_keywords_(wake_keywords),
      gen_(gen),
      dis_minute_(0, 59),
      dis_activity_selector_(0, static_cast<int>(activities.size()) - 1),
      dis_wake_keyword_selector_(0, static_cast<int>(wake_keywords.size()) -1),
      should_generate_remark_(remark_config.has_value() ? remark_config->generation_chance : 0.0) {}

void EventGenerator::generate_events_for_day(std::string& log_content) {
    for (int i = 0; i < items_per_day_; ++i) {
        int hour;
        int minute = dis_minute_(gen_);
        std::string text;
        
        bool is_wakeup_event = false;

        if (i == 0) {
            // 处理“起床”的特殊情况
            text = wake_keywords_[dis_wake_keyword_selector_(gen_)];
            hour = 6;
            is_wakeup_event = true;
        } else {
            // 处理其他随机事件
            text = common_activities_[dis_activity_selector_(gen_)];
            double progress_ratio = (items_per_day_ > 1) ? static_cast<double>(i) / (items_per_day_ - 1) : 1.0;
            int logical_hour = 6 + static_cast<int>(std::round(progress_ratio * 19.0));
            if (logical_hour > 25) logical_hour = 25;
            hour = (logical_hour >= 24) ? logical_hour - 24 : logical_hour;
        }
        
        // 尝试添加备注
        std::string remark_str;
        if (remark_config_ && !is_wakeup_event && should_generate_remark_(gen_)) {
            const std::string& delimiter = remark_delimiters_[remark_delimiter_idx_];
            const std::string& content = remark_config_->contents[remark_content_idx_];
            remark_str = std::format(" {}{}", delimiter, content);

            // 更新索引以实现循环
            remark_delimiter_idx_ = (remark_delimiter_idx_ + 1) % remark_delimiters_.size();
            remark_content_idx_ = (remark_content_idx_ + 1) % remark_config_->contents.size();
        }

        std::format_to(std::back_inserter(log_content), "{:02}{:02}{}{}\n", hour, minute, text, remark_str);
    }
}
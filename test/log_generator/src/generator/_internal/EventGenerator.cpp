#include "EventGenerator.h"
#include <format>
#include <cmath>
#include <iterator>

EventGenerator::EventGenerator(int items_per_day, const std::vector<std::string>& activities, std::mt19937& gen)
    : items_per_day_(items_per_day), common_activities_(activities), gen_(gen),
      dis_minute_(0, 59),
      dis_activity_selector_(0, static_cast<int>(activities.size()) - 1) {}

void EventGenerator::generate_events_for_day(std::string& log_content) {
    for (int i = 0; i < items_per_day_; ++i) {
        int hour;
        int minute = dis_minute_(gen_);
        std::string text;

        if (i == 0) {
            // 处理“起床”的特殊情况
            text = "起床";
            hour = 6;
        } else {
            // 处理其他随机事件
            text = common_activities_[dis_activity_selector_(gen_)];
            double progress_ratio = (items_per_day_ > 1) ? static_cast<double>(i) / (items_per_day_ - 1) : 1.0;
            int logical_hour = 6 + static_cast<int>(std::round(progress_ratio * 19.0));
            if (logical_hour > 25) logical_hour = 25;
            hour = (logical_hour >= 24) ? logical_hour - 24 : logical_hour;
        }
        std::format_to(std::back_inserter(log_content), "{:02}{:02}{}\n", hour, minute, text);
    }
}
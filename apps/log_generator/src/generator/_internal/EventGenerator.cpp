// generator/_internal/EventGenerator.cpp
#include "EventGenerator.h"
#include <format>
#include <cmath>
#include <iterator>
#include <algorithm> // 为 std::max 添加此头文件

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

void EventGenerator::generate_events_for_day(std::string& log_content, bool is_nosleep_day) {
    // 以总分钟数来跟踪上一个事件的时间，初始值设为第一个可能事件（06:00）之前
    int last_total_minutes = 5 * 60 + 59; 

    for (int i = 0; i < items_per_day_; ++i) {
        int hour;
        int minute;
        std::string text;
        
        bool is_wakeup_event = false;

        if (i == 0 && !is_nosleep_day) {
            // 正常天的第一个事件是“起床”
            text = wake_keywords_[dis_wake_keyword_selector_(gen_)];
            hour = 6;
            minute = dis_minute_(gen_);
            is_wakeup_event = true;

            last_total_minutes = hour * 60 + minute;
        } else {
            // 通宵天或正常天的后续事件
            text = common_activities_[dis_activity_selector_(gen_)];
            
            // 为当前事件计算一个目标时间范围，以保持事件在一天中的大致分布
            double slot_size = (19.0 * 60.0) / items_per_day_; // 活动时间共19小时
            int slot_start = static_cast<int>((6 * 60) + i * slot_size);
            int slot_end = static_cast<int>((6 * 60) + (i + 1) * slot_size) - 1;

            // 随机生成的开始时间必须晚于上一个事件的时间!!!
            int effective_start = std::max(slot_start, last_total_minutes + 1);

            // 确保时间范围的结束点有效
            if (slot_end <= effective_start) {
                slot_end = effective_start + 5; // 给一个小的随机窗口
            }

            // 在计算出的有效时间窗口内生成一个随机时间
            std::uniform_int_distribution<> time_dist(effective_start, slot_end);
            int current_total_minutes = time_dist(gen_);
            
            // 最终保障措施：如果出现任何意外（如取整问题），强制时间递增
            if (current_total_minutes <= last_total_minutes) {
                current_total_minutes = last_total_minutes + 1;
            }
            
            // 将总分钟数转换回小时和分钟
            int logical_hour = current_total_minutes / 60;
            minute = current_total_minutes % 60;
            hour = (logical_hour >= 24) ? logical_hour - 24 : logical_hour;

            last_total_minutes = current_total_minutes;
        }
        
        // 尝试添加备注
        std::string remark_str;
        if (remark_config_ && !is_wakeup_event && should_generate_remark_(gen_)) {
            const std::string& delimiter = remark_delimiters_[remark_delimiter_idx_];
            const std::string& content = remark_config_->contents[remark_content_idx_];
            remark_str = std::format(" {}{}", delimiter, content);

            // 轮换使用分隔符和备注内容
            remark_delimiter_idx_ = (remark_delimiter_idx_ + 1) % remark_delimiters_.size();
            remark_content_idx_ = (remark_content_idx_ + 1) % remark_config_->contents.size();
        }

        std::format_to(std::back_inserter(log_content), "{:02}{:02}{}{}\n", hour, minute, text, remark_str);
    }
}
// domain/components/event_generator.cpp
#include "domain/components/event_generator.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <format>
#include <iterator>

// 移除 Lambda 中的 static 变量，直接通过列表初始化。
// 字符串字面量（如 "00"）本身具有静态存储期，std::string_view
// 在编译期可以直接引用它们。
namespace {
constexpr int kMinutesPerHour = 60;
constexpr int kHoursPerDay = 24;
constexpr int kDayStartHour = 6;
constexpr int kMinMinute = 0;
constexpr int kMaxMinute = kMinutesPerHour - 1;
constexpr int kDayStartTotalMinutes = kDayStartHour * kMinutesPerHour;
constexpr int kLastMinuteBeforeDayStart = kDayStartTotalMinutes - 1;
constexpr int kScheduleWindowHours = 19;
constexpr double kScheduleWindowMinutes =
    static_cast<double>(kScheduleWindowHours * kMinutesPerHour);
constexpr int kMinimumSlotMinutes = 5;

constexpr std::array<std::string_view, 60> kDigits = {
    "00", "01", "02", "03", "04", "05", "06", "07", "08", "09", "10", "11",
    "12", "13", "14", "15", "16", "17", "18", "19", "20", "21", "22", "23",
    "24", "25", "26", "27", "28", "29", "30", "31", "32", "33", "34", "35",
    "36", "37", "38", "39", "40", "41", "42", "43", "44", "45", "46", "47",
    "48", "49", "50", "51", "52", "53", "54", "55", "56", "57", "58", "59"};
}  // namespace

EventGenerator::EventGenerator(
    int items_per_day, const std::vector<std::string>& activities,
    const std::optional<ActivityRemarkConfig>& remark_config,
    const std::vector<std::string>& wake_keywords, std::mt19937& gen)
    : items_per_day_(items_per_day),
      common_activities_(activities),
      remark_config_(remark_config),
      wake_keywords_(wake_keywords),
      gen_(gen),
      dis_minute_(kMinMinute, kMaxMinute),
      dis_activity_selector_(0, static_cast<int>(activities.size()) - 1),
      dis_wake_keyword_selector_(0, static_cast<int>(wake_keywords.size()) - 1),
      should_generate_remark_(
          remark_config.has_value() ? remark_config->generation_chance : 0.0) {
  slot_size_minutes_ = kScheduleWindowMinutes / items_per_day_;
}

void EventGenerator::generate_events_for_day(std::string& log_content,
                                             bool is_nosleep_day) {
  int last_total_minutes = kLastMinuteBeforeDayStart;
  int start_index = 0;

  if (!is_nosleep_day) {
    std::string_view text = wake_keywords_[dis_wake_keyword_selector_(gen_)];
    int hour = kDayStartHour;
    int minute = dis_minute_(gen_);
    last_total_minutes = (hour * kMinutesPerHour) + minute;

    // 使用查找表快速拼接
    log_content.append(kDigits[hour]);
    log_content.append(kDigits[minute]);
    log_content.append(text);
    log_content.push_back('\n');

    start_index = 1;
  }

  for (int i = start_index; i < items_per_day_; ++i) {
    std::string_view text = common_activities_[dis_activity_selector_(gen_)];

    int slot_start =
        static_cast<int>(kDayStartTotalMinutes + (i * slot_size_minutes_));
    int slot_end = static_cast<int>(kDayStartTotalMinutes +
                                    ((i + 1) * slot_size_minutes_)) -
                   1;
    int effective_start = std::max(slot_start, last_total_minutes + 1);

    if (slot_end <= effective_start) {
      slot_end = effective_start + kMinimumSlotMinutes;
    }

    std::uniform_int_distribution<> time_dist(effective_start, slot_end);
    int current_total_minutes = time_dist(gen_);

    if (current_total_minutes <= last_total_minutes) {
      current_total_minutes = last_total_minutes + 1;
    }

    int logical_hour = current_total_minutes / kMinutesPerHour;
    int minute = current_total_minutes % kMinutesPerHour;
    int hour =
        (logical_hour >= kHoursPerDay) ? logical_hour - kHoursPerDay
                                       : logical_hour;

    last_total_minutes = current_total_minutes;

    // 手动拼接，避免 std::format 解析开销
    log_content.append(kDigits[hour]);
    log_content.append(kDigits[minute]);
    log_content.append(text);

    if (remark_config_ && should_generate_remark_(gen_)) {
      std::string_view delimiter = remark_delimiters_[remark_delimiter_idx_];
      std::string_view content = remark_config_->contents[remark_content_idx_];

      log_content.push_back(' ');
      log_content.append(delimiter);
      log_content.append(content);

      remark_delimiter_idx_ =
          (remark_delimiter_idx_ + 1) % remark_delimiters_.size();
      remark_content_idx_ =
          (remark_content_idx_ + 1) % remark_config_->contents.size();
    }

    log_content.push_back('\n');
  }
}

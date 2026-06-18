// domain/components/event_generator.cpp
#include "domain/components/event_generator.hpp"

#include <algorithm>
#include <string_view>
#include <vector>

namespace {
constexpr int kMinutesPerHour = 60;
constexpr int kHoursPerDay = 24;
constexpr int kMinutesPerDay = kHoursPerDay * kMinutesPerHour;
constexpr int kMinMinute = 0;
constexpr int kMaxMinute = kMinutesPerHour - 1;
constexpr int kDefaultWakeHour = 6;
constexpr int kInitialPreviousDayLastHour = 2;
constexpr int kInitialPreviousDayLastMinute = 59;
constexpr int kInitialPreviousDayLastTotalMinutes =
    (kInitialPreviousDayLastHour * kMinutesPerHour) +
    kInitialPreviousDayLastMinute;
constexpr int kBudgetJitterMinutes = 45;
constexpr int kMaxCarryErrorMinutes = 180;
constexpr int kMaxSingleActivityMinutes = 16 * kMinutesPerHour;

}  // namespace

EventGenerator::EventGenerator(
    int items_per_day, const std::vector<std::string>& activities,
    const std::optional<ActivityRemarkConfig>& remark_config,
    const std::vector<std::string>& wake_keywords, EventStyle event_style,
    std::mt19937& gen)
    : items_per_day_(items_per_day),
      event_style_(event_style),
      common_activities_(activities),
      remark_config_(remark_config),
      wake_keywords_(wake_keywords),
      gen_(gen),
      dis_minute_(kMinMinute, kMaxMinute),
      dis_activity_selector_(0, 0),
      dis_wake_keyword_selector_(0, static_cast<int>(wake_keywords.size()) - 1),
      dis_budget_jitter_minutes_(-kBudgetJitterMinutes, kBudgetJitterMinutes),
      should_generate_remark_(
          remark_config.has_value() ? remark_config->generation_chance : 0.0) {
  activity_candidates_.reserve(activities.size());
  for (int index = 0; index < static_cast<int>(activities.size()); ++index) {
    const auto& candidate = activities[static_cast<size_t>(index)];
    const bool is_wake_keyword =
        std::find(wake_keywords.begin(), wake_keywords.end(), candidate) !=
        wake_keywords.end();
    if (!is_wake_keyword) {
      activity_candidates_.push_back(index);
    }
  }

  if (activity_candidates_.empty()) {
    for (int index = 0; index < static_cast<int>(activities.size()); ++index) {
      activity_candidates_.push_back(index);
    }
  }

  dis_activity_selector_ = std::uniform_int_distribution<>(
      0, static_cast<int>(activity_candidates_.size()) - 1);
  reset_for_new_month();
}

void EventGenerator::reset_for_new_month() {
  previous_day_last_minutes_ = kInitialPreviousDayLastTotalMinutes;
  carry_error_minutes_ = 0;
}

auto EventGenerator::to_minute_of_day(int logical_minutes) -> int {
  int minute_of_day = logical_minutes % kMinutesPerDay;
  if (minute_of_day < 0) {
    minute_of_day += kMinutesPerDay;
  }
  return minute_of_day;
}

auto EventGenerator::select_wake_time_minutes(int day_start_minutes,
                                              int day_end_minutes,
                                              int non_wake_event_count) -> int {
  int earliest_wake = day_start_minutes + 1;
  int latest_wake = day_end_minutes - non_wake_event_count;
  latest_wake = std::min(latest_wake,
                         day_start_minutes + kMaxSingleActivityMinutes);
  if (latest_wake < earliest_wake) {
    latest_wake = earliest_wake;
  }

  const int preferred_wake_minute =
      (kDefaultWakeHour * kMinutesPerHour) + dis_minute_(gen_);
  int wake_logical = preferred_wake_minute;
  while (wake_logical <= day_start_minutes) {
    wake_logical += kMinutesPerDay;
  }

  if (wake_logical > latest_wake) {
    std::uniform_int_distribution<> fallback_dist(earliest_wake, latest_wake);
    wake_logical = fallback_dist(gen_);
  }

  return std::clamp(wake_logical, earliest_wake, latest_wake);
}

auto EventGenerator::build_event_minutes(int start_minutes_exclusive,
                                         int end_minutes_inclusive,
                                         int event_count)
    -> std::vector<int> {
  std::vector<int> event_minutes;
  if (event_count <= 0) {
    return event_minutes;
  }
  event_minutes.reserve(static_cast<size_t>(event_count));

  const int minimum_end = start_minutes_exclusive + event_count;
  if (end_minutes_inclusive < minimum_end) {
    end_minutes_inclusive = minimum_end;
  }

  int previous = start_minutes_exclusive;
  for (int index = 0; index < event_count - 1; ++index) {
    const int remaining_after_current = event_count - index - 1;
    int earliest = previous + 1;
    int latest = end_minutes_inclusive - remaining_after_current;

    // Keep every segment no longer than 16 hours while still guaranteeing
    // enough capacity for the remaining events and final endpoint.
    earliest = std::max(
        earliest,
        end_minutes_inclusive -
            (remaining_after_current * kMaxSingleActivityMinutes));
    latest = std::min(latest, previous + kMaxSingleActivityMinutes);
    if (latest < earliest) {
      latest = earliest;
    }

    std::uniform_int_distribution<> time_dist(earliest, latest);
    const int current = time_dist(gen_);
    event_minutes.push_back(current);
    previous = current;
  }

  event_minutes.push_back(end_minutes_inclusive);
  return event_minutes;
}

auto EventGenerator::build_point_event(
    int logical_minutes, std::string_view activity_token,
    std::optional<std::string> remark_suffix) const -> GeneratedEvent {
  const int minute_of_day = to_minute_of_day(logical_minutes);
  return GeneratedEvent{
      .kind = GeneratedEventKind::Point,
      .start_minute = minute_of_day,
      .end_minute = minute_of_day,
      .activity_token = std::string(activity_token),
      .remark_suffix = std::move(remark_suffix),
  };
}

auto EventGenerator::build_interval_event(
    int start_minutes, int end_minutes, std::string_view activity_token,
    std::optional<std::string> remark_suffix) const -> GeneratedEvent {
  return GeneratedEvent{
      .kind = GeneratedEventKind::Interval,
      .start_minute = to_minute_of_day(start_minutes),
      .end_minute = to_minute_of_day(end_minutes),
      .activity_token = std::string(activity_token),
      .remark_suffix = std::move(remark_suffix),
  };
}

auto EventGenerator::maybe_build_remark_suffix() -> std::optional<std::string> {
  if (!remark_config_ || !should_generate_remark_(gen_)) {
    return std::nullopt;
  }

  std::string suffix = " ";
  suffix.append(remark_delimiters_[remark_delimiter_idx_]);
  suffix.append(remark_config_->contents[remark_content_idx_]);

  remark_delimiter_idx_ = (remark_delimiter_idx_ + 1) % remark_delimiters_.size();
  remark_content_idx_ =
      (remark_content_idx_ + 1) % remark_config_->contents.size();
  return suffix;
}

auto EventGenerator::build_day_events(int day_start_minutes, int day_end_minutes,
                                      bool is_nosleep_day)
    -> std::vector<GeneratedEvent> {
  std::vector<GeneratedEvent> events;
  int event_start_minutes = day_start_minutes;
  int non_wake_event_count = items_per_day_;

  events.reserve(static_cast<size_t>(items_per_day_));

  if (!is_nosleep_day) {
    non_wake_event_count = items_per_day_ - 1;
    const int wake_minutes =
        select_wake_time_minutes(day_start_minutes, day_end_minutes,
                                 std::max(0, non_wake_event_count));
    events.push_back(build_point_event(
        wake_minutes, wake_keywords_[dis_wake_keyword_selector_(gen_)],
        std::nullopt));
    event_start_minutes = wake_minutes;
  }

  const auto event_minutes =
      build_event_minutes(event_start_minutes, day_end_minutes,
                          std::max(0, non_wake_event_count));

  for (int minute_index = 0;
       minute_index < static_cast<int>(event_minutes.size()); ++minute_index) {
    const int candidate_index = activity_candidates_[dis_activity_selector_(gen_)];
    events.push_back(build_point_event(
        event_minutes[minute_index],
        common_activities_[static_cast<size_t>(candidate_index)],
        maybe_build_remark_suffix()));
  }

  const int generated_minutes = day_end_minutes - day_start_minutes;
  carry_error_minutes_ += generated_minutes - kMinutesPerDay;
  carry_error_minutes_ =
      std::clamp(carry_error_minutes_, -kMaxCarryErrorMinutes,
                 kMaxCarryErrorMinutes);
  previous_day_last_minutes_ = to_minute_of_day(day_end_minutes);
  return events;
}

auto EventGenerator::build_interval_events(
    int day_start_minutes, int day_end_minutes, bool is_nosleep_day)
    -> std::vector<GeneratedEvent> {
  std::vector<GeneratedEvent> events;
  int interval_start_minutes = day_start_minutes;
  int non_wake_event_count = items_per_day_;

  events.reserve(static_cast<size_t>(items_per_day_));

  if (!is_nosleep_day) {
    non_wake_event_count = items_per_day_ - 1;
    const int wake_minutes =
        select_wake_time_minutes(day_start_minutes, day_end_minutes,
                                 std::max(0, non_wake_event_count));
    events.push_back(build_point_event(
        wake_minutes, wake_keywords_[dis_wake_keyword_selector_(gen_)],
        std::nullopt));
    interval_start_minutes = wake_minutes;
  }

  const auto event_minutes =
      build_event_minutes(interval_start_minutes, day_end_minutes,
                          std::max(0, non_wake_event_count));

  int current_start = interval_start_minutes;
  for (int minute_index = 0;
       minute_index < static_cast<int>(event_minutes.size()); ++minute_index) {
    const int current_end = event_minutes[minute_index];
    const int candidate_index = activity_candidates_[dis_activity_selector_(gen_)];
    events.push_back(build_interval_event(
        current_start, current_end,
        common_activities_[static_cast<size_t>(candidate_index)],
        maybe_build_remark_suffix()));
    current_start = current_end;
  }

  const int generated_minutes = day_end_minutes - day_start_minutes;
  carry_error_minutes_ += generated_minutes - kMinutesPerDay;
  carry_error_minutes_ =
      std::clamp(carry_error_minutes_, -kMaxCarryErrorMinutes,
                 kMaxCarryErrorMinutes);
  previous_day_last_minutes_ = to_minute_of_day(day_end_minutes);
  return events;
}

auto EventGenerator::generate_events_for_day(bool is_nosleep_day)
    -> std::vector<GeneratedEvent> {
  const int day_start_minutes = previous_day_last_minutes_;
  int day_budget_minutes =
      kMinutesPerDay + dis_budget_jitter_minutes_(gen_) - carry_error_minutes_;

  const int minimum_required_budget = std::max(1, items_per_day_);
  if (day_budget_minutes < minimum_required_budget) {
    day_budget_minutes = minimum_required_budget;
  }

  const int day_end_minutes = day_start_minutes + day_budget_minutes;
  if (event_style_ == EventStyle::Interval) {
    return build_interval_events(day_start_minutes, day_end_minutes,
                                 is_nosleep_day);
  }
  return build_day_events(day_start_minutes, day_end_minutes, is_nosleep_day);
}

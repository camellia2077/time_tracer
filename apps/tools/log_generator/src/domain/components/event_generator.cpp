// domain/components/event_generator.cpp
#include "domain/components/event_generator.hpp"

#include <algorithm>
#include <array>
#include <string_view>
#include <vector>

namespace {
constexpr int kMinutesPerHour = 60;
constexpr int kHoursPerDay = 24;
constexpr int kMinutesPerDay = kHoursPerDay * kMinutesPerHour;
constexpr int kSecondsPerMinute = 60;
constexpr int kSecondsPerDay = kMinutesPerDay * kSecondsPerMinute;
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
constexpr int kDefaultBedtimeHour = 23;
constexpr int kDefaultSleepDurationHours = 7;

}  // namespace

EventGenerator::EventGenerator(
    int items_per_day, const std::vector<ActivityTokenVariant>& activities,
    const std::optional<ActivityRemarkConfig>& remark_config,
    const std::vector<std::string>& wake_keywords, EventStyle event_style,
    bool enable_explicit_sleep, std::mt19937& gen)
    : items_per_day_(items_per_day),
      event_style_(event_style),
      enable_explicit_sleep_(enable_explicit_sleep),
      common_activities_(activities),
      remark_config_(remark_config),
      wake_keywords_(wake_keywords),
      gen_(gen),
      dis_minute_(kMinMinute, kMaxMinute),
      dis_second_(0, kSecondsPerMinute - 1),
      dis_activity_selector_(0, 0),
      dis_remark_content_selector_(
          0, remark_config.has_value()
                 ? std::max(0, static_cast<int>(remark_config->contents.size()) - 1)
                 : 0),
      dis_remark_lines_(1, remark_config.has_value()
                               ? std::max(1, remark_config->max_lines)
                               : 1),
      dis_wake_keyword_selector_(0, static_cast<int>(wake_keywords.size()) - 1),
      dis_budget_jitter_minutes_(-kBudgetJitterMinutes, kBudgetJitterMinutes),
      should_generate_mixed_point_(0.5),
      should_generate_explicit_sleep_(0.5),
      should_use_canonical_token_(0.5),
      should_generate_remark_(remark_config.has_value() ? 0.5 : 0.0) {
  activity_candidates_.reserve(activities.size());
  for (int index = 0; index < static_cast<int>(activities.size()); ++index) {
    const auto& candidate = activities[static_cast<size_t>(index)];
    const bool is_wake_keyword =
        std::find(wake_keywords.begin(), wake_keywords.end(),
                  candidate.alias_token) !=
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
  after_explicit_sleep_ = false;
  forced_wake_seconds_.reset();
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

auto EventGenerator::take_wake_seconds(int day_start_minutes,
                                       int day_end_minutes,
                                       int non_wake_event_count) -> int {
  if (forced_wake_seconds_.has_value()) {
    const int wake_seconds = *forced_wake_seconds_;
    forced_wake_seconds_.reset();
    return wake_seconds;
  }
  return select_wake_time_minutes(day_start_minutes, day_end_minutes,
                                  non_wake_event_count) *
             kSecondsPerMinute +
         dis_second_(gen_);
}

auto EventGenerator::build_activity_start_seconds(int start_minutes,
                                                  int end_minutes,
                                                  int event_count,
                                                  bool include_initial_start,
                                                  std::optional<int>
                                                      initial_start_seconds)
    -> std::vector<int> {
  std::vector<int> activity_starts;
  if (event_count <= 0) {
    return activity_starts;
  }
  activity_starts.reserve(static_cast<size_t>(event_count));

  const int generated_start_count =
      event_count - (include_initial_start ? 1 : 0);
  if (include_initial_start) {
    activity_starts.push_back(initial_start_seconds.value_or(
        start_minutes * kSecondsPerMinute + dis_second_(gen_)));
  }
  if (generated_start_count <= 0) {
    return activity_starts;
  }

  // Build wall-clock candidates from the logical day window. The logical
  // day may cross midnight, so grouping uses the normalized minute of day.
  std::array<std::vector<int>, 4> candidates_by_period;
  // Never reuse the boundary minute: for a wake day this prevents an activity
  // from being emitted before the wake second, and for a nosleep day it keeps
  // the first event after the previous logical-day boundary.
  const int first_candidate = start_minutes + 1;
  const int last_candidate = end_minutes - 1;
  for (int minute = first_candidate; minute <= last_candidate; ++minute) {
    const int minute_of_day = to_minute_of_day(minute);
    candidates_by_period[static_cast<size_t>(minute_of_day / 360)]
        .push_back(minute);
  }

  const int base_count = generated_start_count / 4;
  const int remainder = generated_start_count % 4;
  std::array<int, 4> requested_counts{};
  for (int period = 0; period < 4; ++period) {
    requested_counts[static_cast<size_t>(period)] =
        base_count + (period < remainder ? 1 : 0);
  }

  // A normal logical day overlaps all four periods. If a very small custom
  // window does not, move unavailable slots to periods with spare candidates.
  int unassigned_count = 0;
  for (int period = 0; period < 4; ++period) {
    auto& candidates = candidates_by_period[static_cast<size_t>(period)];
    std::shuffle(candidates.begin(), candidates.end(), gen_);
    const int available = static_cast<int>(candidates.size());
    if (requested_counts[static_cast<size_t>(period)] > available) {
      unassigned_count +=
          requested_counts[static_cast<size_t>(period)] - available;
      requested_counts[static_cast<size_t>(period)] = available;
    }
  }
  for (int period = 0; period < 4 && unassigned_count > 0; ++period) {
    auto& candidates = candidates_by_period[static_cast<size_t>(period)];
    const int available = static_cast<int>(candidates.size());
    const int spare = available - requested_counts[static_cast<size_t>(period)];
    const int added = std::min(spare, unassigned_count);
    requested_counts[static_cast<size_t>(period)] += added;
    unassigned_count -= added;
  }

  for (int period = 0; period < 4; ++period) {
    const auto& candidates = candidates_by_period[static_cast<size_t>(period)];
    const int count = requested_counts[static_cast<size_t>(period)];
    for (int index = 0; index < count; ++index) {
      activity_starts.push_back(
          candidates[static_cast<size_t>(index)] * kSecondsPerMinute +
          dis_second_(gen_));
    }
  }
  std::sort(activity_starts.begin() + (include_initial_start ? 1 : 0),
            activity_starts.end());
  return activity_starts;
}

auto ToSecondOfDay(int logical_seconds) -> int {
  int second_of_day = logical_seconds % kSecondsPerDay;
  if (second_of_day < 0) {
    second_of_day += kSecondsPerDay;
  }
  return second_of_day;
}

auto EventGenerator::build_point_event(
    int logical_seconds, std::string_view activity_token,
    std::optional<std::string> remark_suffix) const -> GeneratedEvent {
  const int second_of_day = ToSecondOfDay(logical_seconds);
  return GeneratedEvent{
      .kind = GeneratedEventKind::Point,
      .start_minute = second_of_day / kSecondsPerMinute,
      .end_minute = second_of_day / kSecondsPerMinute,
      .start_second_of_day = second_of_day,
      .end_second_of_day = second_of_day,
      .activity_token = std::string(activity_token),
      .remark_suffix = std::move(remark_suffix),
  };
}

auto EventGenerator::build_interval_event(
    int start_seconds, int end_seconds, std::string_view activity_token,
    std::optional<std::string> remark_suffix) const -> GeneratedEvent {
  const int start_second_of_day = ToSecondOfDay(start_seconds);
  const int end_second_of_day = ToSecondOfDay(end_seconds);
  return GeneratedEvent{
      .kind = GeneratedEventKind::Interval,
      .start_minute = start_second_of_day / kSecondsPerMinute,
      .end_minute = end_second_of_day / kSecondsPerMinute,
      .start_second_of_day = start_second_of_day,
      .end_second_of_day = end_second_of_day,
      .activity_token = std::string(activity_token),
      .remark_suffix = std::move(remark_suffix),
  };
}

auto EventGenerator::maybe_build_remark_suffix() -> std::optional<std::string> {
  if (!remark_config_ || remark_config_->contents.empty() ||
      !should_generate_remark_(gen_)) {
    return std::nullopt;
  }

  const int line_count = dis_remark_lines_(gen_);
  std::string suffix = " // ";
  for (int line_index = 0; line_index < line_count; ++line_index) {
    if (line_index > 0) {
      suffix += "\n// ";
    }
    suffix += remark_config_->contents[
        static_cast<size_t>(dis_remark_content_selector_(gen_))];
  }
  return suffix;
}

auto EventGenerator::resolve_activity_token(const ActivityTokenVariant& activity)
    -> std::string_view {
  if (!activity.canonical_token.empty() && should_use_canonical_token_(gen_)) {
    return activity.canonical_token;
  }
  return activity.alias_token;
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
    const int wake_seconds = take_wake_seconds(
        day_start_minutes, day_end_minutes, std::max(0, non_wake_event_count));
    const int wake_minutes = wake_seconds / kSecondsPerMinute;
    events.push_back(build_point_event(
        wake_seconds, wake_keywords_[dis_wake_keyword_selector_(gen_)],
        std::nullopt));
    event_start_minutes = wake_minutes;
  }

  const auto activity_starts = build_activity_start_seconds(
      event_start_minutes, day_end_minutes,
      std::max(0, non_wake_event_count), false);

  for (const int activity_start : activity_starts) {
    const int candidate_index = activity_candidates_[dis_activity_selector_(gen_)];
    const auto& activity = common_activities_[static_cast<size_t>(candidate_index)];
    events.push_back(build_point_event(
        activity_start,
        resolve_activity_token(activity),
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
  std::optional<int> initial_start_seconds;

  events.reserve(static_cast<size_t>(items_per_day_));

  if (!is_nosleep_day) {
    non_wake_event_count = items_per_day_ - 1;
    const int wake_seconds = take_wake_seconds(
        day_start_minutes, day_end_minutes, std::max(0, non_wake_event_count));
    const int wake_minutes = wake_seconds / kSecondsPerMinute;
    events.push_back(build_point_event(
        wake_seconds, wake_keywords_[dis_wake_keyword_selector_(gen_)],
        std::nullopt));
    interval_start_minutes = wake_minutes;
    initial_start_seconds = wake_seconds + 1;
  }

  const auto activity_starts = build_activity_start_seconds(
      interval_start_minutes, day_end_minutes,
      std::max(0, non_wake_event_count), true, initial_start_seconds);

  for (int index = 0; index < static_cast<int>(activity_starts.size()); ++index) {
    const int current_start = activity_starts[static_cast<size_t>(index)];
    const int current_end =
        index + 1 < static_cast<int>(activity_starts.size())
            ? activity_starts[static_cast<size_t>(index + 1)]
            : day_end_minutes * kSecondsPerMinute;
    const int candidate_index = activity_candidates_[dis_activity_selector_(gen_)];
    const auto& activity = common_activities_[static_cast<size_t>(candidate_index)];
    events.push_back(build_interval_event(
        current_start, current_end, resolve_activity_token(activity),
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

auto EventGenerator::build_mixed_events(int day_start_minutes,
                                        int day_end_minutes,
                                        bool is_nosleep_day)
    -> std::vector<GeneratedEvent> {
  std::vector<GeneratedEvent> events;
  int segment_start_minutes = day_start_minutes;
  int non_wake_event_count = items_per_day_;
  std::optional<int> initial_start_seconds;

  events.reserve(static_cast<size_t>(items_per_day_));

  if (!is_nosleep_day) {
    non_wake_event_count = items_per_day_ - 1;
    const int wake_seconds = take_wake_seconds(
        day_start_minutes, day_end_minutes, std::max(0, non_wake_event_count));
    const int wake_minutes = wake_seconds / kSecondsPerMinute;
    events.push_back(build_point_event(
        wake_seconds, wake_keywords_[dis_wake_keyword_selector_(gen_)],
        std::nullopt));
    segment_start_minutes = wake_minutes;
    initial_start_seconds = wake_seconds + 1;
  }

  const auto activity_starts = build_activity_start_seconds(
      segment_start_minutes, day_end_minutes,
      std::max(0, non_wake_event_count), true, initial_start_seconds);

  for (int index = 0; index < static_cast<int>(activity_starts.size()); ++index) {
    const int current_start = activity_starts[static_cast<size_t>(index)];
    const int current_end =
        index + 1 < static_cast<int>(activity_starts.size())
            ? activity_starts[static_cast<size_t>(index + 1)]
            : day_end_minutes * kSecondsPerMinute;
    const int candidate_index = activity_candidates_[dis_activity_selector_(gen_)];
    const auto& activity =
        common_activities_[static_cast<size_t>(candidate_index)];
    const auto activity_token = resolve_activity_token(activity);
    auto remark_suffix = maybe_build_remark_suffix();

    if (should_generate_mixed_point_(gen_)) {
      events.push_back(
          build_point_event(current_start + 1, activity_token,
                            std::move(remark_suffix)));
    } else {
      events.push_back(build_interval_event(
          current_start, current_end, activity_token, std::move(remark_suffix)));
    }
  }

  const int generated_minutes = day_end_minutes - day_start_minutes;
  carry_error_minutes_ += generated_minutes - kMinutesPerDay;
  carry_error_minutes_ =
      std::clamp(carry_error_minutes_, -kMaxCarryErrorMinutes,
                 kMaxCarryErrorMinutes);
  previous_day_last_minutes_ = to_minute_of_day(day_end_minutes);
  return events;
}

auto EventGenerator::build_explicit_sleep_events()
    -> std::vector<GeneratedEvent> {
  std::vector<GeneratedEvent> events;
  int wake_minutes = 6 * kMinutesPerHour + dis_minute_(gen_);
  int wake_second = dis_second_(gen_);
  if (forced_wake_seconds_.has_value()) {
    wake_minutes = *forced_wake_seconds_ / kSecondsPerMinute;
    wake_second = *forced_wake_seconds_ % kSecondsPerMinute;
    forced_wake_seconds_.reset();
  }
  int bedtime_minutes = kDefaultBedtimeHour * kMinutesPerHour +
                        dis_minute_(gen_);
  if (bedtime_minutes <= wake_minutes) {
    bedtime_minutes += kMinutesPerDay;
  }

  const int activity_count = std::max(0, items_per_day_ - 2);
  const auto activity_starts = build_activity_start_seconds(
      wake_minutes, bedtime_minutes, activity_count, false);
  events.reserve(static_cast<size_t>(activity_count + 2));

  events.push_back(build_point_event(
      wake_minutes * kSecondsPerMinute + wake_second,
      wake_keywords_[dis_wake_keyword_selector_(gen_)], std::nullopt));

  for (const int activity_start : activity_starts) {
    const int candidate_index =
        activity_candidates_[dis_activity_selector_(gen_)];
    const auto& activity =
        common_activities_[static_cast<size_t>(candidate_index)];
    events.push_back(build_point_event(
        activity_start, resolve_activity_token(activity),
        maybe_build_remark_suffix()));
  }

  const int sleep_end_minutes = bedtime_minutes +
                                (kDefaultSleepDurationHours * kMinutesPerHour);
  const int sleep_start_second =
      bedtime_minutes * kSecondsPerMinute + dis_second_(gen_);
  const int sleep_end_second =
      sleep_end_minutes * kSecondsPerMinute + dis_second_(gen_);
  events.push_back(build_interval_event(
      sleep_start_second, sleep_end_second, "sleep_day", std::nullopt));

  // The next generated day starts from the end of this explicit interval.
  // Keeping this boundary in generator state makes the following wake point
  // agree with the authored sleep interval instead of creating a second,
  // overlapping overnight segment.
  previous_day_last_minutes_ = to_minute_of_day(sleep_end_minutes);
  carry_error_minutes_ = 0;
  after_explicit_sleep_ = true;
  forced_wake_seconds_ = ToSecondOfDay(sleep_end_second);
  return events;
}

auto EventGenerator::generate_events_for_day(bool is_nosleep_day)
    -> std::vector<GeneratedEvent> {
  if (enable_explicit_sleep_ && !is_nosleep_day &&
      should_generate_explicit_sleep_(gen_)) {
    return build_explicit_sleep_events();
  }

  const int day_start_minutes = previous_day_last_minutes_;
  if (after_explicit_sleep_) {
    // The first day after an authored overnight interval must not generate
    // point events in the interval's 00:00-06:00 tail. Generate only through
    // the next natural midnight, then return to the ordinary 24-hour window.
    const int day_end_minutes =
        day_start_minutes +
        (kMinutesPerDay - to_minute_of_day(day_start_minutes));
    after_explicit_sleep_ = false;
    if (event_style_ == EventStyle::Interval) {
      return build_interval_events(day_start_minutes, day_end_minutes,
                                   is_nosleep_day);
    }
    if (event_style_ == EventStyle::Mixed) {
      return build_mixed_events(day_start_minutes, day_end_minutes,
                                is_nosleep_day);
    }
    return build_day_events(day_start_minutes, day_end_minutes,
                            is_nosleep_day);
  }

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
  if (event_style_ == EventStyle::Mixed) {
    return build_mixed_events(day_start_minutes, day_end_minutes,
                              is_nosleep_day);
  }
  return build_day_events(day_start_minutes, day_end_minutes, is_nosleep_day);
}

#include <array>
#include <algorithm>
#include <iostream>
#include <random>
#include <string>
#include <vector>

#include "common/config_types.hpp"
#include "domain/components/event_generator.hpp"
#include "domain/formatting/event_line_formatter.hpp"
#include "domain/model/generated_event.hpp"

namespace {

auto ExpectEqual(const std::string& actual, const std::string& expected,
                 const char* case_name) -> bool {
  if (actual == expected) {
    return true;
  }

  std::cerr << "[FAIL] " << case_name << "\n";
  std::cerr << "  expected: " << expected << "\n";
  std::cerr << "  actual  : " << actual << "\n";
  return false;
}

auto ExpectTrue(bool condition, const char* case_name) -> bool {
  if (condition) {
    return true;
  }

  std::cerr << "[FAIL] " << case_name << "\n";
  return false;
}

auto MinuteOfDay(const GeneratedEvent& event) -> int {
  return event.start_minute;
}

auto HasStrictlyIncreasingLogicalStarts(
    const std::vector<GeneratedEvent>& events) -> bool {
  int previous_second = -1;
  bool crossed_midnight = false;
  for (const auto& event : events) {
    if (event.start_second_of_day < 0) {
      return false;
    }
    const int current_second = event.start_second_of_day;
    if (previous_second >= 0 && current_second <= previous_second) {
      if (crossed_midnight) {
        return false;
      }
      crossed_midnight = true;
    }
    previous_second = current_second;
  }
  return true;
}

auto CountNonWakeEventsByPeriod(const std::vector<GeneratedEvent>& events,
                                const std::vector<std::string>& wake_keywords)
    -> std::array<int, 4> {
  std::array<int, 4> counts{};
  for (const auto& event : events) {
    bool is_wake = false;
    for (const auto& wake_keyword : wake_keywords) {
      if (event.activity_token == wake_keyword) {
        is_wake = true;
        break;
      }
    }
    if (is_wake) {
      continue;
    }
    ++counts[static_cast<size_t>(MinuteOfDay(event) / (6 * 60))];
  }
  return counts;
}

}  // namespace

auto main() -> int {
  bool all_passed = true;

  all_passed &= ExpectEqual(
      EventLineFormatter::format_point_event_line(8 * 60 + 13, "o",
                                                  std::nullopt),
      "081300o", "default point rendering uses HHMMSS");

  all_passed &= ExpectEqual(
      EventLineFormatter::format_point_event_line(6 * 60 + 6, "w",
                                                  std::string(" //wake")),
      "060600w //wake", "default point rendering keeps zero seconds");

  all_passed &= ExpectEqual(
      EventLineFormatter::format_interval_event_line(
          0, (2 * 60) + 12, "ow-rank-tracer", std::string(" //remark")),
      "000000-021200ow-rank-tracer //remark",
      "default interval rendering uses HHMMSS");

  all_passed &= ExpectEqual(
      EventLineFormatter::format_point_event_line(
          8 * 60 + 13, "o", std::nullopt, TimeFormat::Hhmmss),
      "081300o", "HHMMSS point rendering appends zero seconds");

  all_passed &= ExpectEqual(
      EventLineFormatter::format_point_event_line_seconds(
          (8 * 60 + 13) * 60 + 27, "o", std::nullopt, TimeFormat::Hhmmss),
      "081327o", "HHMMSS rendering keeps generated seconds");

  all_passed &= ExpectEqual(
      EventLineFormatter::format_interval_event_line(
          0, (2 * 60) + 12, "ow-rank-tracer", std::nullopt,
          TimeFormat::Hhmmss),
      "000000-021200ow-rank-tracer",
      "HHMMSS interval rendering appends zero seconds");

  GeneratedEvent point_event{
      .kind = GeneratedEventKind::Point,
      .start_minute = 8 * 60,
      .end_minute = 8 * 60 + 13,
      .activity_token = "o",
      .remark_suffix = std::nullopt,
  };
  std::string point_buffer;
  EventLineFormatter::append_formatted_event(point_buffer, point_event);
  all_passed &= ExpectEqual(point_buffer, "081300o",
                            "append_formatted_event defaults to HHMMSS");

  GeneratedEvent interval_event{
      .kind = GeneratedEventKind::Interval,
      .start_minute = 9 * 60,
      .end_minute = (10 * 60) + 30,
      .activity_token = "study",
      .remark_suffix = std::string(" // focus"),
  };
  std::string interval_buffer;
  EventLineFormatter::append_formatted_event(interval_buffer, interval_event);
  all_passed &= ExpectEqual(interval_buffer, "090000-103000study // focus",
                            "append_formatted_event defaults to HHMMSS");

  GeneratedEvent multiline_event{
      .kind = GeneratedEventKind::Point,
      .start_minute = 9 * 60,
      .end_minute = 9 * 60,
      .activity_token = "study",
      .remark_suffix = std::string(" // first\n// second"),
  };
  std::string multiline_buffer;
  EventLineFormatter::append_formatted_event(multiline_buffer,
                                             multiline_event);
  all_passed &= ExpectEqual(
      multiline_buffer, "090000study // first\n// second",
      "event rendering keeps physical // continuation lines");

  all_passed &= ExpectTrue(!point_event.remark_suffix.has_value(),
                           "GeneratedEvent supports missing remark_suffix");
  all_passed &= ExpectTrue(interval_event.remark_suffix.has_value(),
                           "GeneratedEvent supports present remark_suffix");

  GeneratedEvent wake_then_interval{
      .kind = GeneratedEventKind::Point,
      .start_minute = 6 * 60 + 6,
      .end_minute = 6 * 60 + 6,
      .activity_token = "w",
      .remark_suffix = std::nullopt,
  };
  std::string mixed_buffer;
  EventLineFormatter::append_formatted_event(mixed_buffer, wake_then_interval);
  mixed_buffer.push_back('\n');
  EventLineFormatter::append_formatted_event(mixed_buffer, interval_event);
  all_passed &= ExpectEqual(mixed_buffer,
                            "060600w\n090000-103000study // focus",
                            "default mixed rendering uses HHMMSS");

  const std::vector<ActivityTokenVariant> activities = {
      {.alias_token = "study", .canonical_token = "routine_study"},
      {.alias_token = "rest", .canonical_token = "recovery_rest"},
      {.alias_token = "w", .canonical_token = "wake"},
  };
  const std::vector<std::string> wake_keywords = {"w"};
  const ActivityRemarkConfig remark_config{
      .contents = {"remark-a", "remark-b"},
      .max_lines = 4,
  };

  std::mt19937 gen_a(123);
  std::mt19937 gen_b(123);
  std::mt19937 gen_c(456);

  EventGenerator seeded_a(4, activities, remark_config, wake_keywords,
                          EventStyle::Interval, true, gen_a);
  EventGenerator seeded_b(4, activities, remark_config, wake_keywords,
                          EventStyle::Interval, true, gen_b);
  EventGenerator seeded_c(4, activities, remark_config, wake_keywords,
                          EventStyle::Interval, true, gen_c);

  const auto events_a = seeded_a.generate_events_for_day(false);
  const auto events_b = seeded_b.generate_events_for_day(false);
  const auto events_c = seeded_c.generate_events_for_day(false);

  std::string rendered_a;
  std::string rendered_b;
  std::string rendered_c;
  for (const auto& event : events_a) {
    EventLineFormatter::append_formatted_event(rendered_a, event);
    rendered_a.push_back('\n');
  }
  for (const auto& event : events_b) {
    EventLineFormatter::append_formatted_event(rendered_b, event);
    rendered_b.push_back('\n');
  }
  for (const auto& event : events_c) {
    EventLineFormatter::append_formatted_event(rendered_c, event);
    rendered_c.push_back('\n');
  }

  all_passed &= ExpectEqual(rendered_a, rendered_b,
                            "same seed yields identical rendered events");
  all_passed &= ExpectTrue(rendered_a != rendered_c,
                           "different seeds may yield different rendered events");

  std::mt19937 mixed_gen(123);
  EventGenerator mixed_generator(24, activities, remark_config, wake_keywords,
                                 EventStyle::Mixed, true, mixed_gen);
  const auto mixed_events = mixed_generator.generate_events_for_day(false);
  int mixed_point_count = 0;
  int mixed_interval_count = 0;
  bool mixed_wake_is_point = false;
  for (const auto& event : mixed_events) {
    if (event.kind == GeneratedEventKind::Point) {
      ++mixed_point_count;
    }
    if (event.kind == GeneratedEventKind::Interval) {
      ++mixed_interval_count;
    }
    if ((event.activity_token == "w" || event.activity_token == "wake") &&
        event.kind == GeneratedEventKind::Point) {
      mixed_wake_is_point = true;
    }
  }
  all_passed &= ExpectTrue(mixed_wake_is_point,
                           "mixed generation keeps wake as point event");
  all_passed &= ExpectTrue(mixed_point_count > 1 && mixed_interval_count > 0,
                           "mixed generation emits point and interval events");

  std::mt19937 distribution_gen(321);
  EventGenerator distribution_generator(16, activities, remark_config,
                                        wake_keywords, EventStyle::Point,
                                        true, distribution_gen);
  const auto distributed_events =
      distribution_generator.generate_events_for_day(false);
  const auto period_counts =
      CountNonWakeEventsByPeriod(distributed_events, wake_keywords);
  const int min_period_count =
      *std::min_element(period_counts.begin(), period_counts.end());
  const int max_period_count =
      *std::max_element(period_counts.begin(), period_counts.end());
  all_passed &= ExpectTrue(distributed_events.size() == 16U,
                           "uniform distribution preserves daily item count");
  all_passed &= ExpectTrue(max_period_count - min_period_count <= 1,
                           "uniform distribution balances four time periods");
  all_passed &= ExpectTrue(
      HasStrictlyIncreasingLogicalStarts(distributed_events),
      "point generation keeps strictly increasing second-level starts");
  bool saw_non_zero_second = false;
  for (const auto& event : distributed_events) {
    saw_non_zero_second |= event.start_second_of_day % 60 != 0;
  }
  all_passed &= ExpectTrue(saw_non_zero_second,
                           "generated HHMMSS starts include random seconds");

  std::mt19937 interval_distribution_gen(654);
  EventGenerator interval_distribution_generator(
      16, activities, remark_config, wake_keywords, EventStyle::Interval,
      true, interval_distribution_gen);
  const auto distributed_intervals =
      interval_distribution_generator.generate_events_for_day(false);
  all_passed &= ExpectTrue(distributed_intervals.size() == 16U,
                           "uniform interval distribution preserves item count");
  for (const auto& event : distributed_intervals) {
    if (event.kind != GeneratedEventKind::Interval) {
      continue;
    }
    const int duration = (event.end_minute - event.start_minute + (24 * 60)) %
                         (24 * 60);
    all_passed &= ExpectTrue(duration > 0,
                             "uniform interval events remain non-empty");
  }
  all_passed &= ExpectTrue(
      HasStrictlyIncreasingLogicalStarts(distributed_intervals),
      "interval generation keeps strictly increasing second-level starts");

  std::mt19937 mixed_order_gen(987);
  EventGenerator mixed_order_generator(16, activities, remark_config,
                                       wake_keywords, EventStyle::Mixed,
                                       true, mixed_order_gen);
  all_passed &= ExpectTrue(
      HasStrictlyIncreasingLogicalStarts(
          mixed_order_generator.generate_events_for_day(false)),
      "mixed generation keeps strictly increasing second-level starts");

  std::mt19937 remark_gen(789);
  EventGenerator remark_generator(4, activities, remark_config, wake_keywords,
                                  EventStyle::Point, true, remark_gen);
  bool saw_remark = false;
  bool saw_missing_remark = false;
  int generated_remark_count = 0;
  int missing_remark_count = 0;
  for (int day = 0; day < 32; ++day) {
    for (const auto& event : remark_generator.generate_events_for_day(false)) {
      if (!event.remark_suffix.has_value()) {
        saw_missing_remark = true;
        ++missing_remark_count;
        continue;
      }
      saw_remark = true;
      ++generated_remark_count;
      const std::string& suffix = *event.remark_suffix;
      size_t line_count = 1;
      for (size_t position = 0; position < suffix.size(); ++position) {
        if (suffix[position] == '\n') {
          ++line_count;
        }
      }
      all_passed &= ExpectTrue(line_count >= 1 && line_count <= 4,
                               "activity remarks contain 1 to 4 physical lines");
      all_passed &= ExpectTrue(suffix.starts_with(" // "),
                               "activity remarks start with //");
      all_passed &= ExpectTrue(suffix.find("\n// ") != std::string::npos ||
                                   line_count == 1,
                               "activity remark continuation lines use //");
    }
  }
  if (!(saw_remark && saw_missing_remark)) {
    std::cerr << "[INFO] generated remarks=" << generated_remark_count
              << ", missing remarks=" << missing_remark_count << "\n";
  }
  all_passed &= ExpectTrue(saw_remark && saw_missing_remark,
                           "activity remark generation uses a 50 percent decision");

  if (!all_passed) {
    return 1;
  }

  std::cout << "[PASS] log_generator_unit_tests\n";
  return 0;
}

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

}  // namespace

auto main() -> int {
  bool all_passed = true;

  all_passed &= ExpectEqual(
      EventLineFormatter::format_point_event_line(8 * 60 + 13, "o",
                                                  std::nullopt),
      "0813o", "format_point_event_line without remark");

  all_passed &= ExpectEqual(
      EventLineFormatter::format_point_event_line(6 * 60 + 6, "w",
                                                  std::string(" //wake")),
      "0606w //wake", "format_point_event_line with wake token remark");

  all_passed &= ExpectEqual(
      EventLineFormatter::format_interval_event_line(
          0, (2 * 60) + 12, "ow-rank-tracer", std::string(" //remark")),
      "0000-0212ow-rank-tracer //remark",
      "format_interval_event_line with remark");

  GeneratedEvent point_event{
      .kind = GeneratedEventKind::Point,
      .start_minute = 8 * 60,
      .end_minute = 8 * 60 + 13,
      .activity_token = "o",
      .remark_suffix = std::nullopt,
  };
  std::string point_buffer;
  EventLineFormatter::append_formatted_event(point_buffer, point_event);
  all_passed &= ExpectEqual(point_buffer, "0813o",
                            "append_formatted_event dispatches point");

  GeneratedEvent interval_event{
      .kind = GeneratedEventKind::Interval,
      .start_minute = 9 * 60,
      .end_minute = (10 * 60) + 30,
      .activity_token = "study",
      .remark_suffix = std::string(" #focus"),
  };
  std::string interval_buffer;
  EventLineFormatter::append_formatted_event(interval_buffer, interval_event);
  all_passed &= ExpectEqual(interval_buffer, "0900-1030study #focus",
                            "append_formatted_event dispatches interval");

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
  all_passed &= ExpectEqual(mixed_buffer, "0606w\n0900-1030study #focus",
                            "wake point plus non-wake interval rendering");

  const std::vector<std::string> activities = {"study", "rest", "w"};
  const std::vector<std::string> wake_keywords = {"w"};
  const ActivityRemarkConfig remark_config{
      .contents = {"remark-a", "remark-b"},
      .generation_chance = 0.7,
  };

  std::mt19937 gen_a(123);
  std::mt19937 gen_b(123);
  std::mt19937 gen_c(456);

  EventGenerator seeded_a(4, activities, remark_config, wake_keywords,
                          EventStyle::Interval, gen_a);
  EventGenerator seeded_b(4, activities, remark_config, wake_keywords,
                          EventStyle::Interval, gen_b);
  EventGenerator seeded_c(4, activities, remark_config, wake_keywords,
                          EventStyle::Interval, gen_c);

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

  if (!all_passed) {
    return 1;
  }

  std::cout << "[PASS] log_generator_unit_tests\n";
  return 0;
}

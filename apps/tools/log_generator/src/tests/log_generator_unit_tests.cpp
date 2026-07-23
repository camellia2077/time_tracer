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

  std::mt19937 mixed_gen(123);
  EventGenerator mixed_generator(24, activities, remark_config, wake_keywords,
                                 EventStyle::Mixed, mixed_gen);
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

  std::mt19937 remark_gen(789);
  EventGenerator remark_generator(4, activities, remark_config, wake_keywords,
                                  EventStyle::Point, remark_gen);
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

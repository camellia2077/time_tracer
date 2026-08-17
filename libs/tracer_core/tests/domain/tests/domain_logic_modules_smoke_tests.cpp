import tracer.core.domain;

#include <algorithm>
#include <iostream>
#include <map>
#include <set>
#include <string>
#include <string_view>
#include <type_traits>
#include <vector>

namespace {

using tracer::core::domain::model::ActivityRecordKind;
using tracer::core::domain::model::BaseActivityRecord;
using tracer::core::domain::model::DailyLog;
using tracer::core::domain::model::RawEvent;
using tracer::core::domain::model::SourceSpan;
using tracer::core::domain::modlogic::converter::DayProcessor;
using tracer::core::domain::modlogic::converter::LogLinker;
using tracer::core::domain::modlogic::converter::LogProcessingResult;
using tracer::core::domain::modlogic::converter::LogProcessor;
using tracer::core::domain::modlogic::validator_common::Diagnostic;
using tracer::core::domain::modlogic::validator_common::DiagnosticSeverity;
using tracer::core::domain::modlogic::validator_common::Error;
using tracer::core::domain::modlogic::validator_common::ErrorType;
using tracer::core::domain::modlogic::validator_structure::StructValidator;
using tracer::core::domain::modlogic::validator_txt::LineRules;
using tracer::core::domain::modlogic::validator_txt::StructureRules;
using tracer::core::domain::modlogic::validator_txt::TextValidator;
using tracer::core::domain::types::ConverterConfig;
using tracer::core::domain::types::DateCheckMode;

using RawEventKindType = decltype(RawEvent{}.kind);

auto Expect(bool condition, std::string_view message, int& failures) -> void {
  if (condition) {
    return;
  }
  ++failures;
  std::cerr << "[FAIL] " << message << '\n';
}

auto BuildTestConfig() -> ConverterConfig {
  ConverterConfig config;
  config.text_mapping["Clash Royale"] = "recreation_game_clash-royale";
  config.text_mapping["study"] = "study";
  config.text_mapping["sleep"] = "sleep";
  config.text_mapping["wake"] = "wake";
  config.sleep_inference.wake_keywords = {"wake"};
  return config;
}

void TestConverterBridge(int& failures) {
  ConverterConfig config = BuildTestConfig();
  DayProcessor processor(config);
  LogLinker linker(config);

  DailyLog previous_day;
  previous_day.date = "2026-03-01";
  RawEvent prev_event;
  prev_event.endTimeStr = "23:30:00";
  previous_day.rawEvents.push_back(prev_event);

  DailyLog day_to_process;
  day_to_process.date = "2026-03-02";
  day_to_process.getupTime = "07:00:00";
  processor.Process(previous_day, day_to_process);

  std::map<std::string, std::vector<DailyLog>> data_map;
  DailyLog first_day;
  first_day.date = "2026-03-01";
  first_day.getupTime = "06:45:00";
  data_map["2026-03"].push_back(first_day);

  LogLinker::ExternalPreviousEvent external_previous_event{"2026-02-28",
                                                           "23:45:00"};
  linker.LinkFirstDayWithExternalPreviousEvent(data_map,
                                               external_previous_event);
  const DailyLog& linked_day = data_map["2026-03"].front();
  Expect(!linked_day.processedActivities.empty(),
         "Linked day should contain generated sleep activity.", failures);

  LogProcessingResult processing_result;
  Expect(processing_result.success,
         "LogProcessingResult default success should be true.", failures);
  Expect(std::is_class_v<LogProcessor>,
         "LogProcessor type should be visible through module bridge.",
         failures);
}

void TestValidatorBridge(int& failures) {
  ConverterConfig config = BuildTestConfig();
  LineRules line_rules(config);

  Expect(LineRules::IsYear("y2026"), "LineRules::IsYear bridge mismatch.",
         failures);
  Expect(LineRules::IsMonth("m03"), "LineRules::IsMonth bridge mismatch.",
         failures);
  Expect(LineRules::IsDate("d0301"), "LineRules::IsDate bridge mismatch.",
         failures);
  Expect(!LineRules::IsDate("0301"),
         "Bare MMDD should no longer be a date marker.", failures);
  Expect(line_rules.IsRemark("// note"), "LineRules::IsRemark bridge mismatch.",
         failures);

  SourceSpan span;
  span.file_path = "module-smoke.txt";
  span.line_start = 4;
  span.line_end = 4;
  span.column_start = 1;
  span.column_end = 8;
  span.raw_text = "0730study";

  std::set<Error> valid_errors;
  const bool valid_event =
      line_rules.IsValidEventLine("0730study", 4, valid_errors, span);
  Expect(valid_event && valid_errors.empty(),
         "Known activity should pass without validation errors.", failures);

  std::set<Error> canonical_errors;
  const bool canonical_event = line_rules.IsValidEventLine(
      "0900recreation_game_clash-royale", 5, canonical_errors, span);
  Expect(canonical_event && canonical_errors.empty(),
         "Canonical activity token should pass without validation errors.",
         failures);

  std::set<Error> unknown_errors;
  const bool unknown_event =
      line_rules.IsValidEventLine("0900unknown", 6, unknown_errors, span);
  Expect(unknown_event,
         "Unknown activity should remain a structural-valid line.", failures);
  Expect(!unknown_errors.empty(),
         "Unknown activity should be insightsed as semantic error.", failures);
  Expect(!unknown_errors.empty() &&
             unknown_errors.begin()->type == ErrorType::kUnrecognizedActivity,
         "Unknown activity error type mismatch.", failures);

  std::set<Error> interval_errors;
  const bool interval_event = line_rules.IsValidEventLine(
      "0900-1030study // focus", 7, interval_errors, span);
  Expect(interval_event && interval_errors.empty(),
         "Interval activity should pass structural line validation.", failures);

  std::set<Error> bad_interval_errors;
  const bool bad_interval_event = line_rules.IsValidEventLine(
      "0900-2460study", 8, bad_interval_errors, span);
  Expect(!bad_interval_event,
         "Interval activity with invalid end time should fail line validation.",
         failures);

  StructureRules structure_rules;
  std::set<Error> structure_errors;
  structure_rules.ProcessYearLine(1, "y2026", structure_errors, span);
  structure_rules.ProcessMonthLine(2, "m03", structure_errors, span);
  structure_rules.ProcessDateLine(3, "d0301", structure_errors, span);
  structure_rules.ProcessEventLine(4, "0730study", structure_errors, span);
  Expect(structure_rules.HasSeenYear(),
         "StructureRules should track year header state.", failures);
  Expect(structure_rules.HasSeenMonth(),
         "StructureRules should track month header state.", failures);

  TextValidator text_validator(config);
  std::set<Error> text_errors;
  const bool text_ok = text_validator.Validate(
      "module-smoke.txt", "y2026\nm03\nd0301\n0730study\n0800sleep\n",
      text_errors);
  Expect(text_ok && text_errors.empty(),
         "TextValidator should pass a minimal valid text sample.", failures);

  std::set<Error> canonical_text_errors;
  const bool canonical_text_ok = text_validator.Validate(
      "module-smoke-canonical.txt",
      "y2026\nm03\nd0301\n0700wake\n0900recreation_game_clash-royale\n",
      canonical_text_errors);
  Expect(canonical_text_ok && canonical_text_errors.empty(),
         "TextValidator should accept canonical activity tokens in source TXT.",
         failures);

  Diagnostic diagnostic;
  diagnostic.code = "module.smoke";
  Expect(diagnostic.severity == DiagnosticSeverity::kError,
         "Diagnostic default severity mismatch.", failures);
}

void TestStructureValidatorBridge(int& failures) {
  StructValidator struct_validator(DateCheckMode::kNone, {"wake"});

  DailyLog day;
  day.date = "2026-03-01";

  BaseActivityRecord activity_one;
  activity_one.start_time_str = "07:00:00";
  activity_one.end_time_str = "07:30:00";
  activity_one.project_path = "study";
  activity_one.duration_seconds = 30 * 60;

  BaseActivityRecord activity_two;
  activity_two.start_time_str = "08:00:00";
  activity_two.end_time_str = "08:20:00";
  activity_two.project_path = "exercise";
  activity_two.duration_seconds = 20 * 60;

  day.processedActivities.push_back(activity_one);
  day.processedActivities.push_back(activity_two);

  std::vector<DailyLog> days;
  days.push_back(day);
  std::vector<Diagnostic> diagnostics;
  const bool ok =
      struct_validator.Validate("module-smoke.txt", days, diagnostics);
  Expect(ok, "StructValidator should pass for valid activity data.", failures);
  Expect(diagnostics.empty(),
         "StructValidator diagnostics should be empty for valid sample.",
         failures);

  DailyLog end_only_day;
  end_only_day.date = "2026-03-02";
  BaseActivityRecord end_only_activity =
      BaseActivityRecord::MakeEndOnly("09:00:00", "study");
  end_only_day.processedActivities.push_back(end_only_activity);

  std::vector<DailyLog> end_only_days{end_only_day};
  std::vector<Diagnostic> end_only_diagnostics;
  const bool end_only_ok = struct_validator.Validate(
      "module-smoke.txt", end_only_days, end_only_diagnostics);
  Expect(end_only_ok, "StructValidator should allow an end-only activity.",
         failures);
  Expect(end_only_diagnostics.empty(),
         "End-only activity should not insights zero-duration diagnostics.",
         failures);

  DailyLog invalid_end_only_day;
  invalid_end_only_day.date = "2026-03-03";
  invalid_end_only_day.processedActivities.push_back(
      BaseActivityRecord::MakeEndOnly("", "study"));
  std::vector<Diagnostic> invalid_end_only_diagnostics;
  const bool invalid_end_only_ok = struct_validator.Validate(
      "module-smoke.txt", {invalid_end_only_day}, invalid_end_only_diagnostics);
  Expect(!invalid_end_only_ok,
         "StructValidator should reject an end-only activity without an end.",
         failures);
  Expect(std::ranges::any_of(invalid_end_only_diagnostics,
                             [](const Diagnostic& diagnostic) {
                               return diagnostic.code ==
                                      "activity.record.invalid_boundary_shape";
                             }),
         "Invalid end-only shape should have a dedicated diagnostic.",
         failures);

  DailyLog interval_gap_day;
  interval_gap_day.date = "2026-03-01";
  interval_gap_day.rawEvents.push_back(
      RawEvent{.kind = RawEventKindType::Interval,
               .startTimeStr = std::string("09:00:00"),
               .endTimeStr = "10:30:00",
               .description = "study",
               .source_span = SourceSpan{.file_path = "module-smoke.txt",
                                         .line_start = 10,
                                         .line_end = 10,
                                         .column_start = 1,
                                         .column_end = 14,
                                         .raw_text = "0900-1030study"}});
  interval_gap_day.rawEvents.push_back(
      RawEvent{.kind = RawEventKindType::Interval,
               .startTimeStr = std::string("14:01:00"),
               .endTimeStr = "19:00:00",
               .description = "sleep",
               .source_span = SourceSpan{.file_path = "module-smoke.txt",
                                         .line_start = 11,
                                         .line_end = 11,
                                         .column_start = 1,
                                         .column_end = 14,
                                         .raw_text = "1401-1900sleep"}});

  std::vector<DailyLog> interval_gap_days{interval_gap_day};
  std::vector<Diagnostic> interval_gap_diagnostics;
  const bool interval_gap_ok = struct_validator.Validate(
      "module-smoke.txt", interval_gap_days, interval_gap_diagnostics);
  Expect(interval_gap_ok,
         "StructValidator should allow gaps between explicit intervals.",
         failures);
  Expect(interval_gap_diagnostics.empty(),
         "Gaps between explicit intervals should not emit diagnostics.",
         failures);

  StructValidator full_mode_validator(DateCheckMode::kFull, {"wake"});
  DailyLog sparse_interval_day;
  sparse_interval_day.date = "2026-03-05";
  sparse_interval_day.rawEvents.push_back(
      RawEvent{.kind = RawEventKindType::Interval,
               .startTimeStr = std::string("08:03:00"),
               .endTimeStr = "09:07:00",
               .description = "study",
               .source_span = SourceSpan{.file_path = "module-smoke.txt",
                                         .line_start = 12,
                                         .line_end = 12,
                                         .column_start = 1,
                                         .column_end = 14,
                                         .raw_text = "0803-0907study"}});

  std::vector<DailyLog> sparse_interval_days{interval_gap_day,
                                             sparse_interval_day};
  std::vector<Diagnostic> sparse_interval_diagnostics;
  const bool sparse_interval_ok = full_mode_validator.Validate(
      "module-smoke.txt", sparse_interval_days, sparse_interval_diagnostics);
  Expect(sparse_interval_ok,
         "Date continuity checks should not reject sparse interval months.",
         failures);
  Expect(sparse_interval_diagnostics.empty(),
         "Sparse interval months should not emit date continuity diagnostics.",
         failures);

  DailyLog invalid_day;
  invalid_day.date = "2026-03-02";
  invalid_day.rawEvents.push_back(
      RawEvent{.endTimeStr = "07:00:00", .description = "study"});
  invalid_day.rawEvents.push_back(
      RawEvent{.endTimeStr = "08:00:00",
               .description = "wake",
               .source_span = SourceSpan{.file_path = "module-smoke.txt",
                                         .line_start = 2,
                                         .line_end = 2,
                                         .column_start = 1,
                                         .column_end = 8,
                                         .raw_text = "0800wake"}});

  std::vector<DailyLog> invalid_days{invalid_day};
  std::vector<Diagnostic> invalid_diagnostics;
  const bool invalid_ok = struct_validator.Validate(
      "module-smoke.txt", invalid_days, invalid_diagnostics);
  Expect(!invalid_ok,
         "StructValidator should fail when wake keyword is not first event.",
         failures);
  Expect(!invalid_diagnostics.empty() &&
             invalid_diagnostics.front().code == "wake.keyword.not_first_event",
         "Wake keyword ordering should surface dedicated logic diagnostic.",
         failures);

  DailyLog overlap_day;
  overlap_day.date = "2026-03-03";
  overlap_day.getupTime = "06:06:00";
  overlap_day.rawEvents.push_back(
      RawEvent{.kind = RawEventKindType::Interval,
               .startTimeStr = std::string("09:00:00"),
               .endTimeStr = "10:30:00",
               .description = "study",
               .source_span = SourceSpan{.file_path = "module-smoke.txt",
                                         .line_start = 3,
                                         .line_end = 3,
                                         .column_start = 1,
                                         .column_end = 14,
                                         .raw_text = "0900-1030study"}});
  overlap_day.rawEvents.push_back(
      RawEvent{.endTimeStr = "10:00:00",
               .description = "sleep",
               .source_span = SourceSpan{.file_path = "module-smoke.txt",
                                         .line_start = 4,
                                         .line_end = 4,
                                         .column_start = 1,
                                         .column_end = 9,
                                         .raw_text = "1000sleep"}});

  std::vector<DailyLog> overlap_days{overlap_day};
  std::vector<Diagnostic> overlap_diagnostics;
  const bool overlap_ok = struct_validator.Validate(
      "module-smoke.txt", overlap_days, overlap_diagnostics);
  Expect(!overlap_ok,
         "StructValidator should fail when a point event overlaps an interval.",
         failures);
  Expect(std::any_of(overlap_diagnostics.begin(), overlap_diagnostics.end(),
                     [](const Diagnostic& diagnostic) -> bool {
                       return diagnostic.code == "timeline.event.overlap";
                     }),
         "Mixed timeline overlap should surface a dedicated logic diagnostic.",
         failures);

  DailyLog interval_overlap_day;
  interval_overlap_day.date = "2026-03-07";
  interval_overlap_day.rawEvents.push_back(
      RawEvent{.kind = RawEventKindType::Interval,
               .startTimeStr = std::string("09:54:00"),
               .endTimeStr = "10:07:00",
               .description = "study",
               .source_span = SourceSpan{.file_path = "module-smoke.txt",
                                         .line_start = 13,
                                         .line_end = 13,
                                         .column_start = 1,
                                         .column_end = 14,
                                         .raw_text = "0954-1007study"}});
  interval_overlap_day.rawEvents.push_back(
      RawEvent{.kind = RawEventKindType::Interval,
               .startTimeStr = std::string("09:54:00"),
               .endTimeStr = "14:09:00",
               .description = "sleep",
               .source_span = SourceSpan{.file_path = "module-smoke.txt",
                                         .line_start = 14,
                                         .line_end = 14,
                                         .column_start = 1,
                                         .column_end = 14,
                                         .raw_text = "0954-1409sleep"}});

  std::vector<DailyLog> interval_overlap_days{interval_overlap_day};
  std::vector<Diagnostic> interval_overlap_diagnostics;
  const bool interval_overlap_ok = struct_validator.Validate(
      "module-smoke.txt", interval_overlap_days, interval_overlap_diagnostics);
  Expect(!interval_overlap_ok,
         "StructValidator should reject overlapping explicit intervals.",
         failures);
  Expect(std::any_of(interval_overlap_diagnostics.begin(),
                     interval_overlap_diagnostics.end(),
                     [](const Diagnostic& diagnostic) -> bool {
                       return diagnostic.code == "timeline.event.overlap";
                     }),
         "Overlapping explicit intervals should insights overlap diagnostic.",
         failures);

  DailyLog interval_wake_day;
  interval_wake_day.date = "2026-03-04";
  interval_wake_day.rawEvents.push_back(
      RawEvent{.kind = RawEventKindType::Interval,
               .startTimeStr = std::string("06:00:00"),
               .endTimeStr = "07:00:00",
               .description = "wake",
               .source_span = SourceSpan{.file_path = "module-smoke.txt",
                                         .line_start = 5,
                                         .line_end = 5,
                                         .column_start = 1,
                                         .column_end = 13,
                                         .raw_text = "0600-0700wake"}});

  std::vector<DailyLog> interval_wake_days{interval_wake_day};
  std::vector<Diagnostic> interval_wake_diagnostics;
  const bool interval_wake_ok = struct_validator.Validate(
      "module-smoke.txt", interval_wake_days, interval_wake_diagnostics);
  Expect(!interval_wake_ok,
         "StructValidator should fail when wake is authored as an interval.",
         failures);
  Expect(std::any_of(
             interval_wake_diagnostics.begin(), interval_wake_diagnostics.end(),
             [](const Diagnostic& diagnostic) -> bool {
               return diagnostic.code == "wake.keyword.interval_not_allowed";
             }),
         "Interval wake should insights a dedicated wake interval diagnostic.",
         failures);

  DailyLog wrapped_interval_day;
  wrapped_interval_day.date = "2026-03-05";
  wrapped_interval_day.isContinuation = true;
  wrapped_interval_day.rawEvents.push_back(
      RawEvent{.kind = RawEventKindType::Interval,
               .startTimeStr = std::string("21:32:00"),
               .endTimeStr = "01:35:00",
               .description = "study",
               .source_span = SourceSpan{.file_path = "module-smoke.txt",
                                         .line_start = 6,
                                         .line_end = 6,
                                         .column_start = 1,
                                         .column_end = 14,
                                         .raw_text = "2132-0135study"}});
  wrapped_interval_day.rawEvents.push_back(
      RawEvent{.kind = RawEventKindType::Interval,
               .startTimeStr = std::string("01:35:00"),
               .endTimeStr = "02:17:00",
               .description = "sleep",
               .source_span = SourceSpan{.file_path = "module-smoke.txt",
                                         .line_start = 7,
                                         .line_end = 7,
                                         .column_start = 1,
                                         .column_end = 14,
                                         .raw_text = "0135-0217sleep"}});
  wrapped_interval_day.rawEvents.push_back(
      RawEvent{.kind = RawEventKindType::Interval,
               .startTimeStr = std::string("02:17:00"),
               .endTimeStr = "02:39:00",
               .description = "study",
               .source_span = SourceSpan{.file_path = "module-smoke.txt",
                                         .line_start = 8,
                                         .line_end = 8,
                                         .column_start = 1,
                                         .column_end = 14,
                                         .raw_text = "0217-0239study"}});

  std::vector<DailyLog> wrapped_interval_days{wrapped_interval_day};
  std::vector<Diagnostic> wrapped_interval_diagnostics;
  const bool wrapped_interval_ok = struct_validator.Validate(
      "module-smoke.txt", wrapped_interval_days, wrapped_interval_diagnostics);
  Expect(wrapped_interval_ok,
         "StructValidator should allow monotonic wrapped cross-midnight "
         "intervals.",
         failures);
  Expect(wrapped_interval_diagnostics.empty(),
         "Wrapped interval chain should not emit diagnostics.", failures);

  ConverterConfig converter_config = BuildTestConfig();
  DayProcessor processor(converter_config);

  DailyLog cross_midnight_point_too_long_day;
  cross_midnight_point_too_long_day.date = "2026-03-11";
  cross_midnight_point_too_long_day.isContinuation = true;
  cross_midnight_point_too_long_day.rawEvents.push_back(
      RawEvent{.kind = RawEventKindType::Interval,
               .startTimeStr = std::string("21:32:00"),
               .endTimeStr = "01:35:00",
               .description = "sleep",
               .source_span = SourceSpan{.file_path = "module-smoke.txt",
                                         .line_start = 19,
                                         .line_end = 19,
                                         .column_start = 1,
                                         .column_end = 14,
                                         .raw_text = "2132-0135sleep"}});
  cross_midnight_point_too_long_day.rawEvents.push_back(
      RawEvent{.kind = RawEventKindType::Point,
               .endTimeStr = "23:50:00",
               .description = "study",
               .source_span = SourceSpan{.file_path = "module-smoke.txt",
                                         .line_start = 20,
                                         .line_end = 20,
                                         .column_start = 1,
                                         .column_end = 9,
                                         .raw_text = "2350study"}});
  DailyLog previous_for_cross_midnight_point;
  previous_for_cross_midnight_point.date = "2026-03-10";
  processor.Process(previous_for_cross_midnight_point,
                    cross_midnight_point_too_long_day);

  Expect(cross_midnight_point_too_long_day.processedActivities.size() == 2,
         "Point event after cross-midnight interval should still produce a "
         "derived activity.",
         failures);
  if (cross_midnight_point_too_long_day.processedActivities.size() == 2) {
    const auto& trailing_activity =
        cross_midnight_point_too_long_day.processedActivities.back();
    Expect(trailing_activity.start_time_str == "01:35:00" &&
               trailing_activity.end_time_str == "23:50:00",
           "Point event after cross-midnight interval should start at the "
           "previous interval end.",
           failures);
    Expect(trailing_activity.duration_seconds == ((22 * 60) + 15) * 60,
           "Point event after cross-midnight interval should not be "
           "reinterpreted as same-day 23:50.",
           failures);
  }

  std::vector<DailyLog> cross_midnight_point_too_long_days{
      cross_midnight_point_too_long_day};
  std::vector<Diagnostic> cross_midnight_point_too_long_diagnostics;
  const bool cross_midnight_point_too_long_ok = struct_validator.Validate(
      "module-smoke.txt", cross_midnight_point_too_long_days,
      cross_midnight_point_too_long_diagnostics);
  Expect(!cross_midnight_point_too_long_ok,
         "StructValidator should reject too-long point activity after "
         "cross-midnight interval.",
         failures);
  Expect(
      std::any_of(cross_midnight_point_too_long_diagnostics.begin(),
                  cross_midnight_point_too_long_diagnostics.end(),
                  [](const Diagnostic& diagnostic) -> bool {
                    return diagnostic.code == "activity.duration.too_long";
                  }),
      "Too-long point activity after cross-midnight interval should insights "
      "duration diagnostic.",
      failures);

  DailyLog cross_midnight_point_allowed_day = cross_midnight_point_too_long_day;
  cross_midnight_point_allowed_day.rawEvents.back().remark =
      "special case @allow-long";
  processor.Process(previous_for_cross_midnight_point,
                    cross_midnight_point_allowed_day);

  std::vector<DailyLog> cross_midnight_point_allowed_days{
      cross_midnight_point_allowed_day};
  std::vector<Diagnostic> cross_midnight_point_allowed_diagnostics;
  const bool cross_midnight_point_allowed_ok = struct_validator.Validate(
      "module-smoke.txt", cross_midnight_point_allowed_days,
      cross_midnight_point_allowed_diagnostics);
  Expect(cross_midnight_point_allowed_ok,
         "StructValidator should allow too-long point activity when "
         "@allow-long is authored.",
         failures);
  Expect(cross_midnight_point_allowed_diagnostics.empty(),
         "@allow-long point activity after cross-midnight interval should not "
         "emit diagnostics.",
         failures);

  DailyLog cross_midnight_interval_overlap_day;
  cross_midnight_interval_overlap_day.date = "2026-03-12";
  cross_midnight_interval_overlap_day.rawEvents.push_back(
      RawEvent{.kind = RawEventKindType::Interval,
               .startTimeStr = std::string("23:00:00"),
               .endTimeStr = "01:00:00",
               .description = "sleep",
               .source_span = SourceSpan{.file_path = "module-smoke.txt",
                                         .line_start = 21,
                                         .line_end = 21,
                                         .column_start = 1,
                                         .column_end = 14,
                                         .raw_text = "2300-0100sleep"}});
  cross_midnight_interval_overlap_day.rawEvents.push_back(RawEvent{
      .kind = RawEventKindType::Interval,
      .startTimeStr = std::string("00:30:00"),
      .endTimeStr = "02:00:00",
      .description = "study",
      .remark = "special case @allow-long",
      .source_span = SourceSpan{.file_path = "module-smoke.txt",
                                .line_start = 22,
                                .line_end = 22,
                                .column_start = 1,
                                .column_end = 34,
                                .raw_text = "0030-0200study // @allow-long"}});
  std::vector<DailyLog> cross_midnight_interval_overlap_days{
      cross_midnight_interval_overlap_day};
  std::vector<Diagnostic> cross_midnight_interval_overlap_diagnostics;
  const bool cross_midnight_interval_overlap_ok = struct_validator.Validate(
      "module-smoke.txt", cross_midnight_interval_overlap_days,
      cross_midnight_interval_overlap_diagnostics);
  Expect(!cross_midnight_interval_overlap_ok,
         "StructValidator should reject overlapping interval after "
         "cross-midnight interval.",
         failures);
  Expect(std::any_of(cross_midnight_interval_overlap_diagnostics.begin(),
                     cross_midnight_interval_overlap_diagnostics.end(),
                     [](const Diagnostic& diagnostic) -> bool {
                       return diagnostic.code == "timeline.event.overlap";
                     }),
         "@allow-long should not bypass cross-midnight interval overlap.",
         failures);

  DailyLog cross_midnight_interval_gap_day;
  cross_midnight_interval_gap_day.date = "2026-03-13";
  cross_midnight_interval_gap_day.rawEvents.push_back(
      RawEvent{.kind = RawEventKindType::Interval,
               .startTimeStr = std::string("23:00:00"),
               .endTimeStr = "01:00:00",
               .description = "sleep",
               .source_span = SourceSpan{.file_path = "module-smoke.txt",
                                         .line_start = 23,
                                         .line_end = 23,
                                         .column_start = 1,
                                         .column_end = 14,
                                         .raw_text = "2300-0100sleep"}});
  cross_midnight_interval_gap_day.rawEvents.push_back(
      RawEvent{.kind = RawEventKindType::Interval,
               .startTimeStr = std::string("01:30:00"),
               .endTimeStr = "02:00:00",
               .description = "study",
               .source_span = SourceSpan{.file_path = "module-smoke.txt",
                                         .line_start = 24,
                                         .line_end = 24,
                                         .column_start = 1,
                                         .column_end = 14,
                                         .raw_text = "0130-0200study"}});
  std::vector<DailyLog> cross_midnight_interval_gap_days{
      cross_midnight_interval_gap_day};
  std::vector<Diagnostic> cross_midnight_interval_gap_diagnostics;
  const bool cross_midnight_interval_gap_ok = struct_validator.Validate(
      "module-smoke.txt", cross_midnight_interval_gap_days,
      cross_midnight_interval_gap_diagnostics);
  Expect(cross_midnight_interval_gap_ok,
         "StructValidator should allow a gap after a cross-midnight interval.",
         failures);
  Expect(cross_midnight_interval_gap_diagnostics.empty(),
         "Gap after cross-midnight interval should not emit diagnostics.",
         failures);

  DailyLog zero_interval_day;
  zero_interval_day.date = "2026-03-06";
  zero_interval_day.rawEvents.push_back(
      RawEvent{.kind = RawEventKindType::Interval,
               .startTimeStr = std::string("10:30:00"),
               .endTimeStr = "10:30:00",
               .description = "study",
               .source_span = SourceSpan{.file_path = "module-smoke.txt",
                                         .line_start = 9,
                                         .line_end = 9,
                                         .column_start = 1,
                                         .column_end = 14,
                                         .raw_text = "1030-1030study"}});

  std::vector<DailyLog> zero_interval_days{zero_interval_day};
  std::vector<Diagnostic> zero_interval_diagnostics;
  const bool zero_interval_ok = struct_validator.Validate(
      "module-smoke.txt", zero_interval_days, zero_interval_diagnostics);
  Expect(!zero_interval_ok,
         "StructValidator should reject zero-duration interval ranges.",
         failures);
  Expect(std::any_of(
             zero_interval_diagnostics.begin(), zero_interval_diagnostics.end(),
             [](const Diagnostic& diagnostic) -> bool {
               return diagnostic.code == "timeline.interval.invalid_range";
             }),
         "Zero-duration interval should insights invalid_range diagnostic.",
         failures);

  DailyLog too_long_interval_day;
  too_long_interval_day.date = "2026-03-08";
  too_long_interval_day.isContinuation = true;
  too_long_interval_day.rawEvents.push_back(
      RawEvent{.kind = RawEventKindType::Interval,
               .startTimeStr = std::string("10:30:00"),
               .endTimeStr = "09:00:00",
               .description = "study",
               .source_span = SourceSpan{.file_path = "module-smoke.txt",
                                         .line_start = 15,
                                         .line_end = 15,
                                         .column_start = 1,
                                         .column_end = 14,
                                         .raw_text = "1030-0900study"}});
  DailyLog previous_for_too_long;
  previous_for_too_long.date = "2026-03-07";
  processor.Process(previous_for_too_long, too_long_interval_day);

  std::vector<DailyLog> too_long_interval_days{too_long_interval_day};
  std::vector<Diagnostic> too_long_interval_diagnostics;
  const bool too_long_interval_ok =
      struct_validator.Validate("module-smoke.txt", too_long_interval_days,
                                too_long_interval_diagnostics);
  Expect(!too_long_interval_ok,
         "StructValidator should reject cross-midnight intervals over 16h "
         "without @allow-long.",
         failures);
  Expect(
      std::any_of(too_long_interval_diagnostics.begin(),
                  too_long_interval_diagnostics.end(),
                  [](const Diagnostic& diagnostic) -> bool {
                    return diagnostic.code == "activity.duration.too_long";
                  }),
      "Too-long cross-midnight interval should insights duration diagnostic.",
      failures);
  Expect(std::none_of(too_long_interval_diagnostics.begin(),
                      too_long_interval_diagnostics.end(),
                      [](const Diagnostic& diagnostic) -> bool {
                        return diagnostic.code ==
                               "timeline.interval.invalid_range";
                      }),
         "Too-long cross-midnight interval should not be rejected as "
         "invalid_range.",
         failures);

  DailyLog boundary_overlap_interval_day;
  boundary_overlap_interval_day.date = "2026-03-10";
  boundary_overlap_interval_day.isContinuation = true;
  boundary_overlap_interval_day.rawEvents.push_back(
      RawEvent{.kind = RawEventKindType::Interval,
               .startTimeStr = std::string("09:00:00"),
               .endTimeStr = "12:00:00",
               .description = "sleep",
               .source_span = SourceSpan{.file_path = "module-smoke.txt",
                                         .line_start = 17,
                                         .line_end = 17,
                                         .column_start = 1,
                                         .column_end = 14,
                                         .raw_text = "0900-1200sleep"}});
  boundary_overlap_interval_day.rawEvents.push_back(RawEvent{
      .kind = RawEventKindType::Interval,
      .startTimeStr = std::string("10:30:00"),
      .endTimeStr = "09:00:00",
      .description = "study",
      .remark = "special case @allow-long",
      .source_span = SourceSpan{.file_path = "module-smoke.txt",
                                .line_start = 18,
                                .line_end = 18,
                                .column_start = 1,
                                .column_end = 34,
                                .raw_text = "1030-0900study // @allow-long"}});
  DailyLog previous_for_boundary_overlap;
  previous_for_boundary_overlap.date = "2026-03-09";
  processor.Process(previous_for_boundary_overlap,
                    boundary_overlap_interval_day);

  std::vector<DailyLog> boundary_overlap_interval_days{
      boundary_overlap_interval_day};
  std::vector<Diagnostic> boundary_overlap_interval_diagnostics;
  const bool boundary_overlap_interval_ok = struct_validator.Validate(
      "module-smoke.txt", boundary_overlap_interval_days,
      boundary_overlap_interval_diagnostics);
  Expect(!boundary_overlap_interval_ok,
         "StructValidator should reject cross-midnight interval text that "
         "starts before the last boundary.",
         failures);
  Expect(
      std::any_of(boundary_overlap_interval_diagnostics.begin(),
                  boundary_overlap_interval_diagnostics.end(),
                  [](const Diagnostic& diagnostic) -> bool {
                    return diagnostic.code == "timeline.event.overlap";
                  }),
      "Boundary-overlapping cross-midnight interval should insights overlap.",
      failures);

  DailyLog allowed_long_interval_day;
  allowed_long_interval_day.date = "2026-03-09";
  allowed_long_interval_day.isContinuation = true;
  allowed_long_interval_day.rawEvents.push_back(RawEvent{
      .kind = RawEventKindType::Interval,
      .startTimeStr = std::string("10:30:00"),
      .endTimeStr = "09:00:00",
      .description = "study",
      .remark = "special case @allow-long",
      .source_span = SourceSpan{.file_path = "module-smoke.txt",
                                .line_start = 16,
                                .line_end = 16,
                                .column_start = 1,
                                .column_end = 34,
                                .raw_text = "1030-0900study // @allow-long"}});
  DailyLog previous_for_allowed_long;
  previous_for_allowed_long.date = "2026-03-08";
  processor.Process(previous_for_allowed_long, allowed_long_interval_day);

  Expect(allowed_long_interval_day.processedActivities.size() == 1,
         "Allowed long cross-midnight interval should produce one activity.",
         failures);
  if (!allowed_long_interval_day.processedActivities.empty()) {
    Expect(
        allowed_long_interval_day.processedActivities.front()
                .duration_seconds == ((22 * 60) + 30) * 60,
        "Allowed long cross-midnight interval should keep next-day duration.",
        failures);
  }

  std::vector<DailyLog> allowed_long_interval_days{allowed_long_interval_day};
  std::vector<Diagnostic> allowed_long_interval_diagnostics;
  const bool allowed_long_interval_ok =
      struct_validator.Validate("module-smoke.txt", allowed_long_interval_days,
                                allowed_long_interval_diagnostics);
  Expect(allowed_long_interval_ok,
         "StructValidator should allow >16h cross-midnight intervals with "
         "@allow-long.",
         failures);
  Expect(allowed_long_interval_diagnostics.empty(),
         "Allowed long cross-midnight interval should not emit diagnostics.",
         failures);
}

}  // namespace

auto main() -> int {
  int failures = 0;
  TestConverterBridge(failures);
  TestValidatorBridge(failures);
  TestStructureValidatorBridge(failures);

  if (failures == 0) {
    std::cout << "[PASS] tracer_core_domain_logic_modules_smoke_tests\n";
    return 0;
  }

  std::cerr << "[FAIL] tracer_core_domain_logic_modules_smoke_tests failures: "
            << failures << '\n';
  return 1;
}

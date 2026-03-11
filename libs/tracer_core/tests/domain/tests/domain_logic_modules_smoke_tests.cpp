import tracer.core.domain;

#include <iostream>
#include <map>
#include <set>
#include <string>
#include <string_view>
#include <type_traits>
#include <vector>

namespace {

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
using tracer::core::domain::model::BaseActivityRecord;
using tracer::core::domain::model::DailyLog;
using tracer::core::domain::model::RawEvent;
using tracer::core::domain::model::SourceSpan;
using tracer::core::domain::types::ConverterConfig;
using tracer::core::domain::types::DateCheckMode;

auto Expect(bool condition, std::string_view message, int& failures) -> void {
  if (condition) {
    return;
  }
  ++failures;
  std::cerr << "[FAIL] " << message << '\n';
}

auto BuildTestConfig() -> ConverterConfig {
  ConverterConfig config;
  config.remark_prefix = "#";
  config.text_mapping["study"] = "study";
  config.text_mapping["sleep"] = "sleep";
  return config;
}

void TestConverterBridge(int& failures) {
  ConverterConfig config = BuildTestConfig();
  DayProcessor processor(config);
  LogLinker linker(config);

  DailyLog previous_day;
  previous_day.date = "2026-03-01";
  RawEvent prev_event;
  prev_event.endTimeStr = "23:30";
  previous_day.rawEvents.push_back(prev_event);

  DailyLog day_to_process;
  day_to_process.date = "2026-03-02";
  day_to_process.getupTime = "07:00";
  processor.Process(previous_day, day_to_process);
  Expect(day_to_process.hasSleepActivity,
         "DayProcessor should generate sleep activity when previous day exists.",
         failures);

  std::map<std::string, std::vector<DailyLog>> data_map;
  DailyLog first_day;
  first_day.date = "2026-03-01";
  first_day.getupTime = "06:45";
  data_map["2026-03"].push_back(first_day);

  LogLinker::ExternalPreviousEvent external_previous_event{
      "2026-02-28", "23:45"};
  linker.LinkFirstDayWithExternalPreviousEvent(data_map,
                                               external_previous_event);
  const DailyLog& linked_day = data_map["2026-03"].front();
  Expect(linked_day.hasSleepActivity,
         "LogLinker should link external previous event for first day.",
         failures);
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
  Expect(LineRules::IsDate("0301"), "LineRules::IsDate bridge mismatch.",
         failures);
  Expect(line_rules.IsRemark("# note"), "LineRules::IsRemark bridge mismatch.",
         failures);

  SourceSpan span;
  span.file_path = "module-smoke.txt";
  span.line_start = 4;
  span.line_end = 4;
  span.column_start = 1;
  span.column_end = 8;
  span.raw_text = "0730study";

  std::set<Error> valid_errors;
  const bool valid_event = line_rules.IsValidEventLine("0730study", 4,
                                                       valid_errors, span);
  Expect(valid_event && valid_errors.empty(),
         "Known activity should pass without validation errors.", failures);

  std::set<Error> unknown_errors;
  const bool unknown_event =
      line_rules.IsValidEventLine("0900unknown", 5, unknown_errors, span);
  Expect(unknown_event, "Unknown activity should remain a structural-valid line.",
         failures);
  Expect(!unknown_errors.empty(),
         "Unknown activity should be reported as semantic error.", failures);
  Expect(!unknown_errors.empty() &&
             unknown_errors.begin()->type == ErrorType::kUnrecognizedActivity,
         "Unknown activity error type mismatch.", failures);

  StructureRules structure_rules;
  std::set<Error> structure_errors;
  structure_rules.ProcessYearLine(1, "y2026", structure_errors, span);
  structure_rules.ProcessMonthLine(2, "m03", structure_errors, span);
  structure_rules.ProcessDateLine(3, "0301", structure_errors, span);
  structure_rules.ProcessEventLine(4, "0730study", structure_errors, span);
  Expect(structure_rules.HasSeenYear(),
         "StructureRules should track year header state.", failures);
  Expect(structure_rules.HasSeenMonth(),
         "StructureRules should track month header state.", failures);

  TextValidator text_validator(config);
  std::set<Error> text_errors;
  const bool text_ok =
      text_validator.Validate("module-smoke.txt",
                              "y2026\nm03\n0301\n0730study\n0800sleep\n",
                              text_errors);
  Expect(text_ok && text_errors.empty(),
         "TextValidator should pass a minimal valid text sample.", failures);

  Diagnostic diagnostic;
  diagnostic.code = "module.smoke";
  Expect(diagnostic.severity == DiagnosticSeverity::kError,
         "Diagnostic default severity mismatch.", failures);
}

void TestStructureValidatorBridge(int& failures) {
  StructValidator struct_validator(DateCheckMode::kNone);

  DailyLog day;
  day.date = "2026-03-01";

  BaseActivityRecord activity_one;
  activity_one.start_time_str = "07:00";
  activity_one.end_time_str = "07:30";
  activity_one.project_path = "study";
  activity_one.duration_seconds = 30 * 60;

  BaseActivityRecord activity_two;
  activity_two.start_time_str = "08:00";
  activity_two.end_time_str = "08:20";
  activity_two.project_path = "exercise";
  activity_two.duration_seconds = 20 * 60;

  day.processedActivities.push_back(activity_one);
  day.processedActivities.push_back(activity_two);

  std::vector<DailyLog> days;
  days.push_back(day);
  std::vector<Diagnostic> diagnostics;
  const bool ok = struct_validator.Validate("module-smoke.txt", days,
                                            diagnostics);
  Expect(ok, "StructValidator should pass for valid activity data.", failures);
  Expect(diagnostics.empty(),
         "StructValidator diagnostics should be empty for valid sample.",
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

  std::cerr
      << "[FAIL] tracer_core_domain_logic_modules_smoke_tests failures: "
      << failures << '\n';
  return 1;
}

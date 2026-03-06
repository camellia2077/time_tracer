#include "domain/logic/converter/convert/core/converter_core.hpp"
#include "domain/logic/converter/log_processor.hpp"
#include "domain/logic/validator/common/diagnostic.hpp"
#include "domain/logic/validator/common/validator_utils.hpp"
#include "domain/logic/validator/structure/structure_validator.hpp"
#include "domain/logic/validator/txt/facade/text_validator.hpp"
#include "domain/logic/validator/txt/rules/txt_rules.hpp"
#include "domain/model/daily_log.hpp"
#include "domain/model/source_span.hpp"
#include "domain/model/time_data_models.hpp"
#include "domain/types/converter_config.hpp"
#include "domain/types/date_check_mode.hpp"

#include <iostream>
#include <map>
#include <set>
#include <string>
#include <string_view>
#include <type_traits>
#include <vector>

namespace {

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

void TestLegacyConverterHeaders(int& failures) {
  ConverterConfig config = BuildTestConfig();
  DayProcessor processor(config);
  LogLinker linker(config);

  DailyLog previous_day;
  previous_day.date = "2026-03-01";
  RawEvent prev_event;
  prev_event.endTimeStr = "23:20";
  previous_day.rawEvents.push_back(prev_event);

  DailyLog day_to_process;
  day_to_process.date = "2026-03-02";
  day_to_process.getupTime = "06:40";
  processor.Process(previous_day, day_to_process);
  Expect(day_to_process.hasSleepActivity,
         "Legacy DayProcessor path should generate sleep activity.", failures);

  std::map<std::string, std::vector<DailyLog>> data_map;
  DailyLog first_day;
  first_day.date = "2026-03-01";
  first_day.getupTime = "07:10";
  data_map["2026-03"].push_back(first_day);

  LogLinker::ExternalPreviousEvent external_previous_event{
      "2026-02-28", "23:30"};
  linker.LinkFirstDayWithExternalPreviousEvent(data_map,
                                               external_previous_event);
  Expect(data_map["2026-03"].front().hasSleepActivity,
         "Legacy LogLinker path should link sleep activity.", failures);

  LogProcessingResult processing_result;
  Expect(processing_result.success,
         "Legacy LogProcessingResult default success mismatch.", failures);
  Expect(std::is_class_v<LogProcessor>,
         "Legacy LogProcessor declaration should stay visible.", failures);
}

void TestLegacyValidatorHeaders(int& failures) {
  ConverterConfig config = BuildTestConfig();
  validator::txt::LineRules line_rules(config);

  Expect(validator::txt::LineRules::IsYear("y2026"),
         "Legacy LineRules::IsYear mismatch.", failures);
  Expect(validator::txt::LineRules::IsMonth("m03"),
         "Legacy LineRules::IsMonth mismatch.", failures);
  Expect(validator::txt::LineRules::IsDate("0301"),
         "Legacy LineRules::IsDate mismatch.", failures);
  Expect(line_rules.IsRemark("# note"), "Legacy LineRules::IsRemark mismatch.",
         failures);

  SourceSpan span;
  span.file_path = "legacy-smoke.txt";
  span.line_start = 4;
  span.line_end = 4;
  span.column_start = 1;
  span.column_end = 8;
  span.raw_text = "0730study";

  std::set<validator::Error> errors;
  const bool valid =
      line_rules.IsValidEventLine("0730study", 4, errors, span);
  Expect(valid && errors.empty(),
         "Legacy known activity should pass without errors.", failures);

  std::set<validator::Error> unknown_errors;
  const bool unknown =
      line_rules.IsValidEventLine("0900unknown", 5, unknown_errors, span);
  Expect(unknown, "Legacy unknown activity line should remain structurally valid.",
         failures);
  Expect(!unknown_errors.empty(),
         "Legacy unknown activity should report validation error.", failures);

  validator::txt::TextValidator text_validator(config);
  std::set<validator::Error> text_errors;
  const bool text_ok =
      text_validator.Validate("legacy-smoke.txt",
                              "y2026\nm03\n0301\n0730study\n0800sleep\n",
                              text_errors);
  Expect(text_ok && text_errors.empty(),
         "Legacy TextValidator should pass valid sample.", failures);

  validator::Diagnostic diagnostic;
  Expect(diagnostic.severity == validator::DiagnosticSeverity::kError,
         "Legacy Diagnostic default severity mismatch.", failures);
}

void TestLegacyStructureValidatorHeaders(int& failures) {
  validator::structure::StructValidator struct_validator(DateCheckMode::kNone);

  DailyLog day;
  day.date = "2026-03-01";

  BaseActivityRecord activity_one;
  activity_one.start_time_str = "07:00";
  activity_one.end_time_str = "07:30";
  activity_one.project_path = "study";
  activity_one.duration_seconds = 30 * 60;

  BaseActivityRecord activity_two;
  activity_two.start_time_str = "08:00";
  activity_two.end_time_str = "08:15";
  activity_two.project_path = "exercise";
  activity_two.duration_seconds = 15 * 60;

  day.processedActivities.push_back(activity_one);
  day.processedActivities.push_back(activity_two);

  std::vector<DailyLog> days{day};
  std::vector<validator::Diagnostic> diagnostics;
  const bool ok = struct_validator.Validate("legacy-smoke.txt", days,
                                            diagnostics);
  Expect(ok, "Legacy StructValidator should pass for valid input.", failures);
  Expect(diagnostics.empty(),
         "Legacy StructValidator diagnostics should be empty.", failures);
}

}  // namespace

auto main() -> int {
  int failures = 0;
  TestLegacyConverterHeaders(failures);
  TestLegacyValidatorHeaders(failures);
  TestLegacyStructureValidatorHeaders(failures);

  if (failures == 0) {
    std::cout << "[PASS] tracer_core_domain_logic_legacy_headers_compat_tests\n";
    return 0;
  }

  std::cerr
      << "[FAIL] tracer_core_domain_logic_legacy_headers_compat_tests failures: "
      << failures << '\n';
  return 1;
}

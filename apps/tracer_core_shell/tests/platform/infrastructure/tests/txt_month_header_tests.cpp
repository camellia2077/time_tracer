// infrastructure/tests/txt_month_header_tests.cpp
import tracer.core.domain.logic.validator.txt.facade;
import tracer.core.domain.logic.validator.common.validator_utils;

#include <exception>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <set>
#include <sstream>
#include <string>
#include <vector>

#include "application/parser/text_parser.hpp"
#include "infrastructure/tests/android_runtime/android_runtime_test_common.hpp"

namespace android_runtime_tests {
namespace {

using RawEventKindType = decltype(RawEvent{}.kind);

using tracer::core::domain::modlogic::validator_common::Error;
using tracer::core::domain::modlogic::validator_txt::TextValidator;

auto BuildTestConverterConfig() -> ConverterConfig {
  ConverterConfig config;
  config.remark_prefix = "r ";
  config.wake_keywords = {"wake"};
  config.text_mapping = {{"study", "study"},
                         {"sleep", "sleep"},
                         {"wake", "wake"}};
  config.top_parent_mapping = {{"study", "study"}};
  config.initial_top_parents = {{"study", "study"}};
  return config;
}

auto CollectErrorMessages(const std::set<Error>& errors) -> std::string {
  std::string joined;
  for (const auto& error : errors) {
    joined += error.message;
    joined.push_back('\n');
  }
  return joined;
}

auto Expect(bool condition, const std::string& message, int& failures) -> void {
  if (condition) {
    return;
  }
  ++failures;
  std::cerr << "[FAIL] " << message << '\n';
}

auto ReadFixtureText(const std::filesystem::path& relative_path)
    -> std::string {
  const std::filesystem::path fixture_path = BuildRepoRoot() / relative_path;
  std::ifstream input(fixture_path, std::ios::binary);
  if (!input.is_open()) {
    return {};
  }
  return std::string(std::istreambuf_iterator<char>(input),
                     std::istreambuf_iterator<char>());
}

auto TestParserPrefersMonthHeader(int& failures) -> void {
  const ConverterConfig kConfig = BuildTestConverterConfig();
  TextParser parser(kConfig);

  std::istringstream input("y2026\nm02\nd0201\n0641wake\n");
  std::vector<DailyLog> parsed_days;
  parser.Parse(
      input,
      [&parsed_days](DailyLog& day) -> void { parsed_days.push_back(day); },
      "sample.txt");

  Expect(parsed_days.size() == 1,
         "TextParser should parse exactly one day with month header.",
         failures);
  if (parsed_days.empty()) {
    return;
  }
  Expect(parsed_days.front().date == "2026-02-01",
         "TextParser should parse dMMDD date markers with mMM context.",
         failures);
}

auto TestParserSupportsIntervalEventLines(int& failures) -> void {
  const ConverterConfig kConfig = BuildTestConverterConfig();
  TextParser parser(kConfig);

  std::istringstream input(
      "y2026\nm02\nd0201\n0641wake\n0900-1030study //focus\n1353sleep\n");
  std::vector<DailyLog> parsed_days;
  parser.Parse(
      input,
      [&parsed_days](DailyLog& day) -> void { parsed_days.push_back(day); },
      "interval_sample.txt");

  Expect(parsed_days.size() == 1,
         "TextParser should parse one day from interval sample.", failures);
  if (parsed_days.empty() || parsed_days.front().rawEvents.size() != 3) {
    return;
  }

  const RawEvent& interval_event = parsed_days.front().rawEvents[1];
  Expect(interval_event.kind == RawEventKindType::Interval,
         "Parser should classify HHMM-HHMMtoken as interval event.",
         failures);
  Expect(interval_event.startTimeStr.has_value() &&
             *interval_event.startTimeStr == "0900" &&
             interval_event.endTimeStr == "1030",
         "Interval parser should preserve authored start/end times.",
         failures);
  Expect(interval_event.remark == "focus",
         "Interval parser should preserve inline remarks.", failures);

  const RawEvent& point_event = parsed_days.front().rawEvents[2];
  Expect(point_event.kind == RawEventKindType::Point,
         "Parser should keep HHMMtoken as point event.", failures);
}

auto TestParserRejectsDateMonthMismatch(int& failures) -> void {
  const ConverterConfig kConfig = BuildTestConverterConfig();
  TextParser parser(kConfig);

  bool threw = false;
  std::string message;
  try {
    std::vector<DailyLog> parsed_days;
    std::istringstream input("y2026\nm02\nd0101\n0641wake\n");
    parser.Parse(
        input,
        [&parsed_days](DailyLog& day) -> void { parsed_days.push_back(day); },
        "parser_mismatch.txt");
  } catch (const std::exception& error) {
    message = error.what();
    threw = Contains(message, "does not match month header");
  }
  Expect(threw, "TextParser should reject dMMDD month/header mismatch.",
         failures);
}

auto TestParserRejectsMissingMonthHeader(int& failures) -> void {
  const ConverterConfig kConfig = BuildTestConverterConfig();
  TextParser parser(kConfig);

  std::istringstream input("y2026\nd0301\n0641wake\n");
  bool threw = false;
  std::string message;
  try {
    std::vector<DailyLog> parsed_days;
    parser.Parse(
        input,
        [&parsed_days](DailyLog& day) -> void { parsed_days.push_back(day); },
        "missing_month.txt");
  } catch (const std::exception& error) {
    message = error.what();
    threw = Contains(message, "month header (mMM)");
  }
  Expect(threw,
         "TextParser should reject files that omit required month header.",
         failures);
  Expect(Contains(message, "missing_month.txt:2"),
         "TextParser parse errors should include source file and line number.",
         failures);
  Expect(message.starts_with("missing_month.txt:2: Parse error:"),
         "TextParser parse errors should begin with IDE-friendly location.",
         failures);
}

auto TestParserRejectsMissingMonthHeaderFixture(int& failures) -> void {
  const ConverterConfig kConfig = BuildTestConverterConfig();
  TextParser parser(kConfig);

  const std::string fixture_text = ReadFixtureText(
      "test/fixtures/text/invalid/2026-01.missing_month_header.txt");
  Expect(!fixture_text.empty(),
         "missing-month fixture should be readable from test/fixtures.",
         failures);
  if (fixture_text.empty()) {
    return;
  }

  bool threw = false;
  std::string message;
  try {
    std::vector<DailyLog> parsed_days;
    std::istringstream input(fixture_text);
    parser.Parse(
        input,
        [&parsed_days](DailyLog& day) -> void { parsed_days.push_back(day); },
        "2026-01.missing_month_header.txt");
  } catch (const std::exception& error) {
    message = error.what();
    threw = Contains(message, "month header (mMM)");
  }
  Expect(threw,
         "TextParser should reject the missing-month fixture with an mMM error.",
         failures);
  Expect(Contains(message, "2026-01.missing_month_header.txt:3"),
         "missing-month fixture parse error should point at the first day marker line.",
         failures);
}

auto TestValidatorRequiresMonthHeader(int& failures) -> void {
  const ConverterConfig kConfig = BuildTestConverterConfig();
  TextValidator text_validator(kConfig);

  std::set<Error> month_errors;
  const bool kMonthOk = text_validator.Validate(
      "month_ok.txt", "y2026\nm02\nd0201\n0641wake\n", month_errors);
  Expect(kMonthOk && month_errors.empty(),
         "TextValidator should accept yYYYY + mMM + matching dMMDD.",
         failures);

  std::set<Error> missing_month_errors;
  const bool kMissingMonthOk = text_validator.Validate(
      "missing_month.txt", "y2026\nd0201\n0641wake\n", missing_month_errors);
  const std::string kMissingMonthText =
      CollectErrorMessages(missing_month_errors);
  Expect(!kMissingMonthOk,
         "TextValidator should fail when mMM month header is missing.",
         failures);
  Expect(Contains(kMissingMonthText, "Month header (mMM) is required"),
         "Missing month header should report explicit mMM requirement.",
         failures);

  std::set<Error> bare_mmdd_errors;
  const bool kBareMmddOk = text_validator.Validate(
      "bare_mmdd.txt", "y2026\nm02\n0201\n0641wake\n", bare_mmdd_errors);
  const std::string kBareMmddText = CollectErrorMessages(bare_mmdd_errors);
  Expect(!kBareMmddOk, "Bare MMDD should no longer validate as a date marker.",
         failures);
  Expect(Contains(kBareMmddText, "Unrecognized line format: 0201"),
         "Bare MMDD should be reported as an unrecognized line.", failures);

  std::set<Error> year_only_errors;
  const bool kYearOnlyOk =
      text_validator.Validate("year_only.txt", "y2026\n", year_only_errors);
  const std::string kYearOnlyText = CollectErrorMessages(year_only_errors);
  Expect(!kYearOnlyOk, "TextValidator should reject year-only files.",
         failures);
  Expect(Contains(kYearOnlyText, "Month header (mMM) is required"),
         "Year-only files should report missing mMM requirement.", failures);
}

auto TestValidatorRequiresMonthHeaderFixture(int& failures) -> void {
  const ConverterConfig kConfig = BuildTestConverterConfig();
  TextValidator text_validator(kConfig);

  const std::string fixture_text = ReadFixtureText(
      "test/fixtures/text/invalid/2026-01.missing_month_header.txt");
  Expect(!fixture_text.empty(),
         "validator missing-month fixture should be readable from test/fixtures.",
         failures);
  if (fixture_text.empty()) {
    return;
  }

  std::set<Error> errors;
  const bool ok = text_validator.Validate("2026-01.missing_month_header.txt",
                                          fixture_text, errors);
  const std::string error_text = CollectErrorMessages(errors);
  Expect(!ok,
         "TextValidator should reject the missing-month fixture.", failures);
  Expect(Contains(error_text, "Month header (mMM) is required"),
         "missing-month fixture should report explicit missing header wording.",
         failures);
}

auto TestValidatorRejectsMonthConflicts(int& failures) -> void {
  const ConverterConfig kConfig = BuildTestConverterConfig();
  TextValidator text_validator(kConfig);

  std::set<Error> mismatch_errors;
  const bool kMismatchOk = text_validator.Validate(
      "mismatch.txt", "y2026\nm02\nd0101\n0641wake\n", mismatch_errors);
  const std::string kMismatchText = CollectErrorMessages(mismatch_errors);
  Expect(!kMismatchOk, "Month/date mismatch should fail validation.", failures);
  Expect(Contains(kMismatchText, "does not match month header"),
         "Mismatch error should mention month-header conflict.", failures);

  std::set<Error> duplicate_errors;
  const bool kDuplicateOk = text_validator.Validate(
      "duplicate_month.txt", "y2026\nm02\nm03\nd0201\n0641wake\n",
      duplicate_errors);
  const std::string kDuplicateText = CollectErrorMessages(duplicate_errors);
  Expect(!kDuplicateOk, "Duplicate mMM headers should fail validation.",
         failures);
  Expect(Contains(kDuplicateText, "Multiple month headers found"),
         "Duplicate month header should report dedicated error.", failures);

  std::set<Error> late_month_errors;
  const bool kLateMonthOk = text_validator.Validate(
      "late_month.txt", "y2026\nd0201\n0641wake\nm02\n", late_month_errors);
  const std::string kLateMonthText = CollectErrorMessages(late_month_errors);
  Expect(!kLateMonthOk, "Late mMM header should fail validation.", failures);
  Expect(Contains(kLateMonthText, "must appear before the first date line"),
         "Late month header error should explain ordering rule.", failures);
}

auto TestValidatorSupportsIntervalEventLines(int& failures) -> void {
  const ConverterConfig kConfig = BuildTestConverterConfig();
  TextValidator text_validator(kConfig);

  std::set<Error> interval_errors;
  const bool kIntervalOk = text_validator.Validate(
      "interval_ok.txt", "y2026\nm02\nd0201\n0641wake\n0900-1030study\n",
      interval_errors);
  Expect(kIntervalOk && interval_errors.empty(),
         "TextValidator should accept interval event lines.", failures);

  std::set<Error> missing_activity_errors;
  const bool kMissingActivityOk = text_validator.Validate(
      "interval_missing_activity.txt", "y2026\nm02\nd0201\n0900-1030\n",
      missing_activity_errors);
  Expect(!kMissingActivityOk,
         "TextValidator should reject interval lines without an activity token.",
         failures);

  std::set<Error> invalid_time_errors;
  const bool kInvalidTimeOk = text_validator.Validate(
      "interval_bad_time.txt", "y2026\nm02\nd0201\n0900-2460study\n",
      invalid_time_errors);
  Expect(!kInvalidTimeOk,
         "TextValidator should reject interval lines with invalid HHMM values.",
         failures);

  std::set<Error> unknown_interval_errors;
  const bool kUnknownIntervalOk = text_validator.Validate(
      "interval_unknown.txt", "y2026\nm02\nd0201\n0900-1030unknown\n",
      unknown_interval_errors);
  Expect(!kUnknownIntervalOk,
         "TextValidator should reject unknown interval activities semantically.",
         failures);
  Expect(Contains(CollectErrorMessages(unknown_interval_errors),
                  "Unrecognized activity 'unknown'"),
         "Unknown interval activity should keep semantic validation wording.",
         failures);
}

auto TestValidatorReadsIntervalFixture(int& failures) -> void {
  const ConverterConfig kConfig = BuildTestConverterConfig();
  TextValidator text_validator(kConfig);

  const std::string fixture_text = ReadFixtureText(
      "test/fixtures/text/minimal_month/2026-01.interval_day.txt");
  Expect(!fixture_text.empty(),
         "interval-day fixture should be readable from test/fixtures.",
         failures);
  if (fixture_text.empty()) {
    return;
  }

  std::set<Error> errors;
  const bool ok = text_validator.Validate("2026-01.interval_day.txt",
                                          fixture_text, errors);
  Expect(ok && errors.empty(),
         "TextValidator should accept the interval-day fixture.", failures);
}

}  // namespace

auto RunTxtMonthHeaderTests(int& failures) -> void {
  TestParserPrefersMonthHeader(failures);
  TestParserSupportsIntervalEventLines(failures);
  TestParserRejectsDateMonthMismatch(failures);
  TestParserRejectsMissingMonthHeader(failures);
  TestParserRejectsMissingMonthHeaderFixture(failures);
  TestValidatorRequiresMonthHeader(failures);
  TestValidatorRequiresMonthHeaderFixture(failures);
  TestValidatorRejectsMonthConflicts(failures);
  TestValidatorSupportsIntervalEventLines(failures);
  TestValidatorReadsIntervalFixture(failures);
}

}  // namespace android_runtime_tests

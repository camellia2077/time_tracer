// infrastructure/tests/txt_month_header_tests.cpp
#include <exception>
#include <iostream>
#include <set>
#include <sstream>
#include <string>
#include <vector>

#include "application/parser/text_parser.hpp"
#include "domain/logic/validator/txt/facade/text_validator.hpp"
#include "infrastructure/tests/android_runtime/android_runtime_test_common.hpp"

namespace android_runtime_tests {
namespace {

auto BuildTestConverterConfig() -> ConverterConfig {
  ConverterConfig config;
  config.remark_prefix = "r ";
  config.wake_keywords = {"wake"};
  config.text_mapping = {{"study", "study"}, {"wake", "wake"}};
  config.top_parent_mapping = {{"study", "study"}};
  config.initial_top_parents = {{"study", "study"}};
  return config;
}

auto CollectErrorMessages(const std::set<validator::Error>& errors)
    -> std::string {
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

auto TestParserPrefersMonthHeader(int& failures) -> void {
  const ConverterConfig kConfig = BuildTestConverterConfig();
  TextParser parser(kConfig);

  std::istringstream input("y2026\nm02\n0101\n0641wake\n");
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
         "TextParser should prefer mMM over MMDD month digits.", failures);
}

auto TestParserRejectsMissingMonthHeader(int& failures) -> void {
  const ConverterConfig kConfig = BuildTestConverterConfig();
  TextParser parser(kConfig);

  std::istringstream input("y2026\n0301\n0641wake\n");
  bool threw = false;
  try {
    std::vector<DailyLog> parsed_days;
    parser.Parse(
        input,
        [&parsed_days](DailyLog& day) -> void { parsed_days.push_back(day); },
        "missing_month.txt");
  } catch (const std::exception& error) {
    threw = Contains(error.what(), "month header (mMM)");
  }
  Expect(threw,
         "TextParser should reject files that omit required month header.",
         failures);
}

auto TestValidatorRequiresMonthHeader(int& failures) -> void {
  const ConverterConfig kConfig = BuildTestConverterConfig();
  validator::txt::TextValidator validator(kConfig);

  std::set<validator::Error> month_errors;
  const bool kMonthOk = validator.Validate(
      "month_ok.txt", "y2026\nm02\n0201\n0641wake\n", month_errors);
  Expect(kMonthOk && month_errors.empty(),
         "TextValidator should accept yYYYY + mMM + matching MMDD.", failures);

  std::set<validator::Error> missing_month_errors;
  const bool kMissingMonthOk = validator.Validate(
      "missing_month.txt", "y2026\n0201\n0641wake\n", missing_month_errors);
  const std::string kMissingMonthText =
      CollectErrorMessages(missing_month_errors);
  Expect(!kMissingMonthOk,
         "TextValidator should fail when mMM month header is missing.",
         failures);
  Expect(Contains(kMissingMonthText, "Month header (mMM) is required"),
         "Missing month header should report explicit mMM requirement.",
         failures);

  std::set<validator::Error> year_only_errors;
  const bool kYearOnlyOk =
      validator.Validate("year_only.txt", "y2026\n", year_only_errors);
  const std::string kYearOnlyText = CollectErrorMessages(year_only_errors);
  Expect(!kYearOnlyOk, "TextValidator should reject year-only files.",
         failures);
  Expect(Contains(kYearOnlyText, "Month header (mMM) is required"),
         "Year-only files should report missing mMM requirement.", failures);
}

auto TestValidatorRejectsMonthConflicts(int& failures) -> void {
  const ConverterConfig kConfig = BuildTestConverterConfig();
  validator::txt::TextValidator validator(kConfig);

  std::set<validator::Error> mismatch_errors;
  const bool kMismatchOk = validator.Validate(
      "mismatch.txt", "y2026\nm02\n0101\n0641wake\n", mismatch_errors);
  const std::string kMismatchText = CollectErrorMessages(mismatch_errors);
  Expect(!kMismatchOk, "Month/date mismatch should fail validation.", failures);
  Expect(Contains(kMismatchText, "does not match month header"),
         "Mismatch error should mention month-header conflict.", failures);

  std::set<validator::Error> duplicate_errors;
  const bool kDuplicateOk =
      validator.Validate("duplicate_month.txt",
                         "y2026\nm02\nm03\n0201\n0641wake\n", duplicate_errors);
  const std::string kDuplicateText = CollectErrorMessages(duplicate_errors);
  Expect(!kDuplicateOk, "Duplicate mMM headers should fail validation.",
         failures);
  Expect(Contains(kDuplicateText, "Multiple month headers found"),
         "Duplicate month header should report dedicated error.", failures);

  std::set<validator::Error> late_month_errors;
  const bool kLateMonthOk = validator.Validate(
      "late_month.txt", "y2026\n0201\n0641wake\nm02\n", late_month_errors);
  const std::string kLateMonthText = CollectErrorMessages(late_month_errors);
  Expect(!kLateMonthOk, "Late mMM header should fail validation.", failures);
  Expect(Contains(kLateMonthText, "must appear before the first date line"),
         "Late month header error should explain ordering rule.", failures);
}

}  // namespace

auto RunTxtMonthHeaderTests(int& failures) -> void {
  TestParserPrefersMonthHeader(failures);
  TestParserRejectsMissingMonthHeader(failures);
  TestValidatorRequiresMonthHeader(failures);
  TestValidatorRejectsMonthConflicts(failures);
}

}  // namespace android_runtime_tests

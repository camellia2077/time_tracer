// application/tests/modules/report_semantics_tests.cpp
#include "application/tests/modules/reporting_tests.hpp"

#include <stdexcept>

#include "application/ports/reporting/i_report_dto_formatter.hpp"
#include "application/tests/support/test_support.hpp"
#include "application/use_cases/report_api_support.hpp"

namespace tracer_core::application::tests {

namespace report_support = tracer::core::application::use_cases::report_support;
using tracer_core::core::dto::StructuredReportKind;
using tracer_core::core::dto::StructuredReportOutput;

namespace {

class FakeReportFormatter final
    : public tracer_core::application::ports::IReportDtoFormatter {
 public:
  auto FormatDaily(const DailyReportData& report, ReportFormat /*format*/)
      -> std::string override {
    return "daily:" + report.date;
  }

  auto FormatMonthly(const MonthlyReportData& report, ReportFormat /*format*/)
      -> std::string override {
    return "month:" + report.range_label;
  }

  auto FormatPeriod(const PeriodReportData& report, ReportFormat /*format*/)
      -> std::string override {
    return "period:" + report.start_date + "|" + report.end_date;
  }

  auto FormatWeekly(const WeeklyReportData& report, ReportFormat /*format*/)
      -> std::string override {
    return "week:" + report.range_label;
  }

  auto FormatYearly(const YearlyReportData& report, ReportFormat /*format*/)
      -> std::string override {
    return "year:" + report.range_label;
  }
};

auto TestParseRecentDaysArgument(TestState& state) -> void {
  Expect(state, report_support::ParseRecentDaysArgument("7") == 7,
         "ParseRecentDaysArgument should parse positive integer.");
  Expect(state, report_support::ParseRecentDaysArgument(" 14 ") == 14,
         "ParseRecentDaysArgument should ignore ASCII whitespace.");

  bool threw_zero = false;
  try {
    static_cast<void>(report_support::ParseRecentDaysArgument("0"));
  } catch (const std::invalid_argument&) {
    threw_zero = true;
  }
  Expect(state, threw_zero,
         "ParseRecentDaysArgument should reject non-positive values.");

  bool threw_alpha = false;
  try {
    static_cast<void>(report_support::ParseRecentDaysArgument("abc"));
  } catch (const std::invalid_argument&) {
    threw_alpha = true;
  }
  Expect(state, threw_alpha,
         "ParseRecentDaysArgument should reject non-numeric values.");
}

auto TestParseRangeArgument(TestState& state) -> void {
  const auto canonical = report_support::ParseRangeArgument("2026-01-01|2026-01-31");
  Expect(state, canonical.start_date == "2026-01-01",
         "ParseRangeArgument should preserve ISO start_date.");
  Expect(state, canonical.end_date == "2026-01-31",
         "ParseRangeArgument should preserve ISO end_date.");

  const auto comma = report_support::ParseRangeArgument(" 2026-02-01 , 2026-02-09 ");
  Expect(state, comma.start_date == "2026-02-01",
         "ParseRangeArgument should accept comma separators.");
  Expect(state, comma.end_date == "2026-02-09",
         "ParseRangeArgument should trim whitespace around comma-separated dates.");

  bool threw_descending = false;
  try {
    static_cast<void>(report_support::ParseRangeArgument(
        "2026-03-09|2026-03-01"));
  } catch (const std::invalid_argument&) {
    threw_descending = true;
  }
  Expect(state, threw_descending,
         "ParseRangeArgument should reject descending ranges.");

  bool threw_missing = false;
  try {
    static_cast<void>(report_support::ParseRangeArgument("2026-03-01"));
  } catch (const std::invalid_argument&) {
    threw_missing = true;
  }
  Expect(state, threw_missing,
         "ParseRangeArgument should require explicit range separators.");
}

auto TestFormatStructuredReportWindowMetadata(TestState& state) -> void {
  FakeReportFormatter formatter;

  PeriodReportData period{};
  period.has_records = false;
  period.matched_day_count = 0;
  period.matched_record_count = 0;
  period.start_date = "2024-12-01";
  period.end_date = "2024-12-31";
  period.requested_days = 31;

  StructuredReportOutput recent_output{};
  recent_output.ok = true;
  recent_output.kind = StructuredReportKind::kRecent;
  recent_output.report = period;

  const auto recent =
      report_support::FormatStructuredReport(recent_output,
                                             ReportFormat::kMarkdown,
                                             formatter);
  Expect(state, recent.ok,
         "FormatStructuredReport should succeed for recent period reports.");
  Expect(state, recent.content == "period:2024-12-01|2024-12-31",
         "FormatStructuredReport should delegate period formatting.");
  Expect(state, recent.report_window_metadata.has_value(),
         "FormatStructuredReport should expose window metadata for recent reports.");
  if (recent.report_window_metadata.has_value()) {
    const auto& metadata = *recent.report_window_metadata;
    Expect(state, !metadata.has_records,
           "Recent report metadata should preserve has_records=false.");
    Expect(state, metadata.matched_day_count == 0,
           "Recent report metadata should preserve matched_day_count.");
    Expect(state, metadata.matched_record_count == 0,
           "Recent report metadata should preserve matched_record_count.");
    Expect(state, metadata.start_date == "2024-12-01",
           "Recent report metadata should preserve start_date.");
    Expect(state, metadata.end_date == "2024-12-31",
           "Recent report metadata should preserve end_date.");
    Expect(state, metadata.requested_days == 31,
           "Recent report metadata should preserve requested_days.");
  }

  StructuredReportOutput range_output = recent_output;
  range_output.kind = StructuredReportKind::kRange;
  const auto range = report_support::FormatStructuredReport(
      range_output, ReportFormat::kMarkdown, formatter);
  Expect(state, range.ok,
         "FormatStructuredReport should succeed for range period reports.");
  Expect(state, range.report_window_metadata.has_value(),
         "FormatStructuredReport should expose window metadata for range reports.");
}

auto TestFormatStructuredReportOmitsWindowMetadataForMonthlyReports(
    TestState& state) -> void {
  FakeReportFormatter formatter;

  MonthlyReportData monthly{};
  monthly.range_label = "2026-04";

  StructuredReportOutput output{};
  output.ok = true;
  output.kind = StructuredReportKind::kMonth;
  output.report = monthly;

  const auto formatted =
      report_support::FormatStructuredReport(output, ReportFormat::kMarkdown,
                                             formatter);
  Expect(state, formatted.ok,
         "FormatStructuredReport should succeed for monthly reports.");
  Expect(state, formatted.content == "month:2026-04",
         "FormatStructuredReport should delegate monthly formatting.");
  Expect(state, !formatted.report_window_metadata.has_value(),
         "FormatStructuredReport should keep window metadata reserved for recent/range period reports.");
}

auto TestStructuredReportDistinguishesEmptyWindowFromMissingTarget(
    TestState& state) -> void {
  FakePipelineWorkflow pipeline_workflow;
  FakeReportHandler report_handler;
  auto report_data_query = std::make_shared<FakeReportDataQueryService>();
  auto runtime_api =
      BuildRuntimeApiForTest(pipeline_workflow, report_handler, report_data_query);

  const auto empty_range = runtime_api.report().RunStructuredReportQuery(
      {.type = tracer_core::core::dto::ReportQueryType::kRange,
       .argument = "2024-12-01|2024-12-31"});
  Expect(state, empty_range.ok,
         "RunStructuredReportQuery should treat empty range windows as successful reports.");
  Expect(state, empty_range.error_contract.error_code.empty(),
         "RunStructuredReportQuery empty range should not expose target-not-found error code.");
  const auto* empty_range_report =
      std::get_if<PeriodReportData>(&empty_range.report);
  Expect(state, empty_range_report != nullptr,
         "RunStructuredReportQuery empty range should still return period report data.");
  if (empty_range_report != nullptr) {
    Expect(state, !empty_range_report->has_records,
           "RunStructuredReportQuery empty range should preserve has_records=false.");
    Expect(state, empty_range_report->matched_record_count == 0,
           "RunStructuredReportQuery empty range should preserve matched_record_count=0.");
  }

  report_data_query->fail_target_not_found = true;
  const auto missing_day = runtime_api.report().RunStructuredReportQuery(
      {.type = tracer_core::core::dto::ReportQueryType::kDay,
       .argument = "2024-12-31"});
  Expect(state, !missing_day.ok,
         "RunStructuredReportQuery should fail when the named report target is missing.");
  Expect(state,
         missing_day.error_contract.error_code == "reporting.target.not_found",
         "RunStructuredReportQuery missing target should expose reporting.target.not_found.");
  Expect(state, missing_day.error_contract.error_category == "reporting",
         "RunStructuredReportQuery missing target should expose reporting category.");
}

}  // namespace

auto RunReportSemanticsTests(TestState& state) -> void {
  TestParseRecentDaysArgument(state);
  TestParseRangeArgument(state);
  TestFormatStructuredReportWindowMetadata(state);
  TestFormatStructuredReportOmitsWindowMetadataForMonthlyReports(state);
  TestStructuredReportDistinguishesEmptyWindowFromMissingTarget(state);
}

}  // namespace tracer_core::application::tests

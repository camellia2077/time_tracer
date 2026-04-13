// application/tests/modules/report_semantics_tests.cpp
#include "application/tests/modules/reporting_tests.hpp"

#include <stdexcept>

#include "application/tests/support/fakes.hpp"
#include "application/tests/support/test_support.hpp"
#include "application/use_cases/report_api_support.hpp"

namespace tracer_core::application::tests {

namespace report_support = tracer::core::application::use_cases::report_support;
using tracer_core::core::dto::ReportDisplayMode;
using tracer_core::core::dto::TemporalSelectionKind;
using tracer_core::core::dto::TemporalSelectionPayload;

namespace {

auto BuildRangeSelection(std::string start_date, std::string end_date)
    -> TemporalSelectionPayload {
  return {.kind = TemporalSelectionKind::kDateRange,
          .start_date = std::move(start_date),
          .end_date = std::move(end_date)};
}

auto BuildRecentSelection(int days,
                          std::optional<std::string> anchor_date = std::nullopt)
    -> TemporalSelectionPayload {
  return {.kind = TemporalSelectionKind::kRecentDays,
          .days = days,
          .anchor_date = std::move(anchor_date)};
}

auto BuildDaySelection(std::string date) -> TemporalSelectionPayload {
  return {.kind = TemporalSelectionKind::kSingleDay, .date = std::move(date)};
}

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

auto TestTemporalTextQueryPreservesWindowMetadata(TestState& state) -> void {
  FakePipelineWorkflow pipeline_workflow;
  FakeReportHandler report_handler;
  auto report_data_query = std::make_shared<FakeReportDataQueryService>();
  auto runtime_api =
      BuildRuntimeApiForTest(pipeline_workflow, report_handler, report_data_query);

  const auto recent = runtime_api.report().RunTemporalReportQuery(
      {.display_mode = ReportDisplayMode::kRecent,
       .selection = BuildRecentSelection(7, "2026-03-07"),
       .format = ReportFormat::kMarkdown});
  Expect(state, recent.ok,
         "RunTemporalReportQuery should succeed for recent period reports.");
  Expect(state, recent.content == "period:2026-03-01|2026-03-07",
         "RunTemporalReportQuery should delegate period formatting.");
  Expect(state, recent.report_window_metadata.has_value(),
         "RunTemporalReportQuery should expose window metadata for recent reports.");
  if (recent.report_window_metadata.has_value()) {
    const auto& metadata = *recent.report_window_metadata;
    Expect(state, !metadata.has_records,
           "Recent report metadata should preserve has_records=false.");
    Expect(state, metadata.matched_day_count == 0,
           "Recent report metadata should preserve matched_day_count.");
    Expect(state, metadata.matched_record_count == 0,
           "Recent report metadata should preserve matched_record_count.");
    Expect(state, metadata.start_date == "2026-03-01",
           "Recent report metadata should preserve anchored start_date.");
    Expect(state, metadata.end_date == "2026-03-07",
           "Recent report metadata should preserve anchored end_date.");
    Expect(state, metadata.requested_days == 7,
           "Recent report metadata should preserve requested_days.");
  }

  const auto range = runtime_api.report().RunTemporalReportQuery(
      {.display_mode = ReportDisplayMode::kRange,
       .selection = BuildRangeSelection("2024-12-01", "2024-12-31"),
       .format = ReportFormat::kMarkdown});
  Expect(state, range.ok,
         "RunTemporalReportQuery should succeed for range period reports.");
  Expect(state, range.report_window_metadata.has_value(),
         "RunTemporalReportQuery should expose window metadata for range reports.");

  const auto month = runtime_api.report().RunTemporalReportQuery(
      {.display_mode = ReportDisplayMode::kMonth,
       .selection = BuildRangeSelection("2026-04-01", "2026-04-30"),
       .format = ReportFormat::kMarkdown});
  Expect(state, month.ok,
         "RunTemporalReportQuery should succeed for monthly reports.");
  Expect(state, month.content == "month:2026-04",
         "RunTemporalReportQuery should delegate monthly formatting.");
  Expect(state, !month.report_window_metadata.has_value(),
         "RunTemporalReportQuery should keep window metadata reserved for recent/range reports.");
}

auto TestStructuredReportDistinguishesEmptyWindowFromMissingTarget(
    TestState& state) -> void {
  FakePipelineWorkflow pipeline_workflow;
  FakeReportHandler report_handler;
  auto report_data_query = std::make_shared<FakeReportDataQueryService>();
  auto runtime_api =
      BuildRuntimeApiForTest(pipeline_workflow, report_handler, report_data_query);

  const auto empty_range = runtime_api.report().RunTemporalStructuredReportQuery(
      {.display_mode = ReportDisplayMode::kRange,
       .selection = BuildRangeSelection("2024-12-01", "2024-12-31")});
  Expect(state, empty_range.ok,
         "RunTemporalStructuredReportQuery should treat empty range windows as successful reports.");
  Expect(state, empty_range.error_contract.error_code.empty(),
         "RunTemporalStructuredReportQuery empty range should not expose target-not-found error code.");
  const auto* empty_range_report =
      std::get_if<PeriodReportData>(&empty_range.report);
  Expect(state, empty_range_report != nullptr,
         "RunTemporalStructuredReportQuery empty range should still return period report data.");
  if (empty_range_report != nullptr) {
    Expect(state, !empty_range_report->has_records,
           "RunTemporalStructuredReportQuery empty range should preserve has_records=false.");
    Expect(state, empty_range_report->matched_record_count == 0,
           "RunTemporalStructuredReportQuery empty range should preserve matched_record_count=0.");
  }

  report_data_query->fail_target_not_found = true;
  const auto missing_day = runtime_api.report().RunTemporalStructuredReportQuery(
      {.display_mode = ReportDisplayMode::kDay,
       .selection = BuildDaySelection("2024-12-31")});
  Expect(state, !missing_day.ok,
         "RunTemporalStructuredReportQuery should fail when the named report target is missing.");
  Expect(state,
         missing_day.error_contract.error_code == "reporting.target.not_found",
         "RunTemporalStructuredReportQuery missing target should expose reporting.target.not_found.");
  Expect(state, missing_day.error_contract.error_category == "reporting",
         "RunTemporalStructuredReportQuery missing target should expose reporting category.");
}

}  // namespace

auto RunReportSemanticsTests(TestState& state) -> void {
  TestParseRecentDaysArgument(state);
  TestParseRangeArgument(state);
  TestTemporalTextQueryPreservesWindowMetadata(state);
  TestStructuredReportDistinguishesEmptyWindowFromMissingTarget(state);
}

}  // namespace tracer_core::application::tests

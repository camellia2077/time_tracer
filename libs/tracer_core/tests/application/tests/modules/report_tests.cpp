// application/tests/modules/report_tests.cpp
#include "application/tests/modules/reporting_tests.hpp"
#include "application/tests/support/fakes.hpp"
#include "application/tests/support/test_support.hpp"

namespace tracer_core::application::tests {

using tracer_core::core::dto::PeriodBatchQueryRequest;
using tracer_core::core::dto::ReportDisplayMode;
using tracer_core::core::dto::TemporalReportQueryRequest;
using tracer_core::core::dto::TemporalReportTargetsRequest;
using tracer_core::core::dto::TemporalSelectionKind;
using tracer_core::core::dto::TemporalSelectionPayload;

namespace {

auto BuildDaySelection(std::string date) -> TemporalSelectionPayload {
  return {.kind = TemporalSelectionKind::kSingleDay, .date = std::move(date)};
}

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

auto TestTemporalReportQueryResponses(TestState& state) -> void {
  FakePipelineWorkflow pipeline_workflow;
  FakeReportHandler report_handler;
  auto report_data_query = std::make_shared<FakeReportDataQueryService>();
  auto runtime_api =
      BuildRuntimeApiForTest(pipeline_workflow, report_handler, report_data_query);

  const auto kDaySuccess = runtime_api.report().RunTemporalReportQuery(
      {.display_mode = ReportDisplayMode::kDay,
       .selection = BuildDaySelection("2026-01-03"),
       .format = ReportFormat::kMarkdown});
  Expect(state, kDaySuccess.ok,
         "RunTemporalReportQuery should return ok on success.");
  Expect(state, kDaySuccess.content == "daily:2026-01-03",
         "RunTemporalReportQuery should return formatted day content.");

  const auto kRecentSuccess = runtime_api.report().RunTemporalReportQuery(
      {.display_mode = ReportDisplayMode::kRecent,
       .selection = BuildRecentSelection(7, "2026-03-07"),
       .format = ReportFormat::kMarkdown});
  Expect(state, kRecentSuccess.ok,
         "RunTemporalReportQuery recent should succeed with anchor_date.");
  Expect(state, kRecentSuccess.content == "period:2026-03-01|2026-03-07",
         "RunTemporalReportQuery recent should format anchored fixed window.");
  Expect(state,
         kRecentSuccess.report_window_metadata.has_value() &&
             kRecentSuccess.report_window_metadata->requested_days == 7,
         "RunTemporalReportQuery recent should expose requested_days metadata.");

  const auto kBadRecentArg = runtime_api.report().RunTemporalReportQuery(
      {.display_mode = ReportDisplayMode::kRecent,
       .selection = BuildRecentSelection(0),
       .format = ReportFormat::kMarkdown});
  Expect(state, !kBadRecentArg.ok,
         "RunTemporalReportQuery recent should fail DTO on invalid days.");
  Expect(state, Contains(kBadRecentArg.error_message, "RunTemporalReportQuery failed"),
         "RunTemporalReportQuery invalid argument should include operation name.");

  report_data_query->fail_target_not_found = true;
  const auto kMissingDay = runtime_api.report().RunTemporalReportQuery(
      {.display_mode = ReportDisplayMode::kDay,
       .selection = BuildDaySelection("2024-12-31"),
       .format = ReportFormat::kMarkdown});
  Expect(state, !kMissingDay.ok,
         "RunTemporalReportQuery should fail when named report target is missing.");
  Expect(state,
         kMissingDay.error_contract.error_code == "reporting.target.not_found",
         "RunTemporalReportQuery missing-target failure should expose stable error code.");
  Expect(state,
         kMissingDay.error_contract.error_category == "reporting",
         "RunTemporalReportQuery missing-target failure should expose reporting category.");
  report_data_query->fail_target_not_found = false;

  report_handler.period_batch_result = "period-batch-report";
  const auto kBatchSuccess = runtime_api.report().RunPeriodBatchQuery(
      {.days_list = {7, 14}, .format = ReportFormat::kMarkdown});
  Expect(state, kBatchSuccess.ok,
         "RunPeriodBatchQuery should return ok on success.");
  Expect(state, Contains(kBatchSuccess.content, "period:|"),
         "RunPeriodBatchQuery should use structured formatter when available.");
}

auto TestTemporalReportTargetsResponses(TestState& state) -> void {
  FakePipelineWorkflow pipeline_workflow;
  FakeReportHandler report_handler;
  auto report_data_query = std::make_shared<FakeReportDataQueryService>();
  auto runtime_api =
      BuildRuntimeApiForTest(pipeline_workflow, report_handler, report_data_query);

  const auto kSuccess = runtime_api.report().RunTemporalReportTargetsQuery(
      {.display_mode = ReportDisplayMode::kMonth});
  Expect(state, kSuccess.ok,
         "RunTemporalReportTargetsQuery should return ok on success.");
  Expect(state, kSuccess.items == report_data_query->monthly_targets,
         "RunTemporalReportTargetsQuery should return monthly canonical targets.");

  report_data_query->fail_list_targets = true;
  const auto kFailure = runtime_api.report().RunTemporalReportTargetsQuery(
      {.display_mode = ReportDisplayMode::kDay});
  Expect(state, !kFailure.ok,
         "RunTemporalReportTargetsQuery should return failed DTO when listing throws.");
  Expect(state, Contains(kFailure.error_message, "RunTemporalReportTargetsQuery"),
         "RunTemporalReportTargetsQuery failure should include operation name.");

  auto runtime_without_targets =
      BuildRuntimeApiForTest(pipeline_workflow, report_handler);
  const auto kMissingService =
      runtime_without_targets.report().RunTemporalReportTargetsQuery(
          {.display_mode = ReportDisplayMode::kYear});
  Expect(state, !kMissingService.ok,
         "RunTemporalReportTargetsQuery should fail when report data query service is missing.");
  Expect(state,
         Contains(kMissingService.error_message, "RunTemporalReportTargetsQuery"),
         "RunTemporalReportTargetsQuery missing-service failure should include operation name.");
}

auto TestStructuredWindowReportSemantics(TestState& state) -> void {
  FakePipelineWorkflow pipeline_workflow;
  FakeReportHandler report_handler;
  auto report_data_query = std::make_shared<FakeReportDataQueryService>();
  auto runtime_api =
      BuildRuntimeApiForTest(pipeline_workflow, report_handler, report_data_query);

  const auto kEmptyRecent = runtime_api.report().RunTemporalStructuredReportQuery(
      {.display_mode = ReportDisplayMode::kRecent,
       .selection = BuildRecentSelection(7)});
  Expect(state, kEmptyRecent.ok,
         "RunTemporalStructuredReportQuery recent should succeed for empty window.");
  Expect(state, kEmptyRecent.error_contract.error_code.empty(),
         "RunTemporalStructuredReportQuery recent empty window should not expose error code.");
  const auto* kRecentReport = std::get_if<PeriodReportData>(&kEmptyRecent.report);
  Expect(state, kRecentReport != nullptr,
         "RunTemporalStructuredReportQuery recent should return period report data.");
  if (kRecentReport != nullptr) {
    Expect(state, !kRecentReport->has_records,
           "RunTemporalStructuredReportQuery recent empty window should set has_records=false.");
    Expect(state, kRecentReport->matched_day_count == 0,
           "RunTemporalStructuredReportQuery recent empty window should set matched_day_count=0.");
    Expect(state, kRecentReport->matched_record_count == 0,
           "RunTemporalStructuredReportQuery recent empty window should set matched_record_count=0.");
    Expect(state, kRecentReport->requested_days == 7,
           "RunTemporalStructuredReportQuery recent should preserve requested_days.");
  }

  const auto kAnchoredRecent =
      runtime_api.report().RunTemporalStructuredReportQuery(
          {.display_mode = ReportDisplayMode::kRecent,
           .selection = BuildRecentSelection(7, "2026-03-07")});
  Expect(state, kAnchoredRecent.ok,
         "RunTemporalStructuredReportQuery anchored recent should succeed.");
  const auto* kAnchoredRecentReport =
      std::get_if<PeriodReportData>(&kAnchoredRecent.report);
  Expect(state, kAnchoredRecentReport != nullptr,
         "RunTemporalStructuredReportQuery anchored recent should return period data.");
  if (kAnchoredRecentReport != nullptr) {
    Expect(state, kAnchoredRecentReport->start_date == "2026-03-01",
           "Anchored recent should resolve fixed inclusive window start_date.");
    Expect(state, kAnchoredRecentReport->end_date == "2026-03-07",
           "Anchored recent should resolve fixed inclusive window end_date.");
    Expect(state, kAnchoredRecentReport->requested_days == 7,
           "Anchored recent should preserve requested_days.");
  }

  const auto kEmptyRange = runtime_api.report().RunTemporalStructuredReportQuery(
      {.display_mode = ReportDisplayMode::kRange,
       .selection = BuildRangeSelection("2024-12-01", "2024-12-31")});
  Expect(state, kEmptyRange.ok,
         "RunTemporalStructuredReportQuery range should succeed for empty window.");
  Expect(state, kEmptyRange.error_contract.error_code.empty(),
         "RunTemporalStructuredReportQuery range empty window should not expose error code.");
  const auto* kRangeReport = std::get_if<PeriodReportData>(&kEmptyRange.report);
  Expect(state, kRangeReport != nullptr,
         "RunTemporalStructuredReportQuery range should return period report data.");
  if (kRangeReport != nullptr) {
    Expect(state, !kRangeReport->has_records,
           "RunTemporalStructuredReportQuery range empty window should set has_records=false.");
    Expect(state, kRangeReport->matched_day_count == 0,
           "RunTemporalStructuredReportQuery range empty window should set matched_day_count=0.");
    Expect(state, kRangeReport->matched_record_count == 0,
           "RunTemporalStructuredReportQuery range empty window should set matched_record_count=0.");
  }

  const auto kInvalidRecent =
      runtime_api.report().RunTemporalStructuredReportQuery(
          {.display_mode = ReportDisplayMode::kRecent,
           .selection = BuildRecentSelection(0)});
  Expect(state, !kInvalidRecent.ok,
         "RunTemporalStructuredReportQuery recent should fail on non-positive days.");
  Expect(state,
         Contains(kInvalidRecent.error_message,
                  "RunTemporalStructuredReportQuery failed"),
         "RunTemporalStructuredReportQuery recent invalid argument should include operation name.");

  const auto kInvalidRange =
      runtime_api.report().RunTemporalStructuredReportQuery(
          {.display_mode = ReportDisplayMode::kRange,
           .selection = BuildRangeSelection("2026-01-31", "2026-01-01")});
  Expect(state, !kInvalidRange.ok,
         "RunTemporalStructuredReportQuery range should fail on descending range.");
  Expect(state,
         Contains(kInvalidRange.error_message,
                  "RunTemporalStructuredReportQuery failed"),
         "RunTemporalStructuredReportQuery range invalid argument should include operation name.");
}

}  // namespace

auto RunReportTests(TestState& state) -> void {
  TestTemporalReportQueryResponses(state);
  TestTemporalReportTargetsResponses(state);
  TestStructuredWindowReportSemantics(state);
}

}  // namespace tracer_core::application::tests

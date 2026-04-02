// application/tests/modules/report_tests.cpp
#include "application/tests/modules/reporting_tests.hpp"
#include "application/tests/support/fakes.hpp"
#include "application/tests/support/test_support.hpp"

namespace tracer_core::application::tests {

using tracer_core::core::dto::PeriodBatchQueryRequest;
using tracer_core::core::dto::ReportQueryType;
using tracer_core::core::dto::ReportTargetType;

namespace {

auto TestReportQueryResponses(TestState& state) -> void {
  FakePipelineWorkflow pipeline_workflow;
  FakeReportHandler report_handler;
  auto runtime_api = BuildRuntimeApiForTest(pipeline_workflow, report_handler);

  report_handler.daily_query_result = "daily-report";
  const auto kSuccess =
      runtime_api.report().RunReportQuery({.type = ReportQueryType::kDay,
                                           .argument = "20260101",
                                           .format = ReportFormat::kMarkdown});
  Expect(state, kSuccess.ok, "RunReportQuery should return ok on success.");
  Expect(state, kSuccess.content == "daily-report",
         "RunReportQuery should return handler content.");

  report_handler.fail_query = true;
  const auto kFailure =
      runtime_api.report().RunReportQuery({.type = ReportQueryType::kMonth,
                                           .argument = "202601",
                                           .format = ReportFormat::kMarkdown});
  Expect(state, !kFailure.ok,
         "RunReportQuery should return failed DTO when handler throws.");
  Expect(state, Contains(kFailure.error_message, "RunReportQuery failed"),
         "RunReportQuery failure should include operation name.");

  report_handler.fail_query = false;
  const auto kBadRecentArg =
      runtime_api.report().RunReportQuery({.type = ReportQueryType::kRecent,
                                           .argument = "abc",
                                           .format = ReportFormat::kMarkdown});
  Expect(state, !kBadRecentArg.ok,
         "RunReportQuery recent should fail DTO on invalid days argument.");
  Expect(state, Contains(kBadRecentArg.error_message, "RunReportQuery failed"),
         "RunReportQuery invalid argument should include operation name.");

  report_handler.fail_target_not_found = true;
  const auto kMissingDay =
      runtime_api.report().RunReportQuery({.type = ReportQueryType::kDay,
                                           .argument = "2024-12-31",
                                           .format = ReportFormat::kMarkdown});
  Expect(state, !kMissingDay.ok,
         "RunReportQuery should fail when named report target is missing.");
  Expect(state,
         kMissingDay.error_contract.error_code == "reporting.target.not_found",
         "RunReportQuery missing-target failure should expose stable error code.");
  Expect(state,
         kMissingDay.error_contract.error_category == "reporting",
         "RunReportQuery missing-target failure should expose reporting category.");
  report_handler.fail_target_not_found = false;

  report_handler.period_batch_result = "period-batch-report";
  const auto kBatchSuccess = runtime_api.report().RunPeriodBatchQuery(
      {.days_list = {7, 14}, .format = ReportFormat::kMarkdown});
  Expect(state, kBatchSuccess.ok,
         "RunPeriodBatchQuery should return ok on success.");
  Expect(state, kBatchSuccess.content == "period-batch-report",
         "RunPeriodBatchQuery should return handler content.");

  report_handler.fail_period_batch_query = true;
  const auto kBatchFailure = runtime_api.report().RunPeriodBatchQuery(
      {.days_list = {7, 14}, .format = ReportFormat::kMarkdown});
  Expect(state, !kBatchFailure.ok,
         "RunPeriodBatchQuery should return failed DTO when handler throws.");
  Expect(state, Contains(kBatchFailure.error_message, "RunPeriodBatchQuery"),
         "RunPeriodBatchQuery failure should include operation name.");
}

auto TestReportTargetsResponses(TestState& state) -> void {
  FakePipelineWorkflow pipeline_workflow;
  FakeReportHandler report_handler;
  auto report_data_query = std::make_shared<FakeReportDataQueryService>();
  auto runtime_api =
      BuildRuntimeApiForTest(pipeline_workflow, report_handler, report_data_query);

  const auto kSuccess =
      runtime_api.report().RunReportTargetsQuery({.type = ReportTargetType::kMonth});
  Expect(state, kSuccess.ok,
         "RunReportTargetsQuery should return ok on success.");
  Expect(state, kSuccess.items == report_data_query->monthly_targets,
         "RunReportTargetsQuery should return monthly canonical targets.");

  report_data_query->fail_list_targets = true;
  const auto kFailure =
      runtime_api.report().RunReportTargetsQuery({.type = ReportTargetType::kDay});
  Expect(state, !kFailure.ok,
         "RunReportTargetsQuery should return failed DTO when listing throws.");
  Expect(state, Contains(kFailure.error_message, "RunReportTargetsQuery"),
         "RunReportTargetsQuery failure should include operation name.");

  auto runtime_without_targets =
      BuildRuntimeApiForTest(pipeline_workflow, report_handler);
  const auto kMissingService = runtime_without_targets.report().RunReportTargetsQuery(
      {.type = ReportTargetType::kYear});
  Expect(state, !kMissingService.ok,
         "RunReportTargetsQuery should fail when report data query service is missing.");
  Expect(state, Contains(kMissingService.error_message, "RunReportTargetsQuery"),
         "RunReportTargetsQuery missing-service failure should include operation name.");
}

auto TestStructuredWindowReportSemantics(TestState& state) -> void {
  FakePipelineWorkflow pipeline_workflow;
  FakeReportHandler report_handler;
  auto report_data_query = std::make_shared<FakeReportDataQueryService>();
  auto runtime_api =
      BuildRuntimeApiForTest(pipeline_workflow, report_handler, report_data_query);

  const auto kEmptyRecent = runtime_api.report().RunStructuredReportQuery(
      {.type = ReportQueryType::kRecent, .argument = "7"});
  Expect(state, kEmptyRecent.ok,
         "RunStructuredReportQuery recent should succeed for empty window.");
  Expect(state, kEmptyRecent.error_contract.error_code.empty(),
         "RunStructuredReportQuery recent empty window should not expose error code.");
  const auto* kRecentReport =
      std::get_if<PeriodReportData>(&kEmptyRecent.report);
  Expect(state, kRecentReport != nullptr,
         "RunStructuredReportQuery recent should return period report data.");
  if (kRecentReport != nullptr) {
    Expect(state, !kRecentReport->has_records,
           "RunStructuredReportQuery recent empty window should set has_records=false.");
    Expect(state, kRecentReport->matched_day_count == 0,
           "RunStructuredReportQuery recent empty window should set matched_day_count=0.");
    Expect(state, kRecentReport->matched_record_count == 0,
           "RunStructuredReportQuery recent empty window should set matched_record_count=0.");
  }

  const auto kEmptyRange = runtime_api.report().RunStructuredReportQuery(
      {.type = ReportQueryType::kRange, .argument = "2024-12-01|2024-12-31"});
  Expect(state, kEmptyRange.ok,
         "RunStructuredReportQuery range should succeed for empty window.");
  Expect(state, kEmptyRange.error_contract.error_code.empty(),
         "RunStructuredReportQuery range empty window should not expose error code.");
  const auto* kRangeReport =
      std::get_if<PeriodReportData>(&kEmptyRange.report);
  Expect(state, kRangeReport != nullptr,
         "RunStructuredReportQuery range should return period report data.");
  if (kRangeReport != nullptr) {
    Expect(state, !kRangeReport->has_records,
           "RunStructuredReportQuery range empty window should set has_records=false.");
    Expect(state, kRangeReport->matched_day_count == 0,
           "RunStructuredReportQuery range empty window should set matched_day_count=0.");
    Expect(state, kRangeReport->matched_record_count == 0,
           "RunStructuredReportQuery range empty window should set matched_record_count=0.");
  }

  const auto kInvalidRecent = runtime_api.report().RunStructuredReportQuery(
      {.type = ReportQueryType::kRecent, .argument = "0"});
  Expect(state, !kInvalidRecent.ok,
         "RunStructuredReportQuery recent should fail on non-positive days.");
  Expect(state, Contains(kInvalidRecent.error_message, "RunStructuredReportQuery failed"),
         "RunStructuredReportQuery recent invalid argument should include operation name.");

  const auto kInvalidRange = runtime_api.report().RunStructuredReportQuery(
      {.type = ReportQueryType::kRange, .argument = "2026-01-31|2026-01-01"});
  Expect(state, !kInvalidRange.ok,
         "RunStructuredReportQuery range should fail on descending range.");
  Expect(state, Contains(kInvalidRange.error_message, "RunStructuredReportQuery failed"),
         "RunStructuredReportQuery range invalid argument should include operation name.");
}

}  // namespace

auto RunReportTests(TestState& state) -> void {
  TestReportQueryResponses(state);
  TestReportTargetsResponses(state);
  TestStructuredWindowReportSemantics(state);
}

}  // namespace tracer_core::application::tests

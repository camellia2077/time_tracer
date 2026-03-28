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

}  // namespace

auto RunReportTests(TestState& state) -> void {
  TestReportQueryResponses(state);
  TestReportTargetsResponses(state);
}

}  // namespace tracer_core::application::tests

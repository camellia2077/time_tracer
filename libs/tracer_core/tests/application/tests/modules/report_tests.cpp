// application/tests/modules/report_tests.cpp
#include "application/tests/modules/test_modules.hpp"
#include "application/tests/support/fakes.hpp"
#include "application/tests/support/test_support.hpp"

namespace tracer_core::application::tests {

using tracer_core::core::dto::PeriodBatchQueryRequest;
using tracer_core::core::dto::ReportExportType;
using tracer_core::core::dto::ReportQueryType;

namespace {

auto TestReportQueryResponses(TestState& state) -> void {
  FakeWorkflowHandler workflow_handler;
  FakeReportHandler report_handler;
  auto core_api = BuildCoreApiForTest(workflow_handler, report_handler);

  report_handler.daily_query_result = "daily-report";
  const auto kSuccess =
      core_api.RunReportQuery({.type = ReportQueryType::kDay,
                               .argument = "20260101",
                               .format = ReportFormat::kMarkdown});
  Expect(state, kSuccess.ok, "RunReportQuery should return ok on success.");
  Expect(state, kSuccess.content == "daily-report",
         "RunReportQuery should return handler content.");

  report_handler.fail_query = true;
  const auto kFailure =
      core_api.RunReportQuery({.type = ReportQueryType::kMonth,
                               .argument = "202601",
                               .format = ReportFormat::kMarkdown});
  Expect(state, !kFailure.ok,
         "RunReportQuery should return failed DTO when handler throws.");
  Expect(state, Contains(kFailure.error_message, "RunReportQuery failed"),
         "RunReportQuery failure should include operation name.");

  report_handler.fail_query = false;
  const auto kBadRecentArg =
      core_api.RunReportQuery({.type = ReportQueryType::kRecent,
                               .argument = "abc",
                               .format = ReportFormat::kMarkdown});
  Expect(state, !kBadRecentArg.ok,
         "RunReportQuery recent should fail DTO on invalid days argument.");
  Expect(state, Contains(kBadRecentArg.error_message, "RunReportQuery failed"),
         "RunReportQuery invalid argument should include operation name.");

  report_handler.period_batch_result = "period-batch-report";
  const auto kBatchSuccess = core_api.RunPeriodBatchQuery(
      {.days_list = {7, 14}, .format = ReportFormat::kMarkdown});
  Expect(state, kBatchSuccess.ok,
         "RunPeriodBatchQuery should return ok on success.");
  Expect(state, kBatchSuccess.content == "period-batch-report",
         "RunPeriodBatchQuery should return handler content.");

  report_handler.fail_period_batch_query = true;
  const auto kBatchFailure = core_api.RunPeriodBatchQuery(
      {.days_list = {7, 14}, .format = ReportFormat::kMarkdown});
  Expect(state, !kBatchFailure.ok,
         "RunPeriodBatchQuery should return failed DTO when handler throws.");
  Expect(state, Contains(kBatchFailure.error_message, "RunPeriodBatchQuery"),
         "RunPeriodBatchQuery failure should include operation name.");
}

auto TestReportExportResponses(TestState& state) -> void {
  FakeWorkflowHandler workflow_handler;
  FakeReportHandler report_handler;
  auto core_api = BuildCoreApiForTest(workflow_handler, report_handler);

  const auto kSuccess =
      core_api.RunReportExport({.type = ReportExportType::kDay,
                                .format = ReportFormat::kMarkdown,
                                .argument = "20260101",
                                .recent_days_list = {}});
  Expect(state, kSuccess.ok, "RunReportExport should return ok on success.");
  Expect(state, report_handler.daily_export_count == 1,
         "RunReportExport day should call day exporter once.");

  report_handler.fail_export = true;
  const auto kFailure =
      core_api.RunReportExport({.type = ReportExportType::kMonth,
                                .format = ReportFormat::kMarkdown,
                                .argument = "202601",
                                .recent_days_list = {}});
  Expect(state, !kFailure.ok,
         "RunReportExport should return failed DTO when exporter throws.");
  Expect(state, Contains(kFailure.error_message, "RunReportExport failed"),
         "RunReportExport failure should include operation name.");

  report_handler.fail_export = false;
  const auto kBadRecentArg =
      core_api.RunReportExport({.type = ReportExportType::kRecent,
                                .format = ReportFormat::kMarkdown,
                                .argument = "oops",
                                .recent_days_list = {}});
  Expect(state, !kBadRecentArg.ok,
         "RunReportExport recent should fail DTO on invalid days argument.");
  Expect(state, Contains(kBadRecentArg.error_message, "RunReportExport failed"),
         "RunReportExport invalid argument should include operation name.");
}

}  // namespace

auto RunReportTests(TestState& state) -> void {
  TestReportQueryResponses(state);
  TestReportExportResponses(state);
}

}  // namespace tracer_core::application::tests

// infrastructure/tests/android_runtime/android_runtime_smoke_io_report_tests.cpp
#include <iostream>

#include "infrastructure/tests/android_runtime/android_runtime_smoke_io_internal.hpp"

namespace android_runtime_tests::smoke {

auto VerifyReportOutputs(const std::shared_ptr<ITracerCoreApi>& core_api,
                         int& failures) -> void {
  const auto report_result = RunAndCheckReportQuery(
      core_api,
      {.type = tracer_core::core::dto::ReportQueryType::kRecent,
       .argument = "1",
       .format = ReportFormat::kMarkdown},
      "markdown", failures);
  if (report_result && report_result->content.empty()) {
    ++failures;
    std::cerr << "[FAIL] RunReportQuery(markdown) should return non-empty "
                 "content.\n";
  }

  const auto day_report_result = RunAndCheckReportQuery(
      core_api,
      {.type = tracer_core::core::dto::ReportQueryType::kDay,
       .argument = "2026-02-01",
       .format = ReportFormat::kMarkdown},
      "day markdown", failures);
  if (day_report_result) {
    if (!Contains(day_report_result->content, "- **Date**: ")) {
      ++failures;
      std::cerr << "[FAIL] Android day markdown report should include "
                   "'Date' label.\n";
    }
    if (!Contains(day_report_result->content, "- **Total Time Recorded**: ")) {
      ++failures;
      std::cerr << "[FAIL] Android day markdown report should include "
                   "'Total Time Recorded' label.\n";
    }
  }

  const auto structured_result = core_api->RunStructuredReportQuery(
      {.type = tracer_core::core::dto::ReportQueryType::kRecent,
       .argument = "1"});
  if (!structured_result.ok) {
    ++failures;
    std::cerr << "[FAIL] RunStructuredReportQuery should succeed: "
              << structured_result.error_message << '\n';
  }
  if (structured_result.kind !=
      tracer_core::core::dto::StructuredReportKind::kRecent) {
    ++failures;
    std::cerr << "[FAIL] RunStructuredReportQuery should return kRecent.\n";
  }
}

}  // namespace android_runtime_tests::smoke

// infrastructure/tests/android_runtime/android_runtime_smoke_io_report_tests.cpp
#include <iostream>

#include "infrastructure/tests/android_runtime/android_runtime_smoke_io_internal.hpp"
#include "infrastructure/tests/android_runtime/android_runtime_test_common.hpp"

namespace android_runtime_tests::smoke {

using tracer_core::core::dto::ReportDisplayMode;
using tracer_core::core::dto::TemporalSelectionKind;

auto VerifyReportOutputs(const std::shared_ptr<ITracerCoreRuntime>& runtime_api,
                         int& failures) -> void {
  const auto report_result = RunAndCheckReportQuery(
      runtime_api,
      {.display_mode = ReportDisplayMode::kRecent,
       .selection = {.kind = TemporalSelectionKind::kRecentDays, .days = 1},
       .format = ReportFormat::kMarkdown},
      "markdown", failures);
  if (report_result && report_result->content.empty()) {
    ++failures;
    std::cerr
        << "[FAIL] RunTemporalReportQuery(markdown) should return non-empty "
           "content.\n";
  }

  const auto day_report_result = RunAndCheckReportQuery(
      runtime_api,
      {.display_mode = ReportDisplayMode::kDay,
       .selection = {.kind = TemporalSelectionKind::kSingleDay,
                     .date = "2026-02-01"},
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

  const auto structured_result =
      runtime_api->report().RunTemporalStructuredReportQuery(
          {.display_mode = ReportDisplayMode::kRecent,
           .selection = {.kind = TemporalSelectionKind::kRecentDays,
                         .days = 1}});
  if (!structured_result.ok) {
    ++failures;
    std::cerr << "[FAIL] RunTemporalStructuredReportQuery should succeed: "
              << structured_result.error_message << '\n';
  }
  if (structured_result.display_mode != ReportDisplayMode::kRecent) {
    ++failures;
    std::cerr
        << "[FAIL] RunTemporalStructuredReportQuery should return recent.\n";
  }
}

}  // namespace android_runtime_tests::smoke

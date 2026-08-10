// infrastructure/tests/android_runtime/android_runtime_smoke_io_insights_tests.cpp
#include <iostream>

#include "infrastructure/tests/android_runtime/android_runtime_smoke_io_internal.hpp"
#include "infrastructure/tests/android_runtime/android_runtime_test_common.hpp"

namespace android_runtime_tests::smoke {

using tracer_core::core::dto::InsightsDisplayMode;
using tracer_core::core::dto::TemporalSelectionKind;

auto VerifyInsightsOutputs(const std::shared_ptr<ITracerCoreRuntime>& runtime_api,
                         int& failures) -> void {
  const auto insights_result = RunAndCheckInsightsQuery(
      runtime_api,
      {.display_mode = InsightsDisplayMode::kRecent,
       .selection = {.kind = TemporalSelectionKind::kRecentDays, .days = 1},
       .format = InsightsFormat::kMarkdown},
      "markdown", failures);
  if (insights_result && insights_result->content.empty()) {
    ++failures;
    std::cerr
        << "[FAIL] RunTemporalInsightsQuery(markdown) should return non-empty "
           "content.\n";
  }

  const auto day_insights_result = RunAndCheckInsightsQuery(
      runtime_api,
      {.display_mode = InsightsDisplayMode::kDay,
       .selection = {.kind = TemporalSelectionKind::kSingleDay,
                     .date = "2026-02-01"},
       .format = InsightsFormat::kMarkdown},
      "day markdown", failures);
  if (day_insights_result) {
    if (!Contains(day_insights_result->content, "- **Period**: ")) {
      ++failures;
      std::cerr << "[FAIL] Android day markdown insights should include "
                   "'Period' label.\n";
    }
    if (!Contains(day_insights_result->content, "- **Total Time Recorded**: ")) {
      ++failures;
      std::cerr << "[FAIL] Android day markdown insights should include "
                   "'Total Time Recorded' label.\n";
    }
  }

  const auto localized_day_insights_result = RunAndCheckInsightsQuery(
      runtime_api,
      {.display_mode = InsightsDisplayMode::kDay,
       .selection = {.kind = TemporalSelectionKind::kSingleDay,
                     .date = "2026-02-01"},
       .format = InsightsFormat::kMarkdown,
       .locale = "zh"},
      "day markdown zh", failures);
  if (localized_day_insights_result &&
      !Contains(localized_day_insights_result->content, "- **时间范围**: ")) {
    ++failures;
    std::cerr << "[FAIL] Android localized day markdown insights should use "
                 "the Chinese Period label.\n";
  }

  const auto structured_result =
      runtime_api->insights().RunTemporalStructuredInsightsQuery(
          {.display_mode = InsightsDisplayMode::kRecent,
           .selection = {.kind = TemporalSelectionKind::kRecentDays,
                         .days = 1}});
  if (!structured_result.ok) {
    ++failures;
    std::cerr << "[FAIL] RunTemporalStructuredInsightsQuery should succeed: "
              << structured_result.error_message << '\n';
  }
  if (structured_result.display_mode != InsightsDisplayMode::kRecent) {
    ++failures;
    std::cerr
        << "[FAIL] RunTemporalStructuredInsightsQuery should return recent.\n";
  }
}

}  // namespace android_runtime_tests::smoke
